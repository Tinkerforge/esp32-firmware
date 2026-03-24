#!/usr/bin/env -S uv run --group test-runner --script

from pathlib import Path
import subprocess
import os
import signal
import time
import sys
import argparse
import tempfile
import json
import select
import datetime
import atexit
import fcntl
import errno
import fnmatch
from tinkerforge_util.colored import red, green, blue, purple
from junit_xml import TestSuite as JTestSuite, TestCase as JTestCase, to_xml_report_string

DEFAULT_TEST_TIMEOUT = 5 * 60

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("test_filter", nargs='+')
    parser.add_argument("--junit-xml", action='store_true')
    parser.add_argument("--host")
    parser.add_argument("--tty")
    parser.add_argument("--brickd")

    args = parser.parse_args()
    quiet = args.junit_xml

    result = []

    def tprint(*args, **kwargs):
        if not quiet:
            print(*args, **kwargs, flush=True)

    suite_runs: list[tuple[Path, str]] = []  # (suite_path, test_filter)

    for tf in args.test_filter:
        module_filter, suite_filter, test_filter = tf.split('/')
        paths = list(Path(__file__).parent.glob(f"../src/modules/{module_filter}/tests/{suite_filter}.py"))
        if module_filter != '*' and fnmatch.fnmatch("test_runner", module_filter):
            paths += Path(__file__).parent.glob(f"./tests/{suite_filter}.py")
        for p in paths:
            suite_runs.append((p, test_filter))

    # Skip underscore-prefixed helper modules and modules without "run_testsuite"
    suite_runs = [(p, f) for p, f in suite_runs if not p.name.startswith('_') and 'run_testsuite' in p.read_text()]

    for suite_path, test_filter in suite_runs:
        path = suite_path.absolute()

        module_name = Path(path).parts[-3]
        suite_name = Path(path).stem

        jsuite = JTestSuite(f"{module_name}/{suite_name}")

        with tempfile.TemporaryDirectory() as d:
            fifo_in_path = Path(d) / "fifo_in"
            fifo_out_path = Path(d) / "fifo_out"
            os.mkfifo(fifo_in_path)
            os.mkfifo(fifo_out_path)

            [stdout, stdout_w] = os.pipe()
            [stderr, stderr_w] = os.pipe()

            fcntl.fcntl(stdout, fcntl.F_SETFL, fcntl.fcntl(stdout, fcntl.F_GETFL) | os.O_NONBLOCK)
            fcntl.fcntl(stderr, fcntl.F_SETFL, fcntl.fcntl(stderr, fcntl.F_GETFL) | os.O_NONBLOCK)

            env = os.environ.copy()
            env["PYTHONUNBUFFERED"] = "1"

            proc_args = [
                "uv",
                "run",
                "--active", # "Prefer the active virtual environment over the project's virtual environment" ensures that test scripts can use transitive dependencies of test context (for example esptool)
                "--script", path,
                "--test-filter", test_filter,
                "--fifo-in-path", fifo_in_path,
                "--fifo-out-path", fifo_out_path
            ]
            if args.host is not None:
                proc_args += ['--host', args.host]
            if args.tty is not None:
                proc_args += ['--tty', args.tty]
            if args.brickd is not None:
                proc_args += ['--brickd', args.brickd]

            proc = subprocess.Popen(
                proc_args,
                stdout=stdout_w,
                stderr=stderr_w,
                start_new_session=True,
                env=env)
            atexit.register(os.killpg, proc.pid, signal.SIGKILL)

            start = time.monotonic()
            while True:
                try:
                    fifo_in = os.open(str(fifo_in_path), os.O_WRONLY | os.O_NONBLOCK)
                    break
                except OSError as e:
                    if e.errno == errno.ENXIO:
                        if time.monotonic() - start > 1:
                            print(red("test script crashed"))
                            try:
                                print(os.read(stdout, 10000).decode('utf-8'))
                            except:
                                pass
                            try:
                                print(os.read(stderr, 10000).decode('utf-8'))
                            except:
                                pass
                            atexit.unregister(os.killpg)
                            sys.exit(1)
                        time.sleep(0.1)
                    else:
                        raise

            fifo_out = os.open(str(fifo_out_path), os.O_RDONLY)

            test_timeout = DEFAULT_TEST_TIMEOUT

            fifo_out_buf = bytes()
            stdout_buf = bytes()
            stderr_buf = bytes()
            dbg_buf = ""

            os.write(fifo_in, b"start\n")
            start = time.monotonic()

            while True:
                timeout = test_timeout - (time.monotonic() - start)
                if timeout < 0:
                    print("test timed out")
                    os.killpg(proc.pid, signal.SIGKILL)
                    break

                to_read, to_write, to_error = select.select([
                    stdout,
                    stderr,
                    fifo_out
                ], [], [
                    stdout,
                    stderr,
                    fifo_out
                ], timeout)

                if len(to_read) == 0 and len(to_write) == 0 and len(to_error) == 0:
                    print("test timed out")
                    os.killpg(proc.pid, signal.SIGKILL)
                    break

                if len(to_error) != 0:
                    print("fd {to_error} broken")
                    os.killpg(proc.pid, signal.SIGKILL)
                    break

                if stdout in to_read:
                    stdout_buf += os.read(stdout, 1000)

                if stderr in to_read:
                    stderr_buf += os.read(stderr, 1000)

                if fifo_out in to_read:
                    new_str = os.read(fifo_out, 1000)
                    if len(new_str) == 0:
                        proc.wait()
                        #proc.poll()
                        if proc.returncode != 0:
                            print(red(f"test script returned code {proc.returncode}"))
                        break

                    fifo_out_buf += new_str
                    while b"\n" in fifo_out_buf:
                        line, fifo_out_buf = fifo_out_buf.split(b"\n", maxsplit=1)
                        line = line.decode("utf-8")

                        cmd, payload = json.loads(line)

                        now = datetime.datetime.now().isoformat()

                        meta_test = payload["testname"].split("/")[-1] in ["suite_setup", "suite_teardown", "setup", "teardown"]
                        test_finished = False

                        match cmd:
                            case 'notify_test_start':
                                test_timeout = DEFAULT_TEST_TIMEOUT
                                start = time.monotonic()
                                if not meta_test:
                                    tprint(f"{now} Started {payload["testname"]}", end=" ")
                                    jcase = JTestCase(payload["testname"], timestamp=now)
                                    jsuite.test_cases.append(jcase)

                            case 'notify_test_success':
                                test_finished = True
                                if not meta_test:
                                    tprint(green("Success"), f"({time.monotonic() - start:.3f}s)")

                            case 'notify_test_failure':
                                test_finished = True
                                if meta_test:
                                    tprint(payload["testname"], end=" ")
                                else:
                                    x: list[JTestCase] = jsuite.test_cases
                                    x[-1].add_failure_info(payload["error"], payload["tb"], "failure")
                                tprint(red("Failure"), f"({time.monotonic() - start:.3f}s)")
                                tprint("    " + "\n    ".join(x.removeprefix('  ') for x in payload["error"].split("\n")))

                            case 'notify_test_error':
                                test_finished = True
                                if meta_test:
                                    tprint(payload["testname"], end=" ")
                                else:
                                    x: list[JTestCase] = jsuite.test_cases
                                    x[-1].add_error_info(payload["error"], payload["tb"], "error")
                                tprint(purple("Error"), f"({time.monotonic() - start:.3f}s)")
                                tprint("    " + "\n    ".join(payload["tb"].split("\n")))

                            case 'notify_test_skipped':
                                test_finished = True
                                if not meta_test:
                                    tprint(blue("Skipped"), payload["reason"])
                                    x: list[JTestCase] = jsuite.test_cases
                                    x[-1].add_skipped_info(payload["reason"])

                            case 'set_test_timeout':
                                test_timeout = payload["timeout"]
                                if meta_test:
                                    tprint(f"Set test timeout to {test_timeout}")

                            case 'dbg':
                                dbg_buf += payload["message"]

                        if test_finished:
                            if not meta_test:
                                x: list[JTestCase] = jsuite.test_cases
                                x[-1].elapsed_sec = time.monotonic() - start
                                # TODO x[-1].log
                                x[-1].stdout = stdout_buf.decode('utf-8')
                                x[-1].stderr = stderr_buf.decode('utf-8')
                                x[-1].log = dbg_buf
                                x[-1].url = int(time.monotonic() * 1000)
                                stdout_buf = b""
                                stderr_buf = b""
                                dbg_buf = ""
                            elif payload["testname"] == "setup":
                                if len(stdout_buf) > 0:
                                    stdout_buf += b"\n---END SETUP---\n"
                                if len(stderr_buf) > 0:
                                    stderr_buf += b"\n---END SETUP---\n"
                                if len(dbg_buf) > 0:
                                    dbg_buf += "---END SETUP---\n"
                            elif payload["testname"] == "teardown":
                                x: list[JTestCase] = jsuite.test_cases
                                if len(stdout_buf) > 0:
                                    x[-1].stdout = (x[-1].stdout or "") + "\n---BEGIN TEARDOWN---\n" + stdout_buf.decode('utf-8')
                                if len(stderr_buf) > 0:
                                    x[-1].stderr = (x[-1].stderr or "") + "\n---BEGIN TEARDOWN---\n" + stderr_buf.decode('utf-8')
                                if len(dbg_buf) > 0:
                                    x[-1].log += "\n---BEGIN TEARDOWN---\n" + dbg_buf
                                stdout_buf = b""
                                stderr_buf = b""
                                dbg_buf = ""

                            if payload["testname"] != "suite_teardown":
                                try:
                                    os.write(fifo_in, b"start\n")
                                except BrokenPipeError:
                                    # If the fifo is already closed, this was the last test in the suite
                                    break

            # Either the process group was killed or the leader exited.
            atexit.unregister(os.killpg)

        result.append(jsuite)

        # print(f"Suite {path.stem} done.\n---stdout---")
        # print(stdout_buf.decode("utf-8"))
        # print("---stderr---")
        # print(stderr_buf.decode("utf-8"))
        # print("---")

        # foo: list[JTestCase] = jsuite.test_cases
        # for tc in foo:
        #     print(tc.name)
        #     print("---stdout---")
        #     print(tc.stdout)
        #     print("---stderr---")
        #     print(tc.stderr)
        #     print("\n\n\n")

    if args.junit_xml:
        print(to_xml_report_string(result))

if __name__ == '__main__':
    main()
