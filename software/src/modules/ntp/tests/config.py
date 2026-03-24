#!/usr/bin/env -S uv run --group tests --script

# Tests for NTP config validation and save-without-reboot

from urllib.error import HTTPError
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


_original_config = None


def make_config(
    enable=True,
    use_dhcp=True,
    timezone="Europe/Berlin",
    server="time.cloudflare.com",
    server2="time.google.com",
):
    """Build an NTP config payload."""
    return {
        "enable": enable,
        "use_dhcp": use_dhcp,
        "timezone": timezone,
        "server": server,
        "server2": server2,
    }


def suite_setup(tc: TestContext):
    global _original_config
    _original_config = tc.api("ntp/config")


def suite_teardown(tc: TestContext):
    if _original_config is not None:
        tc.api("ntp/config_update", _original_config)


def test_read_config(tc: TestContext):
    config = tc.api("ntp/config")

    for field in ["enable", "use_dhcp", "timezone", "server", "server2"]:
        tc.assert_in(config, field)


def test_read_state(tc: TestContext):
    state = tc.api("ntp/state")

    for field in ["synced", "time"]:
        tc.assert_in(state, field)

    tc.assert_eq(bool, type(state["synced"]))
    tc.assert_eq(int, type(state["time"]))


def test_reject_bad_timezone(tc: TestContext):
    config = make_config(timezone="Not/A_Real_Timezone")

    try:
        tc.api("ntp/config_update", config)
        tc.fail("Expected rejection for bad timezone, got HTTP 200")
    except HTTPError:
        pass


def test_accept_valid_timezone_change(tc: TestContext):
    assert _original_config is not None
    orig_tz = _original_config.get("timezone", "Europe/Berlin")
    new_tz = "America/New_York" if orig_tz != "America/New_York" else "Asia/Tokyo"

    config = tc.api("ntp/config")
    config["timezone"] = new_tz
    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_eq(new_tz, readback["timezone"])


def test_accept_server_change(tc: TestContext):
    config = tc.api("ntp/config")
    config["server"] = "pool.ntp.org"
    config["server2"] = "time.nist.gov"
    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_eq("pool.ntp.org", readback["server"])
    tc.assert_eq("time.nist.gov", readback["server2"])


def test_disable_ntp(tc: TestContext):
    config = tc.api("ntp/config")
    config["enable"] = False
    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_false(readback["enable"])

    state = tc.api("ntp/state")
    tc.assert_false(state["synced"])


def test_re_enable_ntp(tc: TestContext):
    # Ensure NTP is disabled first.
    config = tc.api("ntp/config")
    if config["enable"]:
        config["enable"] = False
        tc.api("ntp/config_update", config)

    # Re-enable.
    config["enable"] = True
    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_true(readback["enable"])


def test_toggle_use_dhcp(tc: TestContext):
    config = tc.api("ntp/config")
    orig_use_dhcp = config["use_dhcp"]
    config["use_dhcp"] = not orig_use_dhcp

    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_eq(not orig_use_dhcp, readback["use_dhcp"])


def test_config_persisted(tc: TestContext):
    target = make_config(
        enable=True,
        use_dhcp=False,
        timezone="Asia/Tokyo",
        server="pool.ntp.org",
        server2="time.nist.gov",
    )
    tc.api("ntp/config_update", target)

    readback = tc.api("ntp/config")
    for field in ["enable", "use_dhcp", "timezone", "server", "server2"]:
        tc.assert_eq(target[field], readback.get(field))


def test_rejected_config_not_saved(tc: TestContext):
    good = make_config(
        enable=True,
        timezone="Europe/London",
        server="time.cloudflare.com",
        server2="time.google.com",
    )
    tc.api("ntp/config_update", good)

    # Try to set a bad config (invalid timezone).
    bad = make_config(timezone="Invalid/Bogus_Zone")
    try:
        tc.api("ntp/config_update", bad)
        tc.fail("Bad config was unexpectedly accepted")
    except HTTPError:
        pass

    # Verify the good config is still there.
    readback = tc.api("ntp/config")
    tc.assert_eq("Europe/London", readback["timezone"])


def test_rapid_config_changes(tc: TestContext):
    timezones = [
        "America/New_York",
        "Asia/Tokyo",
        "Europe/London",
        "America/Los_Angeles",
        "Australia/Sydney",
    ]

    accepted = 0
    for i, tz in enumerate(timezones):
        config = make_config(
            enable=True,
            timezone=tz,
            server=f"ntp{i}.example.com",
            server2=f"ntp{i}b.example.com",
        )
        try:
            tc.api("ntp/config_update", config)
            accepted += 1
        except Exception:
            pass

    tc.assert_gt(0, accepted)

    # Device should still be reachable.
    state = tc.api("ntp/state")
    tc.assert_in(state, "synced")


def test_empty_server_strings(tc: TestContext):
    # Both servers empty.
    config = make_config(server="", server2="")
    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_eq("", readback["server"])
    tc.assert_eq("", readback["server2"])

    # Only server2 set.
    config = make_config(server="", server2="time.google.com")
    tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_eq("", readback["server"])
    tc.assert_eq("time.google.com", readback["server2"])


def test_multiple_timezone_changes(tc: TestContext):
    timezones = ["Europe/Berlin", "America/Chicago", "Asia/Kolkata", "Pacific/Auckland"]

    for tz in timezones:
        config = tc.api("ntp/config")
        config["timezone"] = tz
        tc.api("ntp/config_update", config)

        readback = tc.api("ntp/config")
        tc.assert_eq(tz, readback["timezone"])


def test_disable_enable_cycle(tc: TestContext):
    config = tc.api("ntp/config")

    for _ in range(3):
        config["enable"] = False
        tc.api("ntp/config_update", config)

        config["enable"] = True
        tc.api("ntp/config_update", config)

    readback = tc.api("ntp/config")
    tc.assert_true(readback["enable"])


if __name__ == "__main__":
    run_testsuite(locals())
