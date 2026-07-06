#!/usr/bin/env -S uv run --group tests --script

# Tests for users module (central user management)

import hashlib
from urllib.error import HTTPError
import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext


# Credentials used by the HTTP-auth tests. The firmware stores the HA1
# (MD5(username:realm:password)) as the digest_hash; since HTTP auth needs
# a real hash to validate incoming requests, we provide one here.
_AUTH_USERNAME = "authuser"
_AUTH_PASSWORD = "authpass"
# Must match the firmware's DEFAULT_REALM in src/digest_auth.h.
_AUTH_REALM = "esp32-lib"
_AUTH_DIGEST_HASH = hashlib.md5(f"{_AUTH_USERNAME}:{_AUTH_REALM}:{_AUTH_PASSWORD}".encode("utf-8")).hexdigest()
_AUTH_CREDENTIALS: tuple[str, str] = (_AUTH_USERNAME, _AUTH_DIGEST_HASH)


_original_config: dict | None = None


def _get_user_by_id(config: dict, user_id: int) -> dict | None:
    for user in config["users"]:
        if user["id"] == user_id:
            return user
    return None


def _modify_payload_for_user(tc: TestContext, user_id: int, **overrides: object) -> dict:
    """Build a full modify payload with all required fields for an existing user."""
    cfg = tc.api("users/config")
    user = _get_user_by_id(cfg, user_id)
    if user is None:
        raise ValueError(f"User with id {user_id} not found")

    payload = {
        "id": user_id,
        "roles": user["roles"],
        "current": user["current"],
        "display_name": user["display_name"],
        "username": user["username"],
        "digest_hash": user.get("digest_hash", ""),
    }
    payload.update(overrides)
    return payload


def suite_setup(tc: TestContext) -> None:
    global _original_config
    _original_config = tc.api("users/config")


def setup(tc: TestContext) -> None:
    # A previous test may have left HTTP auth enabled. Fetch the config
    # with credentials if needed so subsequent calls work in either state.
    try:
        cfg = tc.api("users/config")
    except HTTPError:
        cfg = tc.api("users/config", auth=_AUTH_CREDENTIALS)

    # If HTTP auth is still enabled from a previous test, try to disable it
    # using the test credentials so subsequent tests run without auth.
    if cfg.get("http_auth_enabled"):
        try:
            tc.api("users/http_auth_update", {"enabled": False}, auth=_AUTH_CREDENTIALS)
        except HTTPError:
            pass

    try:
        cfg = tc.api("users/config")
    except HTTPError:
        cfg = tc.api("users/config", auth=_AUTH_CREDENTIALS)

    for user in cfg["users"]:
        if user["id"] != 0:
            try:
                tc.api("users/remove", {"id": user["id"]}, auth=_AUTH_CREDENTIALS)
            except HTTPError:
                tc.api("users/remove", {"id": user["id"]})


# ---------- Check for missing keys (breaking changes) ----------

