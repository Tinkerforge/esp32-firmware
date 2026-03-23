#!/usr/bin/env -S uv run --script

import gzip
from io import BytesIO
import re
from urllib.request import HTTPError

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .....test_runner.test_context import TestContext, run_testsuite
else:
    import tinkerforge_util as tfutil
    tfutil.create_parent_module(__file__, 'software')
    from software.test_runner.test_context import run_testsuite, TestContext


### Test if the boot ID is different after a reboot.
def test_boot_id(tc: TestContext):
    old_boot_id = tc.api('event_log/boot_id')['boot_id']
    tc.reboot()
    new_boot_id = tc.api('event_log/boot_id')['boot_id']

    tc.assert_ne(new_boot_id, old_boot_id)


### Test if the event log can be downloaded and vaguely looks like one.
def test_event_log(tc: TestContext):
    try:
        data = tc.http_request('GET', 'event_log')
    except HTTPError as e:
        tc.fail('Event log could not be downloaded: ' + e.msg)

    tc.assert_gt(1024, len(data))

    text = data.decode('utf-8')
    match = re.search(' \\| .{16} \\| ', text)
    tc.assert_ne(None, match)


### Test if the uncompressed and compressed trace log can be downloaded and vaguely look like one.
def decompress_gzip_bytes(data):
    with gzip.GzipFile(fileobj=BytesIO(data)) as f:
        decompressed_data = f.read()
    return decompressed_data

def trace_log_test(tc: TestContext, uri: str, compressed: bool):
    try:
        data = tc.http_request('GET', uri)
    except HTTPError as e:
        tc.fail('Trace log could not be downloaded: ' + e.msg)

    if len(data) == 0:
        # ESP32 without PSRAM? Let it pass.
        return

    if compressed:
        data = decompress_gzip_bytes(data)

    text = data.decode('utf-8')
    begin_rtc_position = text.find('__begin_rtc__')
    tc.assert_ge(0, begin_rtc_position)

def test_trace_log(tc: TestContext):
    trace_log_test(tc, 'trace_log', compressed=False)

def test_trace_log_compressed(tc: TestContext):
    trace_log_test(tc, 'trace_log/10020', compressed=True)


if __name__ == '__main__':
    run_testsuite(locals())
