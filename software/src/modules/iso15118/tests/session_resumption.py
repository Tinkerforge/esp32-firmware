#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

import os
import select
import ssl
import time

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, "software")
from software.test_runner.test_context import run_testsuite, TestContext

from _common import EVTestClient, ISO2, ISO20_AC, IsoTestEnvironment, managed_socket


environment = None
client = None

TLS_EARLY_DATA = 42


def parse_extensions(data, offset):
    if offset + 2 > len(data):
        raise ValueError("truncated extension vector")
    length = int.from_bytes(data[offset:offset + 2], "big")
    if length == 0xFFFF:
        raise ValueError("extension vector exceeds the TLS 1.3 maximum")
    offset += 2
    end = offset + length
    if end != len(data):
        raise ValueError("invalid extension vector length")
    extensions = {}
    while offset < end:
        if offset + 4 > end:
            raise ValueError("truncated extension")
        extension_type = int.from_bytes(data[offset:offset + 2], "big")
        extension_length = int.from_bytes(data[offset + 2:offset + 4], "big")
        offset += 4
        if extension_type in extensions or offset + extension_length > end:
            raise ValueError("invalid extension")
        extensions[extension_type] = data[offset:offset + extension_length]
        offset += extension_length
    return extensions


def parse_new_session_ticket(message):
    if len(message) < 4 or message[0] != 4:
        raise ValueError("invalid NewSessionTicket handshake message")
    if int.from_bytes(message[1:4], "big") != len(message) - 4:
        raise ValueError("invalid NewSessionTicket handshake length")
    body = message[4:]
    if len(body) < 11:
        raise ValueError("truncated NewSessionTicket")
    lifetime = int.from_bytes(body[0:4], "big")
    age_add = int.from_bytes(body[4:8], "big")
    nonce_length = body[8]
    offset = 9
    if offset + nonce_length + 2 > len(body):
        raise ValueError("truncated ticket nonce")
    nonce = body[offset:offset + nonce_length]
    offset += nonce_length
    ticket_length = int.from_bytes(body[offset:offset + 2], "big")
    offset += 2
    if ticket_length == 0 or offset + ticket_length + 2 > len(body):
        raise ValueError("invalid opaque ticket")
    ticket = body[offset:offset + ticket_length]
    offset += ticket_length
    extensions = parse_extensions(body, offset)
    return {
        "lifetime": lifetime,
        "age_add": age_add,
        "nonce": nonce,
        "ticket": ticket,
        "extensions": extensions,
    }


def collect_new_session_tickets(context):
    tickets = []

    def callback(connection, direction, version, content_type, message_type, data):
        del connection, version
        if direction == "read" and int(content_type) == 22 and int(message_type) == 4:
            tickets.append(bytes(data))

    if not hasattr(context, "_msg_callback"):
        raise RuntimeError("Python ssl message callback unavailable")
    context._msg_callback = callback
    return tickets


def assert_ticket_policy(tc, ticket):
    tc.assert_eq(32, len(ticket["nonce"]))
    tc.assert_false(TLS_EARLY_DATA in ticket["extensions"])
    tc.assert_ge(20, ticket["lifetime"])
    tc.assert_le(86400, ticket["lifetime"])
    tc.assert_eq(3600, ticket["lifetime"])


def suite_setup(tc: TestContext):
    global environment, client
    environment = IsoTestEnvironment(tc)
    environment.start()
    client = EVTestClient(environment.host, environment.iface, environment.secc_ll)


def setup(tc: TestContext):
    assert environment is not None
    environment.reset_session()


def suite_teardown(tc: TestContext):
    if environment is not None:
        environment.stop()


def test_tls13_ticket_resumption(tc: TestContext):
    assert client is not None
    context = client.tls13_context()
    ticket_messages = collect_new_session_tickets(context)

    before = len(ticket_messages)
    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_false(tls.session_reused)
        result = client.sap(tls, [ISO20_AC])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(2, result["SchemaID"])
        session = tls.session
    tc.assert_(len(ticket_messages) > before)
    first_tickets = [parse_new_session_ticket(message) for message in ticket_messages[before:]]
    for ticket in first_tickets:
        assert_ticket_policy(tc, ticket)
    first_ticket = first_tickets[-1]

    time.sleep(1)
    tc.assert_(session is not None and session.has_ticket)
    tc.assert_ge(20, session.ticket_lifetime_hint)
    tc.assert_le(86400, session.ticket_lifetime_hint)

    before = len(ticket_messages)
    with managed_socket(client.connect_tls(context, session=session)) as tls:
        tc.assert_(tls.session_reused)
        tc.assert_eq("TLSv1.3", tls.version())
        result = client.sap(tls, [ISO20_AC, ISO2])
        tc.assert_eq("Failed_NoNegotiation", result["ResponseCode"])
    tc.assert_(len(ticket_messages) > before)
    resumed_tickets = [parse_new_session_ticket(message) for message in ticket_messages[before:]]
    for ticket in resumed_tickets:
        assert_ticket_policy(tc, ticket)
    resumed_ticket = resumed_tickets[-1]

    time.sleep(1)
    second_context = client.tls13_context()
    second_ticket_messages = collect_new_session_tickets(second_context)
    with managed_socket(client.connect_tls(second_context)) as tls:
        tc.assert_false(tls.session_reused)
        result = client.sap(tls, [ISO20_AC])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(2, result["SchemaID"])
    tc.assert_(len(second_ticket_messages) > 0)
    second_tickets = [parse_new_session_ticket(message) for message in second_ticket_messages]
    for ticket in second_tickets:
        assert_ticket_policy(tc, ticket)
    second_ticket = second_tickets[-1]
    tickets = first_tickets + resumed_tickets + second_tickets
    tc.assert_eq(len(tickets), len({ticket["nonce"] for ticket in tickets}))
    tc.assert_eq(len(tickets), len({ticket["ticket"] for ticket in tickets}))


