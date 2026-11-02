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
    config.update(enable_ap=True, ap_fallback_only=False, share_ethernet=True, ethernet_passthrough=False)
    reject_config(tc, config)


def test_sharing_requires_always_enabled_ap(tc: TestContext):
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=True, share_ethernet=True, ethernet_passthrough=False)
    reject_config(tc, config)
    config.update(enable_ap=False, ap_fallback_only=False)
    reject_config(tc, config)


def test_toggle_sharing(tc: TestContext):
    if 'wifi_ap_ethernet_sharing' not in tc.api('info/features'):
        tc.skip('Firmware does not support Ethernet sharing')
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=False, ethernet_passthrough=False)
    for enabled in (True, False):
        config['share_ethernet'] = enabled
        tc.api('wifi/ap_config_update', config)
        tc.assert_eq(config, tc.api('wifi/ap_config'))


def test_passthrough_requires_support(tc: TestContext):
    if 'wifi_ap_ethernet_passthrough' in tc.api('info/features'):
        tc.skip('Firmware supports Ethernet passthrough')
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=False, share_ethernet=False, ethernet_passthrough=True)
    reject_config(tc, config)


def test_passthrough_requires_always_enabled_ap(tc: TestContext):
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=True, share_ethernet=False, ethernet_passthrough=True)
    reject_config(tc, config)
    config.update(enable_ap=False, ap_fallback_only=False)
    reject_config(tc, config)


def test_passthrough_excludes_nat(tc: TestContext):
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=False, share_ethernet=True, ethernet_passthrough=True)
    reject_config(tc, config)


def test_switch_sharing_modes(tc: TestContext):
    features = tc.api('info/features')
    if 'wifi_ap_ethernet_passthrough' not in features:
        tc.skip('Firmware does not support Ethernet passthrough')
    config = tc.api('wifi/ap_config')
    config.update(enable_ap=True, ap_fallback_only=False)
    modes = [(False, True), (False, False)]
    if 'wifi_ap_ethernet_sharing' in features:
        modes += [(True, False), (False, True), (True, False), (False, False)]
    for nat, passthrough in modes:
        config.update(share_ethernet=nat, ethernet_passthrough=passthrough)
        tc.api('wifi/ap_config_update', config)
        tc.assert_eq(config, tc.api('wifi/ap_config'))


if __name__ == '__main__':
    run_testsuite(locals())
