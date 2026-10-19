#!/usr/bin/env -S uv run --group tests --script

import requests
import time
from urllib.request import HTTPError
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext


def suite_setup(tc: TestContext):
    tc.restore_before_suite_teardown('network/config')


# network/config does not support updating without reboot (yet?)
def suite_teardown(tc: TestContext):
    tc.reboot()


def test_http_connectivity(tc: TestContext):
    http_port = tc.api('network/config/web_server_port')
    https_port = tc.api('network/config/web_server_port_secure')

    tc.api_schema_port_list = [('http', http_port), ('https', https_port)]

    # Check if the index page can be downloaded via HTTP and is larger than 50 KiB
    try:
        data = tc.http_request('GET', '/', schema_port=('http', http_port))
    except HTTPError as e:
        tc.fail('Web interface could not be downloaded: ' + e.msg)

    tc.assert_gt(50 * 1024, len(data))

    # Enable HTTP+HTTPS mode and check if the index page can be downloaded via HTTPS and is larger than 50 KiB
    if tc.api('network/config/transport_mode') != 2:
        tc.api('network/config/transport_mode', 2)
        tc.reboot()

    try:
        data = tc.http_request('GET', '/', schema_port=('https', https_port))
    except HTTPError as e:
        tc.fail('Web interface could not be downloaded: ' + e.msg)

    tc.assert_gt(50 * 1024, len(data))

    # Enable HTTPS-only mode and reboot through HTTP while it is still available.
    tc.api('network/config/transport_mode', 1)
    tc.reboot()

    # tc.http_request uses urlopen
    # With urlopen, we can't introspect redirects.
    # Use requests instead where necessary.
    http_redirect_response = requests.get(
        f'http://{tc._esp_host}/redirect_test?foo=bar',
        allow_redirects=False,
    )
    redirected_index_response = requests.get(f'http://{tc._esp_host}/', verify=False)

    schema, empty, host, *rest = http_redirect_response.headers['Location'].split('/')
    location = "/".join([schema, empty, host.lower(), *rest])

    # Check that HTTP redirects to HTTPS while HTTP is off.
    tc.assert_eq(307, http_redirect_response.status_code)
    if https_port == 443:
        tc.assert_eq(f'https://{tc._esp_host}/redirect_test?foo=bar', location)
    else:
        tc.assert_eq(f'https://{tc._esp_host}:{https_port}/redirect_test?foo=bar', location)
    tc.assert_eq(200, redirected_index_response.status_code)
    tc.assert_eq('https', redirected_index_response.url.split(':', 1)[0])
    tc.assert_eq(1, len(redirected_index_response.history))
    tc.assert_eq(307, redirected_index_response.history[0].status_code)


if __name__ == '__main__':
    run_testsuite(locals())