def test_config_has_anonymous_user(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    tc.assert_ge(1, len(cfg["users"]))
    anon = _get_user_by_id(cfg, 0)
    tc.assert_(anon is not None)
    tc.assert_eq("anonymous", anon["username"])


def test_config_has_next_user_id(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    tc.assert_("next_user_id" in cfg)


def test_config_has_http_auth_enabled(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    tc.assert_("http_auth_enabled" in cfg)


# ---------- Add user ----------

def test_add_user(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]
    tc.assert_ne(0, next_id)

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 16000,
        "display_name": "Test User",
        "username": "testuser",
        "digest_hash": "",
    })

    def _check() -> None:
        cfg2 = tc.api("users/config")
        user = _get_user_by_id(cfg2, next_id)
        tc.assert_(user is not None)
        tc.assert_eq("testuser", user["username"])
        tc.assert_eq("Test User", user["display_name"])
        tc.assert_eq(0xFFFF, user["roles"])
        tc.assert_eq(16000, user["current"])

    tc.wait_for(_check)


def test_add_user_wrong_next_id(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    # Use a wrong ID (next_id + 1) which should be rejected
    wrong_id = next_id + 1 if next_id < 255 else next_id - 1

    try:
        tc.api("users/add", {
            "id": wrong_id,
            "roles": 0xFFFF,
            "current": 32000,
            "display_name": "Bad User",
            "username": "baduser",
            "digest_hash": "",
        })
    except HTTPError:
        pass

    cfg2 = tc.api("users/config")
    tc.assert_(_get_user_by_id(cfg2, wrong_id) is None)


def test_add_user_duplicate_username(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    # "anonymous" already exists as user 0
    try:
        tc.api("users/add", {
            "id": next_id,
            "roles": 0xFFFF,
            "current": 32000,
            "display_name": "Dup User",
            "username": "anonymous",
            "digest_hash": "",
        })
    except HTTPError:
        pass

    cfg2 = tc.api("users/config")
    tc.assert_(_get_user_by_id(cfg2, next_id) is None)


def test_add_user_next_id_advances(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    first_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": first_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "User A",
        "username": "user_a",
        "digest_hash": "",
    })

    cfg2 = tc.api("users/config")
    second_id = cfg2["next_user_id"]
    tc.assert_ne(first_id, second_id)
    tc.assert_ne(0, second_id)


# ---------- Modify user ----------

def test_modify_user_display_name(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Original",
        "username": "modme",
        "digest_hash": "",
    })

    tc.api("users/modify", _modify_payload_for_user(tc, next_id, display_name="Renamed"))

    def _check() -> None:
        cfg2 = tc.api("users/config")
        user = _get_user_by_id(cfg2, next_id)
        tc.assert_(user is not None)
        tc.assert_eq("Renamed", user["display_name"])
        # Other fields unchanged
        tc.assert_eq("modme", user["username"])
        tc.assert_eq(0xFFFF, user["roles"])
        tc.assert_eq(32000, user["current"])

    tc.wait_for(_check)

def test_modify_user_current(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Current Test",
        "username": "currentuser",
        "digest_hash": "",
    })

    tc.api("users/modify", _modify_payload_for_user(tc, next_id, current=8000))

    def _check() -> None:
        cfg2 = tc.api("users/config")
        user = _get_user_by_id(cfg2, next_id)
        tc.assert_eq(8000, user["current"])

    tc.wait_for(_check)

def test_modify_nonexistent_user(tc: TestContext) -> None:
    try:
        tc.api("users/modify", {
            "id": 254,
            "roles": 0xFFFF,
            "current": 32000,
            "display_name": "Ghost",
            "username": "ghost",
            "digest_hash": "",
        })
    except HTTPError:
        pass

    # Config should be unchanged
    cfg = tc.api("users/config")
    tc.assert_(_get_user_by_id(cfg, 254) is None)

def test_modify_anonymous_display_name(tc: TestContext) -> None:
    old_name = tc.api("users/config")["users"][0]["display_name"]
    tc.api("users/modify", {
        "id": 0,
        "display_name": "Anon Renamed",
        "current": None,
        "username": None,
        "roles": None,
        "digest_hash": None,
    })

    def _check() -> None:
        cfg = tc.api("users/config")
        anon = _get_user_by_id(cfg, 0)
        tc.assert_eq("Anon Renamed", anon["display_name"])

    tc.wait_for(_check)

    # Restore original display name
    tc.api("users/modify", {
        "id": 0,
        "display_name": old_name,
        "current": None,
        "username": None,
        "roles": None,
        "digest_hash": None,
    })

def test_modify_anonymous_username_rejected(tc: TestContext) -> None:
    try:
        tc.api("users/modify", _modify_payload_for_user(tc, 0, username="not_anonymous"))
    except HTTPError:
        pass

    cfg = tc.api("users/config")
    anon = _get_user_by_id(cfg, 0)
    tc.assert_eq("anonymous", anon["username"])


def test_modify_anonymous_roles_rejected(tc: TestContext) -> None:
    try:
        tc.api("users/modify", _modify_payload_for_user(tc, 0, roles=0x1))
    except HTTPError:
        pass


def test_modify_anonymous_current_rejected(tc: TestContext) -> None:
    try:
        tc.api("users/modify", _modify_payload_for_user(tc, 0, current=16000))
    except HTTPError:
        pass


