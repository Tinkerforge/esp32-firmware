from pathlib import Path
from collections.abc import Callable
import traceback
import typing
from dataclasses import dataclass
import argparse
import fnmatch
import json
import re
import io
import time
from urllib.request import Request, urlopen, HTTPError

import jsonpath_ng

import esptool.cmds
from esptool.targets import ESP32ROM

if typing.TYPE_CHECKING:
    from test_runner import parttool
    from test_runner.testbox.testbox import TestBox
    from test_runner.testbox.warp3 import WARP3TestBox
    from test_runner.testbox.warp2 import WARP2TestBox
    from test_runner.testbox.wem import WEMTestBox
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner import parttool
    from software.test_runner.testbox.testbox import TestBox
    from software.test_runner.testbox.warp3 import WARP3TestBox
    from software.test_runner.testbox.warp2 import WARP2TestBox
    from software.test_runner.testbox.wem import WEMTestBox

type TestFn = Callable[[TestContext], typing.Any]

@dataclass
class TestCaseInfo:
    name: str
    fn: TestFn

@dataclass
class TestSuiteInfo:
    module: str
    suite: str
    tests: list[TestCaseInfo]
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

    _testbox: TestBox | None = None

    def init_testbox(self):
        if self._brickd_host is None:
            return False

        if self._brickd_host.startswith('warp3'):
            self._testbox = WARP3TestBox()
        elif self._brickd_host.startswith('warp2'):
            self._testbox = WARP2TestBox()
        elif self._brickd_host.startswith('wem'):
            self._testbox = WEMTestBox()
        else:
            return False

        self._testbox.tc = self
        self._testbox.start(self._brickd_host, 4223)
        return True

    def get_testbox(self) -> TestBox:
        if self._testbox is None:
            if not self.init_testbox():
                self.skip("No testbox connected")

        assert self._testbox is not None
        return self._testbox

    def _get_callee(self):
        stack = traceback.extract_stack()
        return traceback.format_list([stack[-3]])[0]

    class SkipTestError(Exception):
        pass

    def skip(self, reason=""):
        raise TestContext.SkipTestError(reason)

    def _to_runner(self, cmd, **kwargs):
        payload = json.dumps([cmd, dict({"testname": self.testname}, **kwargs)], separators=(',', ':')) + "\n"
        if self._fifo_out is not None:
            self._fifo_out.write(payload)
        else:
            # TODO do we want to print this or just drop it?
            print(payload, end="") # payload already contains \n

    def set_test_timeout(self, timeout: float):
        self._to_runner("set_test_timeout", timeout=timeout)

    def _notify_test_start(self, testname: str):
        self.testname = testname
        self._to_runner("notify_test_start")

    def _notify_test_success(self):
        self._to_runner("notify_test_success")
        self.testname = ""

    def _notify_test_failure(self, error: str, tb: str):
        self._to_runner("notify_test_failure", error=error, tb=tb)
        self.testname = ""

    def _notify_test_error(self, error: str, tb: str):
        self._to_runner("notify_test_error", error=error, tb=tb)
        self.testname = ""

    def _notify_test_skipped(self, reason: str):
        self._to_runner("notify_test_skipped", reason=reason)
        self.testname = ""

    def dbg(self, *args, **kwargs):
        x = io.StringIO()
        print(*args, file=x, **kwargs)
        message = str(x)
        self._to_runner("dbg", message=message + ("\n" if not message.endswith("\n") else ""))

    def _wait_for_start(self):
        if self._fifo_in is not None:
            self._fifo_in.readline()

    def _erase_littlefs_header(self):
        if not self._serial_port:
            return False

        with ESP32ROM(self._serial_port) as esp:
            esp.connect()
            part_table = esptool.cmds.read_flash(esp, 0x8000, 0x1000)
            target = parttool.ParttoolTarget(part_table)

            for p in target.partition_table:
                if p.type == parttool.DATA_TYPE and p.subtype == parttool.SUBTYPES[parttool.DATA_TYPE]['spiffs']:
                    partition_offset = p.offset
                    partition_size = p.size

            if partition_offset is None or partition_size is None:
                raise Exception("Failed to get spiffs partition offset or size from partition table!")

            esptool.cmds.erase_region(esp, partition_offset, 8192)
        return True

    def call_api(self, method:str, api: str, payload: JSON = None, timeout: float = 1, parse: bool = True):
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
        return self.call_api('GET', api, payload, timeout, parse)

    def api_put(self, api: str, payload: JSON = None, *, timeout: float = 1, parse: bool = True):
        return self.call_api('PUT', api, payload, timeout, parse)

    def api_post(self, api: str, payload: JSON = None, *, timeout: float = 1, parse: bool = True):
        return self.call_api('POST', api, payload, timeout, parse)

    def factory_reset(self):
        if self._serial_port:
            self._erase_littlefs_header()
        else:
            self.api_put('factory_reset', {"do_i_know_what_i_am_doing": True})

    def assert_(self, actual):
        if not actual:
            raise AssertionError(f"Expected {actual=} to be true\n" + self._get_callee())

        return actual

    def assert_true(self, actual):
        if not actual:
            raise AssertionError(f"Expected {actual=} to be true\n" + self._get_callee())

        return actual

    def assert_false(self, actual):
        if actual:
            raise AssertionError(f"Expected {actual=} to be false\n" + self._get_callee())

        return actual

    def assert_not(self, actual):
        if actual:
            raise AssertionError(f"Expected {actual=} to be false\n" + self._get_callee())

        return actual

    def assert_eq(self, expected, actual):
        if not (actual == expected):
            raise AssertionError(f"Expected {actual=} to be equal to {expected=}\n" + self._get_callee())

        return actual

    def assert_ne(self, expected, actual):
        if not (actual != expected):
            raise AssertionError(f"Expected{actual=} to not be equal to {expected=}\n" + self._get_callee())

        return actual

    def assert_le(self, expected, actual):
        if not (actual <= expected):
            raise AssertionError(f"Expected {actual=} to be less than or equal to {expected=}\n" + self._get_callee())

        return actual

    def assert_lt(self, expected, actual):
        if not (actual < expected):
            raise AssertionError(f"Expected {actual=} to be less than {expected=}\n" + self._get_callee())

        return actual

    def assert_ge(self, expected, actual):
        if not (actual >= expected):
            raise AssertionError(f"Expected {actual=} to be greater than {expected=}\n" + self._get_callee())

        return actual

    def assert_gt(self, expected, actual):
        if not (actual > expected):
            raise AssertionError(f"Expected {actual=} to be greater than {expected=}\n" + self._get_callee())

        return actual

    def assert_search(self, expected: typing.Union[str, re.Pattern], actual) -> re.Match:
        if isinstance(expected, re.Pattern):
            match = expected.search(actual)
        else:
            match = re.search(expected, actual)

        if match is None:
            raise AssertionError(f"Expected {actual=} to contain pattern {expected=}\n" + self._get_callee())

        return match

    def assert_in(self, expected, actual):
        if not (actual in expected):
            raise AssertionError(f"Expected {actual=} to contain {expected=}\n" + self._get_callee())

        return actual

    def assert_epsilon(self, expected, epsilon, actual):
        if not (expected - epsilon) <= actual <= (expected + epsilon):
            raise AssertionError(f"Expected {actual=} to be in range {expected=} ± {epsilon=}\n" + self._get_callee())

        return actual

    def _fixup_json_path(self, json_path: str):
        if not json_path.startswith('$') and not json_path.startswith('['):
            json_path = '$.' + json_path
        return json_path

    def api(self, api, json_path=None):
        result = self.api_get(api)
        if json_path is None:
            return result

        json_path = self._fixup_json_path(json_path)
        expr = jsonpath_ng.parse(json_path)

        result = [match.value for match in expr.find(self.api_get(api))]
        if len(result) == 1:
            return result[0]
        return result

    def fail(self, message: str):
        raise AssertionError(f"Test failure: {message}")

    def wait_for(self, assert_fn: typing.Callable[[], typing.Any], *, timeout: float = 5.0, poll_delay: float = 0.1) -> typing.Any:
        start = time.monotonic()
        end = start + timeout

        act = '___assert_fn not executed yet___'
        while time.monotonic() <= end:
            try:
                act = assert_fn()
                break
            except AssertionError:
                time.sleep(poll_delay)
        else:
            raise AssertionError(f"Timed out while waiting for {assert_fn=} (last value was {repr(act)}) to not fail\n" + self._get_callee())

        return act

    def assert_fail(self, assert_fn: typing.Callable[[], typing.Any]):
        """Assert that the passed function raises an AssertionError"""
        error = None
        self.dbg("Assert fail {")
        try:
            assert_fn()
        except AssertionError as e:
            error = e
            self.dbg(" Failed")
        self.dbg("}")

        if error is None:
            raise AssertionError(f"Assertion failed: expected inner asserts to fail but all succeeded. ({self._get_callee()})")

    def assert_xor(self, assert_fn_left, assert_fn_right):
        """Assert that exactly one of the passed functions raises an AssertionError"""
        left_error = None
        right_error = None

        self.dbg("Assert XOR {")
        self.dbg("left:")

        try:
            assert_fn_left()
        except AssertionError as e:
            left_error = e
            self.dbg(" Failed")

        self.dbg("right:")

        try:
            assert_fn_right()
        except AssertionError as e:
            right_error = e
            self.dbg(" Failed")

        self.dbg("}")

        if left_error is None and right_error is None:
            raise AssertionError(f"Assertion failed: expected one failed assertion but both succeeded\n" + self._get_callee())

        if left_error is not None and right_error is not None:
            raise AssertionError(f"Assertion failed: expected one failed assertion but both failed. Left {left_error} right {right_error}\n" + self._get_callee())

    # @dataclass
    # class Cap:
    #     val: typing.Any
    #     "Value at the given point in time"

    #     at: float = -1
    #     "Absolute timestamp of captured data point"

    #     after: float = -1
    #     "Relative (to last) timestamp of captured data point"

    #     less: float = 0
    #     "Allow data point to be this much before the specified timestamp"

    #     more: float = 0
    #     "Allow data point to be this much after the specified timestamp"

    # def assert_capture_eq(self, expected: list[Cap], actual, *, print_cap_on_fail = False):
    #     """Assert that collected capture (see start_capture() and stop_capture()) is equal to the expected capture."""
    #     try:
    #         if len(expected) != len(actual):
    #             raise AssertionError(f"Assertion failed: actual capture has length {len(actual)}, expected {len(expected)}: " + self._get_callee())

    #         abs_time = 0

    #         for i, x in enumerate(zip(expected, actual)):
    #             e: TestContext.Cap = x[0]
    #             if e.at == -1 and e.after == -1:
    #                 raise Exception("Both at and after of an expected capture datapoint were not set! Set exactly one.")
    #             if e.at != -1 and e.after != -1:
    #                 raise Exception("Both at and after of an expected capture datapoint were set! Set exactly one.")

    #             if e.at == -1:
    #                 e.at = abs_time + e.after

    #             a_time, a_val = x[1]

    #             if not e.val.get(a_val):
    #                 raise AssertionError(f"Assertion failed: actual capture \"{e.val.message(a_val)}\" at index {i} (timestamp{a_time:.3f}): " + self._get_callee())

    #             lower = e.at - e.less
    #             upper = e.at + e.more
    #             if not lower <= a_time <= upper:
    #                 raise AssertionError(f"Assertion failed: actual capture has correct value {a_val} but timestamp {a_time:.3f} at index {i}, expected in range of {lower:.3f} to {upper:.3f}: " + self._get_callee())

    #             abs_time = a_time
    #     except:
    #         if print_cap_on_fail:
    #             print(f'{expected=}')
    #             print(f'{actual=}')
    #         raise


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
    suite = TestSuiteInfo(Path(l['__file__']).parts[-3], Path(l['__file__']).stem, [])

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
            suite.tests.append(TestCaseInfo(f"{suite.module}/{suite.suite}/{k}", v))

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
