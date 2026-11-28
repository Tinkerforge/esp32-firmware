#!/usr/bin/env -S uv run --group tests --script

# Run over Ethernet: accepted AP configuration updates restart the access point.

from urllib.error import HTTPError

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext


def suite_setup(tc: TestContext):
    tc.restore_before_suite_teardown('wifi/ap_config')


def reject_config(tc: TestContext, config):
    previous = tc.api('wifi/ap_config')
    try:
        tc.api('wifi/ap_config_update', config)
        tc.fail('Invalid Ethernet sharing configuration was accepted')
    except HTTPError as e:
        tc.assert_eq(400, e.code)
    tc.assert_eq(previous, tc.api('wifi/ap_config'))


def test_sharing_requires_support(tc: TestContext):
    if 'wifi_ap_ethernet_sharing' in tc.api('info/features'):
        tc.skip('Firmware supports Ethernet sharing')
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=False, share_ethernet=True)
    reject_config(tc, config)


def test_sharing_requires_always_enabled_ap(tc: TestContext):
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=True, share_ethernet=True)
    reject_config(tc, config)
    config.update(enable_ap=False, ap_fallback_only=False)
    reject_config(tc, config)


def test_toggle_sharing(tc: TestContext):
    if 'wifi_ap_ethernet_sharing' not in tc.api('info/features'):
        tc.skip('Firmware does not support Ethernet sharing')
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=False)
    for enabled in (True, False):
        config['share_ethernet'] = enabled
        tc.api('wifi/ap_config_update', config)
        tc.assert_eq(config, tc.api('wifi/ap_config'))


if __name__ == '__main__':
    run_testsuite(locals())