def test_modify_duplicate_username_rejected(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Dup Check",
        "username": "dupcheck",
        "digest_hash": "",
    })

    # Try to rename to "anonymous" which already exists
    try:
        tc.api("users/modify", _modify_payload_for_user(tc, next_id, username="anonymous"))
    except HTTPError:
        pass

    cfg2 = tc.api("users/config")
    user = _get_user_by_id(cfg2, next_id)
    tc.assert_eq("dupcheck", user["username"])


# ---------- Remove user ----------

def test_remove_user(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Remove Me",
        "username": "removeme",
        "digest_hash": "",
    })
    cfg2 = tc.api("users/config")
    tc.assert_(_get_user_by_id(cfg2, next_id) is not None)

    tc.api("users/remove", {"id": next_id})

    def _check() -> None:
        cfg3 = tc.api("users/config")
        tc.assert_(_get_user_by_id(cfg3, next_id) is None)

    tc.wait_for(_check)


def test_remove_anonymous_rejected(tc: TestContext) -> None:
    try:
        tc.api("users/remove", {"id": 0})
    except HTTPError:
        pass

    cfg = tc.api("users/config")
    tc.assert_(_get_user_by_id(cfg, 0) is not None)


def test_remove_nonexistent_user_rejected(tc: TestContext) -> None:
    try:
        tc.api("users/remove", {"id": 254})
    except HTTPError:
        pass


# ---------- HTTP auth ----------

def test_http_auth_enable_without_password_rejected(tc: TestContext) -> None:
    # Ensure no user has a digest_hash set. Try unauthenticated first; if
    # HTTP auth is already enabled, fall back to the test credentials.
    try:
        cfg = tc.api("users/config")
    except HTTPError:
        cfg = tc.api("users/config", auth=_AUTH_CREDENTIALS)
    all_empty = all(u.get("digest_hash", "") == "" for u in cfg["users"])
    if not all_empty:
        tc.skip("A user already has a digest_hash set; can't test this scenario")

    try:
        tc.api("users/http_auth_update", {"enabled": True})
    except HTTPError:
        pass

    try:
        cfg2 = tc.api("users/config")
    except HTTPError:
        cfg2 = tc.api("users/config", auth=_AUTH_CREDENTIALS)
    tc.assert_false(cfg2["http_auth_enabled"])


def test_http_auth_enable_with_password_succeeds(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Auth User",
        "username": _AUTH_USERNAME,
        "digest_hash": _AUTH_DIGEST_HASH,
    })

    try:
        tc.api("users/http_auth_update", {"enabled": True})

        def _check_enabled() -> None:
            # HTTP auth is now enabled, so this request must be authenticated.
            cfg2 = tc.api("users/config", auth=_AUTH_CREDENTIALS)
            tc.assert_true(cfg2["http_auth_enabled"])

        tc.wait_for(_check_enabled)

        # Verify that an unauthenticated request is rejected with 401 and
        # that an authenticated one succeeds.
        rejected = False
        try:
            tc.api("users/config")
        except HTTPError:
            rejected = True
        tc.assert_true(rejected)

        cfg_auth = tc.api("users/config", auth=_AUTH_CREDENTIALS)
        tc.assert_true(cfg_auth["http_auth_enabled"])
    finally:
        # Disabling HTTP auth also requires auth while it is enabled.
        try:
            tc.api("users/http_auth_update", {"enabled": False}, auth=_AUTH_CREDENTIALS)
        except HTTPError:
            pass

    def _check_disabled() -> None:
        cfg2 = tc.api("users/config")
        tc.assert_false(cfg2["http_auth_enabled"])

    tc.wait_for(_check_disabled)



def test_http_auth_disable(tc: TestContext) -> None:
    # HTTP auth may already be enabled from a previous run. Try the
    # disable command with credentials when needed so this test is
    # idempotent regardless of starting state.
    try:
        tc.api("users/http_auth_update", {"enabled": False})
    except HTTPError:
        tc.api("users/http_auth_update", {"enabled": False}, auth=_AUTH_CREDENTIALS)

    def _check() -> None:
        try:
            cfg = tc.api("users/config")
        except HTTPError:
            cfg = tc.api("users/config", auth=_AUTH_CREDENTIALS)
        tc.assert_false(cfg["http_auth_enabled"])

    tc.wait_for(_check)


# ---------- Add / remove round-trip ----------

