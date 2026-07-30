#!/usr/bin/env -S uv run --group tests --script

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from urllib.error import HTTPError

def teardown(tc: TestContext):
    tc.http_request('PUT', '/config_import/abort', timeout=5)

def test_begin(tc: TestContext):
    expected = tc.list_dir('/certs') + tc.list_dir("/config")

    excluded = ["/config/users_config", "/config/info_last_boots", "/config/remote_access_config"]

    expected = [e for e in expected if e not in excluded]

    tc.http_request('PUT', '/config_import/begin', timeout=5)
    actual = [a.removeprefix("/config_import") for a in tc.list_dir('/config_import')]

    tc.assert_list_eq(expected, actual)


def test_begin_removes_old_directory(tc: TestContext):
    tc.http_request('PUT', '/config_import/begin', timeout=5)
    expected = tc.list_dir('/config_import')

    tc.upload_file('/config_import/foo', 'bar'.encode('utf-8'))
    tc.assert_in(tc.list_dir('/config_import'), "/config_import/foo")

    tc.http_request('PUT', '/config_import/begin', timeout=5)
    actual = tc.list_dir('/config_import')
    tc.assert_list_eq(expected, actual)


def test_path_sanitize(tc: TestContext):
    should_404 = [
        "/config_import/file",
    ]

    invalid_paths = [
        "/config_import/file/foobar",
        "/config_import/file/",
        "/config_import/file/../",
        "/config_import/file/config",
        "/config_import/file/config/",
        "/config_import/file/config/.",
        "/config_import/file/config/../config",
        "/config_import/file/config/../foobar/baz",
        "/config_import/file/certs/../foobar/baz",
    ]

    valid_paths = [
        ("/config_import/certs/0",         "/config_import/file/certs/0"),
        ("/config_import/certs/0_name",    "/config_import/file/certs/0_name"),
        ("/config_import/config/foo",      "/config_import/file/config/foo"),
        ("/config_import/config/foo2/bar", "/config_import/file/config/foo2/bar"),
        ("/config_import/config/baz",      "/config_import/file/config/../config/baz"),
        ("/config_import/config/qux",      "/config_import/file/config/../../data/config/qux"),
    ]

    for p in should_404:
        try:
            tc.http_request('PUT', p)
        except HTTPError as e:
            tc.assert_eq(404, e.code)
        else:
            tc.fail("Expected 404, but request succeeded")

    for p in invalid_paths:
        try:
            tc.http_request('PUT', p)
        except HTTPError as e:
            tc.assert_eq(400, e.code)
            tc.assert_contains("Path does not point into allowed directory", e.msg)
        else:
            tc.fail("Expected 400, but request succeeded")

    for _, p in valid_paths:
        print(p)
        tc.assert_not_raising(HTTPError, lambda: tc.http_request('PUT', p, "foo"))


    files = tc.list_dir('/config_import')
    for e, _ in valid_paths:
        tc.assert_in(files, e)


def test_create_file_in_file(tc: TestContext):
    tc.assert_not_raising(HTTPError, lambda: tc.http_request('PUT', "/config_import/file/config/foo", "foo"))

    try:
        tc.http_request('PUT', "/config_import/file/config/foo/bar", "foo")
    except HTTPError as e:
        tc.assert_eq(500, e.code)
        tc.assert_contains("Failed to create file", e.msg)
    else:
        tc.fail("Expected 500, but request succeeded")


def test_config_import(tc: TestContext):
    print("hier")

    tc.restore_before_teardown('charge_limits/default_limits')
    tc.api('charge_limits/default_limits', {"duration": 10, "energy_wh": 100})

    tc.http_request('PUT', '/config_import/begin', timeout=5)

    # Tests /remove
    tc.http_request('PUT', "/config_import/file/config/foo", "foo")

    # Tests sub-directories
    tc.http_request('PUT', "/config_import/file/config/foo2/bar", "foo")

    # Tests real config + json_update of it
    tc.http_request('PUT', "/config_import/file/config/charge_limits_default_limits", '{"duration":1,"energy_wh":10}')
    tc.http_request('PUT', "/config_import/json_update/config/charge_limits_default_limits", '{"energy_wh":20}')

    actual = tc.list_dir('/config_import')
    for f in [
                "/config_import/config/foo",
                "/config_import/config/foo2/bar",
                "/config_import/config/charge_limits_default_limits",
                "/config_import/json_update/config/charge_limits_default_limits",
             ]:
        tc.assert_contains(f, actual)

    tc.http_request('DELETE', "/config_import/file/config/foo")

    actual = tc.list_dir('/config_import')
    for f in [
                "/config_import/config/foo2/bar",
                "/config_import/config/charge_limits_default_limits",
                "/config_import/json_update/config/charge_limits_default_limits",
             ]:
        tc.assert_contains(f, actual)

    tc.assert_("/config_import/config/foo" not in actual)

    old_boot_id = tc.api('event_log/boot_id/boot_id')

    tc.http_request('PUT', '/config_import/finish')

    tc.wait_for_reboot()

    tc.assert_ne(old_boot_id, tc.api('event_log/boot_id/boot_id'))

    try:
        tc.list_dir('/config_import')
    except HTTPError as e:
        tc.assert_eq(404, e.code)
    else:
        tc.fail("Expected 404, but request succeeded")

    tc.assert_eq(1, tc.api('charge_limits/default_limits/duration'))
    tc.assert_eq(20, tc.api('charge_limits/default_limits/energy_wh'))


if __name__ == "__main__":
    run_testsuite(locals())
