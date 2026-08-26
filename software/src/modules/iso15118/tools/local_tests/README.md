# ISO 15118 local tests

Manual test scripts for the ISO 15118 TLS server and its OCPP wiring, written
against a bare ESP32 Ethernet Brick with WARP4 firmware in iso debug mode.

## Prerequisites

* ESP flashed with a warp4 build, reachable via HTTP (SDP runs over IPv6 link-local multicast)
* `charge_via_iso15118` enabled in the charger iso15118 config
  `curl -X PUT -d '{"enable":false,"current":6000,"phases":3}' http://<ip>/iso15118/debug_update`
* A dev PKI in `../certs/output` that matches the firmware embedded dev_certs.cpp.
* `openssl` CLI for the TLS probes
* The evsim venv for the EXI codec based tests (see ../evsim/README.md)

## Tests

These are not automated tests, they are meant as helpers for probing HUB20-533-001/002/003/004/005 by hand

`local_test_client_hello_parser.sh`: Extracts classify_client_hello from isotls.cpp, compiles it with ASan/UBSan and checks it against ClientHellos captured live from the local OpenSSL plus synthetic edge cases, including a full prefix sweep
`local_test_tls_probes.sh <charger-ip>`: openssl s_client probes: TLS 1.3 group policing (HRR on X25519 first, secp256r1/X25519 only refused per HUB20-533-003/005), sig alg restriction (HUB20-533-001/002), TLS 1.2 curve/cipher/sig alg refusals
`local_test_alerts.py --charger <ip>`: Captures ClientHellos locally and replays them raw to verify the refusals carry a fatal handshake_failure alert on the wire (HUB20-533-002/004)
`local_test_sap_tls12.py --charger <ip>`: supportedAppProtocol over a real TLS 1.2 handshake via the EXI codec: only -20 offered answers Failed_NoNegotiation (V2G20-2356), -2 wins next to -20, -20 with VersionNumberMajor 2 is ignored. Also runs a -2 SessionSetup and prints the EVSEID
`local_test_ocpp_ctrlr.py --charger <ip>`: Runs an embedded OCPP 2.1 CSMS, points the charger at it and checks the ISO15118Ctrlr wiring: ProtocolSupported instances, defaults, ISO15118EvseId consumed in SessionSetupRes, EnforceTlsEnabled SDP refusal round trip (HUB20-52-001)
`local_test_ocsp_stapling_patch.sh`: Host only. Applies the mbedTLS OCSP stapling patch (patches/lib-builder 0006) to a pristine mbedTLS 3.6.6 checkout ($MBEDTLS_SRC or cloned from GitHub), builds it and verifies with openssl s_client -status that the TLS 1.3 Certificate message carries the stapled response exactly when the client requests it (V2G20-2388)
`local_test_ocsp_gating.py --charger <ip>`: Embedded OCPP 2.1 CSMS that provisions the certificate store step by step (roots, -2 and -20 chains signed by the dev CPO sub CA 2, Good OCSP response via GetCertificateStatus) and checks the TLS 1.3 availability gate after every step incl. the PrivateEnviromentEnabled waiver and the staple DER load trace (HUB20-532-002). Wipes the OCPP storage

`local_test_vehicle_chain.py --charger <ip>`: Embedded OCPP 2.1 CSMS plus TLS 1.3 mutual auth sessions that present an OEM (vehicle) chain and drive the -20 Authorization exchange over the EXI codec. Checks the M07 vehicle chain OCSP gate: full chain requested leaf first, EVSEProcessing Ongoing while pending, Finished/OK after all Good, FAILED plus TLS close on Revoked and on a missing status entry (HUB20-432-004/008/009/010), and that a forged OEM chain with matching subject names but different keys is rejected by the handshake. Wipes the OCPP storage

`local_test_pnc.py --charger <ip>`: Embedded OCPP 2.1 CSMS driving Plug and Charge (M01/M02) for both -20 (TLS 1.3 mutual auth) and -2 (TLS 1.2). Checks that AuthorizationSetup/ServiceDiscovery offer PnC and the certificate service, that a signed contract chain authorizes while a wrong challenge or a forged contract chain is rejected, and that CertificateInstallationReq is forwarded to the CSMS via Get15118EVCertificate and the response relayed. Signs the XML signatures over the libcbv2g SignedInfo encoding via the pnc_exi_helper.c host helper (compiled against the libcbv2g sources), since the EcoG codec encodes the same SignedInfo into different bytes the device would not verify. Wipes the OCPP storage

`local_test_session_resumption.py --charger <ip>`: TLS 1.3 session resumption via psk_dhe_ke session tickets [V2G20-1675 ff]. A full TLS 1.3 mutual auth handshake yields a NewSessionTicket with a valid lifetime, the ticket resumes a TLS 1.3 session, SAP over the resumed session answers Failed_NoNegotiation (only full-handshake TLS carries V2G [V2G20-2677]), a fresh full handshake negotiates -20 again, and TLS 1.2 gets no usable ticket and never resumes. Dev PKI only, no OCPP needed. Needs firmware built against libs with session tickets enabled (lib-builder defconfig)