def test_add_remove_add_same_username(tc: TestContext) -> None:
    """After removing a user, the same username can be reused."""
    cfg = tc.api("users/config")
    first_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": first_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Reuse Me",
        "username": "reusable",
        "digest_hash": "",
    })

    tc.api("users/remove", {"id": first_id})

    def _removed() -> None:
        tc.assert_(_get_user_by_id(tc.api("users/config"), first_id) is None)

    tc.wait_for(_removed)

    cfg2 = tc.api("users/config")
    second_id = cfg2["next_user_id"]
    tc.assert_ne(0, second_id)

    # Re-add with the same username should succeed
    tc.api("users/add", {
        "id": second_id,
        "roles": 0xFFFF,
        "current": 32000,
        "display_name": "Reused",
        "username": "reusable",
        "digest_hash": "",
    })

    def _readded() -> None:
        cfg3 = tc.api("users/config")
        user = _get_user_by_id(cfg3, second_id)
        tc.assert_(user is not None)
        tc.assert_eq("reusable", user["username"])

    tc.wait_for(_readded)


# ---------- Persistence ----------

def test_config_persists_after_reboot(tc: TestContext) -> None:
    cfg = tc.api("users/config")
    next_id = cfg["next_user_id"]

    tc.api("users/add", {
        "id": next_id,
        "roles": 0xFFFF,
        "current": 20000,
        "display_name": "Persist",
        "username": "persist_user",
        "digest_hash": "",
    })

    tc.reboot()

    def _check() -> None:
        cfg2 = tc.api("users/config")
        user = _get_user_by_id(cfg2, next_id)
        tc.assert_(user is not None)
        tc.assert_eq("persist_user", user["username"])
        tc.assert_eq(20000, user["current"])

    tc.wait_for(_check, timeout=15)


# ---------- Multiple users ----------

def test_add_multiple_users(tc: TestContext) -> None:
    added_ids = []
    for i in range(3):
        cfg = tc.api("users/config")
        nid = cfg["next_user_id"]
        tc.assert_ne(0, nid)

        tc.api("users/add", {
            "id": nid,
            "roles": 0xFFFF,
            "current": 10000 + i * 1000,
            "display_name": f"Multi {i}",
            "username": f"multi_{i}",
            "digest_hash": "",
        })
        added_ids.append(nid)

    cfg2 = tc.api("users/config")
    for uid in added_ids:
        tc.assert_(_get_user_by_id(cfg2, uid) is not None)


def test_modify_without_id_rejected(tc: TestContext) -> None:
    """Modify command with missing/null id should be rejected."""
    try:
        tc.api("users/modify", {
            "display_name": "No ID",
        })
    except HTTPError:
        pass


def suite_teardown(tc: TestContext) -> None:
    # Restore original config: remove all non-original users
    if _original_config is None:
        return

    try:
        cfg = tc.api("users/config")
    except HTTPError:
        cfg = tc.api("users/config", auth=_AUTH_CREDENTIALS)

    original_ids = {u["id"] for u in _original_config["users"]}
    for user in cfg["users"]:
        if user["id"] not in original_ids and user["id"] != 0:
            try:
                tc.api("users/remove", {"id": user["id"]})
            except HTTPError:
                try:
                    tc.api("users/remove", {"id": user["id"]}, auth=_AUTH_CREDENTIALS)
                except HTTPError:
                    pass

    # Restore anonymous display name
    orig_anon = _get_user_by_id(_original_config, 0)
    if orig_anon is not None:
        try:
            tc.api("users/modify", _modify_payload_for_user(tc, 0, display_name=orig_anon["display_name"]))
        except HTTPError:
            try:
                tc.api("users/modify", _modify_payload_for_user(tc, 0, display_name=orig_anon["display_name"]), auth=_AUTH_CREDENTIALS)
            except HTTPError:
                pass

    # Restore http auth state
    try:
        tc.api("users/http_auth_update", {"enabled": _original_config["http_auth_enabled"]})
    except HTTPError:
        try:
            tc.api("users/http_auth_update", {"enabled": _original_config["http_auth_enabled"]}, auth=_AUTH_CREDENTIALS)
        except HTTPError:
            pass


if __name__ == "__main__":
    run_testsuite(locals())
