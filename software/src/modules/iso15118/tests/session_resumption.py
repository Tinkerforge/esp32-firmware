#!/usr/bin/env -S uv run --locked --group iso15118-tests --script

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


if __name__ == "__main__":
    run_testsuite(locals())
