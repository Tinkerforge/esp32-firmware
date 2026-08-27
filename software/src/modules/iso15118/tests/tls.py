#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import subprocess
import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import CERTS_DIR, IsoTestEnvironment, sdp_request


environment = None


CASES = {
    "tls13_default_groups": (True, ["-tls1_3"]),
    "tls13_p521": (True, ["-tls1_3", "-groups", "P-521"]),
    "tls13_x448": (True, ["-tls1_3", "-groups", "X448"]),
    "tls13_hrr_to_p521": (True, ["-tls1_3", "-groups", "P-256:P-521"]),
    "tls13_p256_only_refused": (False, ["-tls1_3", "-groups", "P-256"]),
    "tls13_x25519_only_refused": (False, ["-tls1_3", "-groups", "X25519"]),
    "tls13_secp521r1_sha512": (
        True, ["-tls1_3", "-sigalgs", "ecdsa_secp521r1_sha512"]
    ),
    "tls13_secp256r1_sha256_refused": (
        False, ["-tls1_3", "-sigalgs", "ecdsa_secp256r1_sha256"]
    ),
    "tls13_rsa_pss_refused": (
        False, ["-tls1_3", "-sigalgs", "rsa_pss_rsae_sha256"]
    ),
    "tls12_p256_ecdhe_ecdsa": (
        True,
        [
            "-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256",
            "-curves", "P-256", "-sigalgs", "ECDSA+SHA256",
        ],
    ),
    "tls12_default_groups": (
        True, ["-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256"]
    ),
    "tls12_x25519_only_refused": (
        False,
        ["-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256", "-curves", "X25519"],
    ),
    "tls12_sha1_only_refused": (
        False,
        [
            "-tls1_2", "-cipher", "ECDHE-ECDSA-AES128-SHA256:@SECLEVEL=0",
            "-curves", "P-256", "-sigalgs", "ECDSA+SHA1",
        ],
    ),
    "tls12_disjoint_cipher_refused": (
        False, ["-tls1_2", "-cipher", "ECDHE-RSA-AES256-GCM-SHA384"]
    ),
}


def suite_setup(tc: TestContext):
    global environment
    environment = IsoTestEnvironment(tc)
    environment.start()


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    if environment is not None:
        environment.stop()


def probe(tc: TestContext, expect_success: bool, arguments):
    assert environment is not None
    response = sdp_request(environment.iface, expected_from=environment.secc_ll)
    tc.assert_(response is not None)
    endpoint = f"[{response['secc_ll']}%{environment.iface}]:{response['port']}"

    command = ["openssl", "s_client", "-connect", endpoint, *arguments]
    if "-tls1_3" in arguments:
        command += [
            "-CAfile", str(CERTS_DIR / "iso20/certs/v2gRootCACert.pem"),
            "-cert", str(CERTS_DIR / "iso20/certs/oemCertChain.pem"),
            "-key", str(CERTS_DIR / "iso20/private_keys/oemLeaf.key"),
            "-pass", "pass:12345",
        ]
    else:
        command += ["-CAfile", str(CERTS_DIR / "iso2/certs/v2gRootCACert.pem")]

    result = subprocess.run(
        command,
        input="Q\n",
        capture_output=True,
        text=True,
        timeout=60,
    )
    output = result.stdout + result.stderr
    tc.dbg(output)
    handshake_completed = bool(tc.assert_search(r"Cipher is (TLS_|ECDHE-)", output)) if expect_success else (
        "Cipher is TLS_" in output or "Cipher is ECDHE-" in output)
    succeeded = result.returncode == 0 and "Verify return code: 0 (ok)" in output
    if expect_success:
        tc.assert_(succeeded)
    else:
        tc.assert_false(handshake_completed and result.returncode == 0)
    time.sleep(0.2)


def generate_tests():
    return {
        f"test_{name}": lambda tc, case=case: probe(tc, *case)
        for name, case in CASES.items()
    }


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
