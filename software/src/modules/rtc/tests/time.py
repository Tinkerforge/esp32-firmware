#!/usr/bin/env -S uv run --group tests --script

# Tests for RTC module

import datetime
import time
from urllib.error import HTTPError
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


_original_ntp_config: dict | None = None


def _time_to_datetime(t: dict) -> datetime.datetime:
    return datetime.datetime(
        year=t["year"],
        month=t["month"],
        day=t["day"],
        hour=t["hour"],
        minute=t["minute"],
        second=t["second"],
    )


def _has_ntp(tc: TestContext) -> bool:
    try:
        tc.api("ntp/config")
        return True
    except HTTPError:
        return False


def suite_setup(tc: TestContext) -> None:
    global _original_ntp_config
    if _has_ntp(tc):
        _original_ntp_config = tc.api("ntp/config")


# Time consistency
def test_time_progresses(tc: TestContext) -> None:
    t1 = _time_to_datetime(tc.api("rtc/time"))
    time.sleep(1.5)
    t2 = _time_to_datetime(tc.api("rtc/time"))
    tc.assert_ge(t1, t2)  # t2 >= t1


def test_weekday_matches_date(tc: TestContext) -> None:
    t = tc.api("rtc/time")
    dt = _time_to_datetime(t)
    # Firmware convention: 0=Sunday
    # Python weekday(): 0=Monday
    expected_weekday = (dt.weekday() + 1) % 7
    tc.assert_eq(expected_weekday, t["weekday"])


# Set time via API
def test_set_time_via_api(tc: TestContext) -> None:
    """Disable NTP, reboot to clear quality deadline, set a known time, verify it takes effect."""
    ntp_was_enabled = False

    if _has_ntp(tc):
        ntp_cfg = tc.api("ntp/config")
        ntp_was_enabled = bool(ntp_cfg.get("enable", False))
        if ntp_was_enabled:
            ntp_cfg["enable"] = False
            tc.api("ntp/config_update", ntp_cfg)

    try:
        # Reboot to clear the quality deadline.
        tc.reboot()

        now            = datetime.datetime.now()
        target_hour    = (now.hour + 5) % 24
        target_weekday = (now.weekday() + 1) % 7

        tc.api(
            "rtc/time_update",
            {
                "year": now.year,
                "month": now.month,
                "day": now.day,
                "hour": target_hour,
                "minute": 0,
                "second": 0,
                "weekday": target_weekday,
            },
        )

        def check() -> None:
            t = tc.api("rtc/time")
            tc.assert_eq(target_hour, t["hour"])
            tc.assert_eq(0, t["minute"])
            tc.assert_le(5, t["second"])

        tc.wait_for(check)
    finally:
        # Re-enable NTP
        if ntp_was_enabled and _has_ntp(tc):
            cfg = tc.api("ntp/config")
            cfg["enable"] = True
            tc.api("ntp/config_update", cfg)


# Rejection of invalid timestamps
def test_reject_zero_time(tc: TestContext) -> None:
    before = _time_to_datetime(tc.api("rtc/time"))

    try:
        tc.api(
            "rtc/time_update",
            {
                "year": 0,
                "month": 0,
                "day": 0,
                "hour": 0,
                "minute": 0,
                "second": 0,
                "weekday": 0,
            },
        )
    except HTTPError:
        pass

    after = _time_to_datetime(tc.api("rtc/time"))

    tc.assert_gt(datetime.datetime(2020, 1, 1), after)

    diff = abs((after - before).total_seconds())
    tc.assert_le(5.0, diff)


def test_reject_ancient_time(tc: TestContext) -> None:
    before = _time_to_datetime(tc.api("rtc/time"))

    try:
        tc.api(
            "rtc/time_update",
            {
                "year": 2000,
                "month": 1,
                "day": 1,
                "hour": 0,
                "minute": 0,
                "second": 0,
                "weekday": 6,  # 2000-01-01 was a Saturday
            },
        )
    except HTTPError:
        pass

    after = _time_to_datetime(tc.api("rtc/time"))

    tc.assert_gt(datetime.datetime(2020, 1, 1), after)

    diff = abs((after - before).total_seconds())
    tc.assert_le(5.0, diff)


def suite_teardown(tc: TestContext) -> None:
    if _original_ntp_config is not None:
        tc.api("ntp/config_update", _original_ntp_config)


if __name__ == "__main__":
    run_testsuite(locals())
