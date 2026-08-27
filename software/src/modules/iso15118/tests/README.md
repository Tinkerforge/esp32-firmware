# ISO 15118 tests

These suites run ISO 15118 tests against a WARP4 through the firmware debug
mode. Debug mode uses Ethernet for SDP and V2G communication, so these tests do
not cover PLC, SLAC or CP behavior.

## Prerequisites

* Java for the EcoG EXI codec, OpenSSL, GCC
* IPv6 link-local multicast connectivity to the WARP4
* A generated development PKI in `../tools/certs/output`
