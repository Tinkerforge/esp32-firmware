# Running tests

You can either use the test_runner:
`./test_runner.py 'api/*/*' --host warp2-abcd --brickd warp2-abcd-pi --tty /dev/USB0`
or run a single test file directly:
`cd ../src/modules/api/tests; ./api.py --host warp2-abcd`

Passing
`--brickd` (hostname of a brickd were testbox hardware is attached),
`--host` (hostname of the ESP that is running the firmware to be tested) amd
`--tty` (serial console of that ESP)
is optional. Tests that require testbox hardware, an ESP (to call APIs of) and/or the serial console are skipped automatically.

Setting a test filter such as `api/*/*` is mandatory for now when using the test runner.
This will be fixed once all tests are ported to be compatible to the runner.
To execute all tests run  `./test_runner.py '*/*/*'` for now

# Writing tests

## Directory layout

esp32-firmware/software
    - test_runner/
        - README.md (you are here)
    - src/modules/
        - api/tests
            api.py
        - evse/tests
            foo.py

api.py and foo.py are testsuites.

## Testsuite files

Each testsuite file contains a fixed prefix

```python
#!/usr/bin/env -S uv run --group tests --script

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext
```

and a fixed suffix:
```python
if __name__ == '__main__':
    run_testsuite(locals())
```

Each function with a name starting with `test_` is a test. Each test is passed the TestContext:

```python
def test_api(tc: TestContext):
    tc.assert_(True)
```

The test context contains assert functions and other helpers.

Each test suite file may contain

```python
def suite_setup(tc: TestContext):
    print("Called once before the first test (or setup) in this file")

def suite_teardown(tc: TestContext):
    print("Called once after the last test (or teardown) in this file")

def setup(tc: TestContext):
    print("Called before every test")

def teardown(tc: TestContext):
    print("Called after every test")
```

## Generating tests

`run_testsuite` collects tests to be executed by searching for keys starting with `test_` in the passed dictionary (typically `locals()`).
To collect generated tests, add them to the dictionary like this:

```python
def generate_tests():
    return {f"test_{k}": lambda tc: tc.assert_eq(i, k) for i, k in enumerate(range(3))}

if __name__ == '__main__':
    run_testsuite(dict(locals(), **generate_tests()))
```
