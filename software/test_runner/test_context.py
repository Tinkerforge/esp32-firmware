from pathlib import Path
from collections.abc import Callable
import traceback
import typing
from dataclasses import dataclass
import argparse
import fnmatch
import json
from urllib.request import Request, urlopen, HTTPError

type TestFn = Callable[[TestContext], typing.Any]

@dataclass
class TestCase:
    name: str
    fn: TestFn

@dataclass
class TestSuite:
    module: str
    suite: str
    tests: list[TestCase]
    suite_setup:    TestFn | None = None
    suite_teardown: TestFn | None = None
    test_setup:     TestFn | None = None
    test_teardown:  TestFn | None = None

JSON: typing.TypeAlias = dict[str, "JSON"] | list["JSON"] | str | int | float | bool | None

@dataclass
class TestContext:
    _fifo_in:  typing.TextIO | None
    _fifo_out: typing.TextIO | None

    _serial_port: str | None
    _esp_host: str
    _brickd_host: str | None


    def _get_callee(self):
        stack = traceback.extract_stack()
        return traceback.format_list([stack[-3]])[0]

    def assert_(self, pred):
        if not pred:
            raise AssertionError(self._get_callee())

    class SkipTestError(Exception):
        pass

    def skip(self, reason=""):
        raise TestContext.SkipTestError(reason)

    def _to_runner(self, cmd, **kwargs):
        payload = json.dumps([cmd, kwargs], separators=(',', ':')) + "\n"
        if self._fifo_out is not None:
            self._fifo_out.write(payload)
        else:
            # TODO do we want to print this or just drop it?
            print(payload, end="") # payload already contains \n

    def set_test_timeout(self, timeout: float):
        self._to_runner("set_test_timeout", timeout=timeout)

    def _notify_test_start(self, testname: str):
        self.testname = testname
        self._to_runner("notify_test_start", testname=self.testname)

    def _notify_test_success(self):
        self._to_runner("notify_test_success", testname=self.testname)
        self.testname = ""

    def _notify_test_failure(self, error: str, tb: str):
        self._to_runner("notify_test_failure", testname=self.testname, error=error, tb=tb)
        self.testname = ""

    def _notify_test_error(self, error: str, tb: str):
        self._to_runner("notify_test_error", testname=self.testname, error=error, tb=tb)
        self.testname = ""

    def _notify_test_skipped(self, reason: str):
        self._to_runner("notify_test_skipped", testname=self.testname, reason=reason)
        self.testname = ""

    def _wait_for_start(self):
        if self._fifo_in is not None:
            self._fifo_in.readline()

    def api(self, method:str, api: str, payload: JSON = None, timeout: float = 1, parse: bool = True):
        req = Request(f'http://{self._esp_host}/{api}', data=json.dumps(payload).encode("utf-8"), method=method, headers={"Content-Type": "application/json"})
        try:
            with urlopen(req, timeout=timeout) as resp:
                result = resp.read()
                if parse:
                    return json.loads(result)
                else:
                    return result
        except HTTPError as e:
            e.msg += ":" + e.read().decode('utf-8')
            raise

    def api_get(self, api: str, payload: JSON = None, *, timeout: float = 1, parse: bool = True):
        return self.api('GET', api, payload, timeout, parse)

    def api_put(self, api: str, payload: JSON = None, *, timeout: float = 1, parse: bool = True):
        return self.api('PUT', api, payload, timeout, parse)

    def api_post(self, api: str, payload: JSON = None, *, timeout: float = 1, parse: bool = True):
        return self.api('POST', api, payload, timeout, parse)
def run_test(tc: TestContext, name: str, fn: TestFn | None) -> bool:
    if fn is None:
        return True

    tc._wait_for_start()

    tc._notify_test_start(name)

    try:
        fn(tc)
        tc._notify_test_success()
        return True
    except TestContext.SkipTestError as e:
        tc._notify_test_skipped(reason=str(e.args[0]))
    except AssertionError as e:
        tc._notify_test_failure(error=str(e), tb=traceback.format_exc())
    except Exception as e:
        tc._notify_test_error(error=str(e), tb=traceback.format_exc())

    return False

def run_testsuite(l: dict[str, typing.Any]):
    suite = TestSuite(Path(l['__file__']).parts[-3], Path(l['__file__']).stem, [])

    parser = argparse.ArgumentParser()
    parser.add_argument("--test-filter", default='*')
    parser.add_argument("--fifo-in-path")
    parser.add_argument("--fifo-out-path")
    parser.add_argument("--host")
    parser.add_argument("--tty")
    parser.add_argument("--brickd")
    args = parser.parse_args()

    # l is locals() of the calling test suite script, containing test, setup and teardown functions.
    for k, v in l.items():
        if not callable(v):
            continue

        if k.startswith("test_") and fnmatch.fnmatch(k, args.test_filter):
            suite.tests.append(TestCase(f"{suite.module}/{suite.suite}/{k}", v))

        if k == "suite_setup": suite.suite_setup = v
        if k == "suite_teardown": suite.suite_teardown = v
        if k == "setup": suite.test_setup = v
        if k == "teardown": suite.test_teardown = v

    tc = TestContext(
            open(args.fifo_in_path, 'r', buffering=1) if args.fifo_in_path else None,
            open(args.fifo_out_path, 'w', buffering=1) if args.fifo_in_path else None,
            args.tty,
            args.host,
            args.brickd
        )

    if not run_test(tc, "suite_setup", suite.suite_setup):
        run_test(tc, "suite_teardown", suite.suite_teardown)
        return

    for t in suite.tests:
        if not run_test(tc, "setup", suite.test_setup):
            run_test(tc, "teardown", suite.test_teardown)
            break

        run_test(tc, t.name, t.fn)
        if not run_test(tc, "teardown", suite.test_teardown):
            break

    run_test(tc, "suite_teardown", suite.suite_teardown)
