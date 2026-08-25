import json
import socket
import struct
import subprocess
import urllib.request

SDP_SECURITY_TLS = 0x00
SDP_SECURITY_NO_TLS = 0x10


def default_iface(charger_ip):
    """Network interface with the route to the charger."""
    out = subprocess.check_output(["ip", "route", "get", charger_ip], text=True)
    return out.split(" dev ")[1].split()[0]


def local_ip_towards(charger_ip):
    """Local source address used to reach the charger."""
    out = subprocess.check_output(["ip", "route", "get", charger_ip], text=True)
    return out.split(" src ")[1].split()[0]


def api_get(charger_ip, path):
    with urllib.request.urlopen(f"http://{charger_ip}/{path}", timeout=5) as r:
        return json.loads(r.read())


def api_put(charger_ip, path, payload):
    req = urllib.request.Request(
        f"http://{charger_ip}/{path}",
        data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json"},
        method="PUT")
    with urllib.request.urlopen(req, timeout=5) as r:
        return r.read().decode()


def enable_debug_mode(charger_ip):
    api_put(charger_ip, "iso15118/debug_update", {"enable": True, "current": 6000, "phases": 3})


def disable_debug_mode(charger_ip):
    api_put(charger_ip, "iso15118/debug_update", {"enable": False, "current": 6000, "phases": 3})


def require_iso_tls_config(charger_ip):
    cfg = api_get(charger_ip, "iso15118/config")
    if not cfg["charge_via_iso15118"]:
        raise SystemExit("charge_via_iso15118 is disabled on the charger, TLS is never offered. Enable it first.")


def sdp_request(iface, security=SDP_SECURITY_TLS, timeout=3):
    """Sends an SDP DiscoveryRequest via IPv6 all-nodes multicast.

    Returns the response as a dict or None on timeout (which is the expected
    outcome for a no-TLS request while EnforceTlsEnabled is set).
    """
    ifindex = socket.if_nametoindex(iface)
    payload = struct.pack("!BB", security, 0x00)  # security, transport TCP
    header = struct.pack("!BBHI", 0x01, 0xFE, 0x9000, len(payload))

    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_IF, ifindex)
    sock.settimeout(timeout)
    sock.sendto(header + payload, ("ff02::1", 15118, 0, ifindex))
    try:
        data, addr = sock.recvfrom(64)
    except socket.timeout:
        return None
    finally:
        sock.close()

    if len(data) != 28:
        raise ValueError(f"unexpected SDP response length {len(data)}: {data.hex()}")

    # esp_ip6_addr words are already in network byte order
    ip_words = struct.unpack("<4I", data[8:24])
    ip_bytes = b"".join(struct.pack("<I", w) for w in ip_words)
    port, security_res, transport = struct.unpack("!HBB", data[24:28])

    return {
        "from": addr[0],
        "secc_ll": socket.inet_ntop(socket.AF_INET6, ip_bytes),
        "port": port,
        "security": security_res,
        "transport": transport,
    }


def connect_secc(charger_ip, iface=None, timeout=20):
    """SDP TLS request plus TCP connect to the announced SECC endpoint.

    Returns the connected TCP socket. TLS is up to the caller.
    """
    iface = iface or default_iface(charger_ip)
    res = sdp_request(iface)
    if res is None:
        raise SystemExit("SDP request timed out, is ISO debug mode enabled?")
    ifindex = socket.if_nametoindex(iface)
    sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.connect((res["secc_ll"], res["port"], 0, ifindex))
    return sock