def test_tls12_not_resumed(tc: TestContext):
    assert client is not None
    context = client.tls12_context()
    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_eq("TLSv1.2", tls.version())
        result = client.sap(tls, [ISO2])
        tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
        tc.assert_eq(1, result["SchemaID"])
        session = tls.session

    time.sleep(1)
    tc.assert_(session is None or not session.has_ticket)
    if session is not None:
        with managed_socket(client.connect_tls(context, session=session)) as tls:
            tc.assert_false(tls.session_reused)
            result = client.sap(tls, [ISO2])
            tc.assert_eq("OK_SuccessfulNegotiation", result["ResponseCode"])
            tc.assert_eq(1, result["SchemaID"])


def test_tls13_idle_ticket_renewal(tc: TestContext):
    if os.environ.get("ISO15118_TICKET_SOAK") != "1":
        tc.skip("Set ISO15118_TICKET_SOAK=1 for the one-hour on-device renewal test")
    assert client is not None
    tc.set_test_timeout(3700)
    context = client.tls13_context()
    ticket_messages = collect_new_session_tickets(context)
    observed_at = []
    original_callback = context._msg_callback

    def callback(*args):
        before = len(ticket_messages)
        original_callback(*args)
        if len(ticket_messages) > before:
            observed_at.append(time.monotonic())

    context._msg_callback = callback
    with managed_socket(client.connect_tls(context)) as tls:
        tc.assert_false(tls.session_reused)
        tc.assert_eq("OK_SuccessfulNegotiation", client.sap(tls, [ISO20_AC])["ResponseCode"])
        tc.assert_eq(1, len(ticket_messages))
        first = parse_new_session_ticket(ticket_messages[0])
        assert_ticket_policy(tc, first)
        tls.setblocking(False)
        deadline = observed_at[0] + 3605
        next_progress = observed_at[0] + 300
        while len(ticket_messages) < 2:
            now = time.monotonic()
            if now >= deadline:
                tc.fail("No replacement ticket received before expiry")
            if now >= next_progress:
                print(f"Waiting for idle renewal: {now - observed_at[0]:.0f}s elapsed", flush=True)
                next_progress += 300
            if select.select([tls], [], [], min(5, deadline - now))[0]:
                try:
                    data = tls.recv(1)
                except ssl.SSLWantReadError:
                    continue
                tc.fail(f"Unexpected application data or closed connection while idle: {data!r}")
        tc.assert_eq(2, len(ticket_messages))
        replacement = parse_new_session_ticket(ticket_messages[1])
        assert_ticket_policy(tc, replacement)
        elapsed = observed_at[1] - observed_at[0]
        # One second of margin is built into the 3579-second schedule.
        tc.assert_(3578 <= elapsed <= first["lifetime"] - 20)
        tc.assert_(replacement["nonce"] != first["nonce"])
        tc.assert_(replacement["ticket"] != first["ticket"])
        tc.assert_eq(first["ticket"][:8], replacement["ticket"][:8])
        session = tls.session
        tc.assert_true(session.has_ticket)
        print(f"PASS idle replacement at {elapsed:.3f}s; {first['lifetime'] - elapsed:.3f}s before expiry", flush=True)

    time.sleep(1)
    before = len(ticket_messages)
    with managed_socket(client.connect_tls(context, session=session)) as tls:
        tc.assert_true(tls.session_reused)
        tc.assert_eq("Failed_NoNegotiation", client.sap(tls, [ISO20_AC, ISO2])["ResponseCode"])
    tc.assert_eq(before + 1, len(ticket_messages))
    resumed = parse_new_session_ticket(ticket_messages[-1])
    assert_ticket_policy(tc, resumed)
    tc.assert_eq(first["ticket"][:8], resumed["ticket"][:8])
    tc.assert_eq(3, len({parse_new_session_ticket(message)["nonce"] for message in ticket_messages}))
    tc.assert_eq(3, len({parse_new_session_ticket(message)["ticket"] for message in ticket_messages}))


if __name__ == "__main__":
    run_testsuite(locals())
