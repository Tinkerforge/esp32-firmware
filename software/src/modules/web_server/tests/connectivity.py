#!/usr/bin/env -S uv run --group tests --script

import requests
import time
from urllib.request import HTTPError
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext

def test_http_connectivity(tc: TestContext):
    https_port = tc.api('network/config/web_server_port_secure')
    https_authority = tc._esp_host if https_port == 443 else f'{tc._esp_host}:{https_port}'

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

    https_index_response = requests.get(f'https://{https_authority}/', verify=False)

    tc.assert_eq(200, https_index_response.status_code)
    tc.assert_gt(50*1024, len(https_index_response.content))

    # Enable HTTPS-only mode and reboot through HTTP while it is still available.
    tc.api('network/config/transport_mode', 1)

    tc.api('reboot', '')
    time.sleep(10)

    http_redirect_response = requests.get(
        f'http://{tc._esp_host}/redirect_test?foo=bar',
        allow_redirects=False,
    )
    redirected_index_response = requests.get(f'http://{tc._esp_host}/', verify=False)

    api_response = requests.put(f'https://{https_authority}/network/config/transport_mode', data='2', verify=False)
    requests.get(f'https://{https_authority}/reboot', verify=False)
    time.sleep(10)

    # Check that HTTP redirects to HTTPS while HTTP is off.
    tc.assert_eq(307, http_redirect_response.status_code)
    tc.assert_eq(f'https://{https_authority}/redirect_test?foo=bar', http_redirect_response.headers['Location'])
    tc.assert_eq(200, redirected_index_response.status_code)
    tc.assert_eq('https', redirected_index_response.url.split(':', 1)[0])
    tc.assert_eq(1, len(redirected_index_response.history))
    tc.assert_eq(307, redirected_index_response.history[0].status_code)

    # Check final API call to make sure the test exits with the host correctly configured
    tc.assert_eq(200, api_response.status_code)

if __name__ == '__main__':
    run_testsuite(locals())
