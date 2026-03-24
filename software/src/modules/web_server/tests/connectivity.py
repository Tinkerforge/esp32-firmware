#!/usr/bin/env -S uv run --group tests --script

import requests
import time
from urllib.request import HTTPError
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext

def test_http_connectivity(tc: TestContext):
    # Check if the index page can be downloaded via HTTP and is larger than 100 KiB
    try:
        data = tc.http_request('GET', '/')
    except HTTPError as e:
        tc.fail('Web interface could not be downloaded: ' + e.msg)

    tc.assert_gt(50*1024, len(data))

    # Enable HTTP+HTTPS mode and check if the index page can be downloaded via HTTPS and is larger than 100 KiB
    if tc.api('network/config/transport_mode') != 2:
        tc.api('network/config/transport_mode', 2)
        tc.reboot()

    https_index_response = requests.get(f'https://{tc._esp_host}/', verify=False)

    tc.assert_eq(200, https_index_response.status_code)
    tc.assert_gt(50*1024, len(https_index_response.content))

    # Enable HTTPS-only mode, execute requests and enable HTTP+HTTPS mode again before checking assertions
    tc.api('network/config/transport_mode', 1)

    try:
        tc.reboot()
    except HTTPError as e:
        forbidden_message = e.msg

    http_forbidden_response = requests.get(f'http://{tc._esp_host}/', verify=False)

    api_response = requests.put(f'https://{tc._esp_host}/network/config/transport_mode', data='2', verify=False)
    requests.get(f'https://{tc._esp_host}/reboot', verify=False)
    time.sleep(10)

    # Check for 403 when checking status after reboot
    tc.assert_eq('Forbidden:HTTP disabled; use HTTPS instead', forbidden_message)

    # Check that the index page returned 403 via HTTP when HTTP was off
    tc.assert_eq(403, http_forbidden_response.status_code)
    tc.assert_eq('HTTP disabled; use HTTPS instead', http_forbidden_response.text)

    # Check final API call to make sure the test exits with the host correctly configured
    tc.assert_eq(200, api_response.status_code)

if __name__ == '__main__':
    run_testsuite(locals())
