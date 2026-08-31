#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import subprocess
import time

import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, "software")

from _common import CERTS_DIR, IsoTestEnvironment, sdp_request
from _ocsp_gating import (
    TLS_EARLY_DATA,
    TLS_STATUS_REQUEST_V2,
    capture_tls13_server_flight,
    parse_extension_vector,
)
from _iso2_chain_selection import probe as probe_tls12_flight
from software.test_runner.test_context import TestContext, run_testsuite

environment = None

TLS_STATUS_REQUEST = 5
TLS_SIGNATURE_ALGORITHMS = 13
TLS_CERTIFICATE_AUTHORITIES = 47
TLS_OID_FILTERS = 48


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
    "tls13_aes256_gcm": (
        True,
        ["-tls1_3", "-ciphersuites", "TLS_AES_256_GCM_SHA384"],
        "TLS_AES_256_GCM_SHA384",
    ),
    "tls13_chacha20": (
        True,
        ["-tls1_3", "-ciphersuites", "TLS_CHACHA20_POLY1305_SHA256"],
        "TLS_CHACHA20_POLY1305_SHA256",
    ),
    "tls13_server_pref_client_aes_first": (
        True,
        [
            "-tls1_3", "-ciphersuites",
            "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256",
        ],
        "TLS_AES_256_GCM_SHA384",
    ),
    "tls13_server_pref_client_chacha_first": (
        True,
        [
            "-tls1_3", "-ciphersuites",
            "TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384",
        ],
        "TLS_AES_256_GCM_SHA384",
    ),
    "tls13_disjoint_cipher_refused": (
        False,
        ["-tls1_3", "-ciphersuites", "TLS_AES_128_GCM_SHA256"],
    ),
    "tls13_max_fragment_length_512": (
        True,
        ["-tls1_3", "-maxfraglen", "512", "-tlsextdebug"],
        None,
        "TLS server extension \"max fragment length\" (id=1), len=1",
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


def probe(tc: TestContext, expect_success: bool, arguments, expected_cipher=None, expected_output=None):
    assert environment is not None
    response = sdp_request(environment.iface, expected_from=environment.secc_ll)
    tc.assert_(response is not None)
    endpoint = f"[{response['secc_ll']}%{environment.iface}]:{response['port']}"

    command = ["openssl", "s_client", "-connect", endpoint, *arguments]
    if "-tls1_3" in arguments:
        command += [
            "-CAfile", str(CERTS_DIR / "iso20/certs/v2gRootCACert.pem"),
            "-cert", str(CERTS_DIR / "iso20/certs/oemLeafCert.pem"),
            "-cert_chain", str(CERTS_DIR / "iso20/certs/oemCertChain.pem"),
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
        check=False,
    )
    output = result.stdout + result.stderr
    tc.dbg(output)
    handshake_completed = bool(tc.assert_search(r"Cipher is (TLS_|ECDHE-)", output)) if expect_success else (
        "Cipher is TLS_" in output or "Cipher is ECDHE-" in output)
    succeeded = result.returncode == 0 and "Verify return code: 0 (ok)" in output
    if expect_success:
        tc.assert_(succeeded)
        if expected_cipher is not None:
            tc.assert_search(rf"Cipher is {expected_cipher}\b", output)
        if expected_output is not None:
            tc.assert_(expected_output in output)
        if "-tls1_3" in arguments:
            # V2G20-2401/2403: all V2G/OEM roots, with C/O/OU/CN/serial
            # in that exact order. The dev roots also prove empty OU handling.
            ca_names = output.split(
                "Acceptable client certificate CA names\n", 1)[1].split(
                "Requested Signature Algorithms:", 1)[0].strip().splitlines()
            tc.assert_eq([
                "C = DE, O = WARP, OU = , CN = OEMRootCA, serialNumber = A569",
                "C = DE, O = WARP, OU = , CN = V2GRootCA, serialNumber = 5749",
            ], ca_names)
    else:
        tc.assert_false(handshake_completed and result.returncode == 0)
    time.sleep(0.2)


def certificate_request_extensions(flight):
    certificate_request = next(
        message for message_type, message in flight.handshake_messages
        if message_type == 13
    )
    context_length = certificate_request[4]
    return parse_extension_vector(certificate_request[4:], 1 + context_length)


def test_tls13_ignores_status_request_v2(tc: TestContext):
    assert environment is not None
    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    try:
        tc.api("ocpp/config_update", disabled, timeout=5)
        time.sleep(1)
        environment.reset_session()
        flight = capture_tls13_server_flight(
            environment.host,
            environment.iface,
            request_status=False,
            request_status_v2=True,
        )

        tc.assert_false(TLS_STATUS_REQUEST_V2 in flight.server_hello_extensions)
        message_types = [message_type for message_type, _ in flight.handshake_messages]
        tc.assert_(8 in message_types)
        tc.assert_(13 in message_types)
        tc.assert_(11 in message_types)

        encrypted_extensions = next(
            message for message_type, message in flight.handshake_messages
            if message_type == 8
        )
        tc.assert_false(TLS_STATUS_REQUEST_V2 in parse_extension_vector(encrypted_extensions[4:], 0))

        request_extensions = certificate_request_extensions(flight)
        tc.assert_false(TLS_STATUS_REQUEST_V2 in request_extensions)
        tc.assert_(len(flight.certificate_entries) > 0)
        for _, extensions in flight.certificate_entries:
            tc.assert_false(TLS_STATUS_REQUEST_V2 in extensions)
    finally:
        tc.api("ocpp/config_update", saved_ocpp, timeout=15)


def test_tls13_certificate_request_omits_forbidden_extensions(tc: TestContext):
    assert environment is not None
    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    try:
        tc.api("ocpp/config_update", disabled, timeout=5)
        time.sleep(1)
        environment.reset_session()
        flight = capture_tls13_server_flight(
            environment.host,
            environment.iface,
            request_status=True,
        )

        request_extensions = certificate_request_extensions(flight)
        tc.assert_(TLS_SIGNATURE_ALGORITHMS in request_extensions)
        tc.assert_(TLS_CERTIFICATE_AUTHORITIES in request_extensions)
        tc.assert_false(TLS_STATUS_REQUEST in request_extensions)
        tc.assert_false(TLS_OID_FILTERS in request_extensions)
    finally:
        tc.api("ocpp/config_update", saved_ocpp, timeout=15)


def test_tls12_selects_null_compression(tc: TestContext):
    assert environment is not None
    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    try:
        tc.api("ocpp/config_update", disabled, timeout=5)
        time.sleep(1)
        environment.reset_session()
        flight = probe_tls12_flight(
            environment.host,
            environment.iface,
            compression_methods=b"\x01\x00",  # DEFLATE followed by null.
        )
        tc.assert_(len(flight.certificates) > 0)
    finally:
        tc.api("ocpp/config_update", saved_ocpp, timeout=15)


def test_tls13_rejects_early_data(tc: TestContext):
    assert environment is not None
    saved_ocpp = tc.api("ocpp/config")
    disabled = dict(saved_ocpp)
    disabled["enable"] = False
    try:
        tc.api("ocpp/config_update", disabled, timeout=5)
        time.sleep(1)
        environment.reset_session()
        flight = capture_tls13_server_flight(
            environment.host,
            environment.iface,
            request_status=False,
            request_early_data=True,
        )

        # No pre_shared_key in ServerHello means the unsolicited extension
        # did not turn the fresh connection into a PSK handshake.
        tc.assert_false(41 in flight.server_hello_extensions)
        message_types = [message_type for message_type, _ in flight.handshake_messages]
        tc.assert_(8 in message_types)
        tc.assert_(13 in message_types)
        tc.assert_(11 in message_types)
        encrypted_extensions = next(
            message for message_type, message in flight.handshake_messages
            if message_type == 8
        )
        tc.assert_false(TLS_EARLY_DATA in parse_extension_vector(encrypted_extensions[4:], 0))
    finally:
        tc.api("ocpp/config_update", saved_ocpp, timeout=15)


def generate_tests():
    return {
        f"test_{name}": lambda tc, case=case: probe(tc, *case)
        for name, case in CASES.items()
    }


if __name__ == "__main__":
    run_testsuite(dict(locals(), **generate_tests()))
