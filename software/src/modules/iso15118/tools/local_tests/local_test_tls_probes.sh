#!/usr/bin/env bash
# TLS group, cipher and signature algorithm probes against the ISO 15118 TLS
# server of a WARP4 in ISO debug mode (HUB20-533-001..005), using the dev PKI
# from ../certs/output.
#
# Usage: ./local_test_tls_probes.sh <charger-ip>
set -u
cd "$(dirname "$0")"

CHARGER=${1:?usage: $0 <charger-ip>}

python3 - "$CHARGER" <<'EOF'
import sys
import common
common.require_iso_tls_config(sys.argv[1])
common.enable_debug_mode(sys.argv[1])
EOF
sleep 1

eval "$(python3 sdp_probe.py --charger "$CHARGER" --retries 5 --eval)" || { echo "SDP discovery failed"; exit 1; }
SECC="${SECC_LL}%${IFACE}"
echo "SECC at [$SECC]:$SECC_PORT"

CERTS="$(cd ../certs/output && pwd)"
OEM20="$CERTS/iso20/certs/oemCertChain.pem"
OEMKEY20="$CERTS/iso20/private_keys/oemLeaf.key"
OEMPASS="pass:$(cat "$CERTS/iso20/private_keys/oemLeafPassword.txt")"
V2G20="$CERTS/iso20/certs/v2gRootCACert.pem"
V2G2="$CERTS/iso2/certs/v2gRootCACert.pem"

pass=0
fail=0

probe() {
    local name="$1"; shift
    local want="$1"; shift # ok | fail
    python3 sdp_probe.py --charger "$CHARGER" > /dev/null || { echo "SDP FAILED before $name"; fail=$((fail+1)); return; }
    sleep 0.3
    out=$(echo Q | timeout 60 openssl s_client -connect "[$SECC]:$SECC_PORT" "$@" 2>&1)
    rc=$?
    proto=$(echo "$out" | grep -oE "Protocol *: *TLSv[0-9.]+" | head -1)
    cipher=$(echo "$out" | grep -oE "Cipher is [A-Z0-9_-]+" | head -1)
    verify_ok=$(echo "$out" | grep -c "Verify return code: 0 (ok)")
    hs_done=$(echo "$out" | grep -cE "Cipher is (TLS_|ECDHE-)")
    alert=$(echo "$out" | grep -oE "alert [a-z ]+" | head -1)
    if [ "$want" = ok ]; then
        if [ "$hs_done" -ge 1 ] && [ $rc -eq 0 ] && [ "$verify_ok" -ge 1 ]; then
            echo "ok   $name: ${proto:-TLSv1.3} $cipher"
            pass=$((pass+1))
        else
            echo "FAIL $name: expected verified handshake, rc=$rc verify_ok=$verify_ok alert='$alert'"
            echo "$out" | tail -5 | sed 's/^/     /'
            fail=$((fail+1))
        fi
    else
        if [ "$hs_done" -ge 1 ] && [ $rc -eq 0 ]; then
            echo "FAIL $name: expected failure but handshake succeeded: $proto $cipher"
            fail=$((fail+1))
        else
            echo "ok   $name: refused (${alert:-no alert seen by s_client, see local_test_alerts.py})"
            pass=$((pass+1))
        fi
    fi
    sleep 0.5
}

echo "--- TLS 1.3 groups (HUB20-533-003/005) ---"
probe "tls13 default groups (x25519 first, expect HRR)" ok \
    -tls1_3 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 P-521 key share" ok \
    -tls1_3 -groups P-521 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 X448 key share" ok \
    -tls1_3 -groups X448 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 P-256 then P-521 (expect HRR to P-521)" ok \
    -tls1_3 -groups P-256:P-521 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 P-256 only (must refuse)" fail \
    -tls1_3 -groups P-256 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 X25519 only (must refuse)" fail \
    -tls1_3 -groups X25519 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"

echo "--- TLS 1.3 sig algs (HUB20-533-001/002) ---"
probe "tls13 sigalg ecdsa_secp521r1_sha512" ok \
    -tls1_3 -sigalgs ecdsa_secp521r1_sha512 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 sigalg ecdsa_secp256r1_sha256 only (secc cert is secp521r1, must refuse)" fail \
    -tls1_3 -sigalgs ecdsa_secp256r1_sha256 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"
probe "tls13 sigalg rsa_pss_rsae_sha256 only (must refuse)" fail \
    -tls1_3 -sigalgs rsa_pss_rsae_sha256 -CAfile "$V2G20" -cert "$OEM20" -key "$OEMKEY20" -pass "$OEMPASS"

echo "--- TLS 1.2 (V2G2-006, HUB20-533) ---"
probe "tls12 P-256 ECDHE-ECDSA-AES128-SHA256" ok \
    -tls1_2 -cipher ECDHE-ECDSA-AES128-SHA256 -curves P-256 -sigalgs ECDSA+SHA256 -CAfile "$V2G2"
probe "tls12 default groups" ok \
    -tls1_2 -cipher ECDHE-ECDSA-AES128-SHA256 -CAfile "$V2G2"
probe "tls12 X25519 only (must refuse)" fail \
    -tls1_2 -cipher ECDHE-ECDSA-AES128-SHA256 -curves X25519 -CAfile "$V2G2"
probe "tls12 sigalg ECDSA+SHA1 only (must refuse)" fail \
    -tls1_2 -cipher 'ECDHE-ECDSA-AES128-SHA256:@SECLEVEL=0' -curves P-256 -sigalgs ECDSA+SHA1 -CAfile "$V2G2"
probe "tls12 disjoint cipher (must refuse)" fail \
    -tls1_2 -cipher ECDHE-RSA-AES256-GCM-SHA384 -CAfile "$V2G2"

echo "--- result: $pass passed, $fail failed ---"
exit $((fail > 0))
