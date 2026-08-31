#!/usr/bin/env python3

import argparse
import socket


def receive_record(connection: socket.socket) -> bytearray:
    record = bytearray()
    while len(record) < 5:
        data = connection.recv(5 - len(record))
        if not data:
            raise RuntimeError("connection closed before TLS record header")
        record.extend(data)
    length = int.from_bytes(record[3:5], "big")
    while len(record) < 5 + length:
        data = connection.recv(5 + length - len(record))
        if not data:
            raise RuntimeError("connection closed within TLS record")
        record.extend(data)
    return record


def extension_bounds(record: bytearray) -> tuple[int, int]:
    if record[0] != 22 or record[5] != 1:
        raise RuntimeError("first TLS record is not a ClientHello")
    offset = 9 + 2 + 32
    offset += 1 + record[offset]
    suites_length = int.from_bytes(record[offset : offset + 2], "big")
    offset += 2 + suites_length
    offset += 1 + record[offset]
    extensions_length = int.from_bytes(record[offset : offset + 2], "big")
    return offset, offset + 2 + extensions_length


def find_extension(record: bytearray, extension_type: int) -> tuple[int, int]:
    length_offset, end = extension_bounds(record)
    offset = length_offset + 2
    while offset < end:
        current_type = int.from_bytes(record[offset : offset + 2], "big")
        current_length = int.from_bytes(record[offset + 2 : offset + 4], "big")
        if current_type == extension_type:
            return offset, current_length
        offset += 4 + current_length
    raise RuntimeError(f"ClientHello has no extension {extension_type}")


def mutate(record: bytearray, mode: str) -> None:
    mfl_offset, mfl_length = find_extension(record, 1)
    if mfl_length != 1:
        raise RuntimeError("unexpected maximum_fragment_length encoding")

    if mode == "invalid-mfl":
        record[mfl_offset + 4] = 5
        return

    length_offset, end = extension_bounds(record)
    record[end:end] = b"\x00\x1c\x00\x02\x02\x00"
    extensions_length = int.from_bytes(record[length_offset : length_offset + 2], "big") + 6
    record[length_offset : length_offset + 2] = extensions_length.to_bytes(2, "big")
    handshake_length = int.from_bytes(record[6:9], "big") + 6
    record[6:9] = handshake_length.to_bytes(3, "big")
    record_length = int.from_bytes(record[3:5], "big") + 6
    record[3:5] = record_length.to_bytes(2, "big")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=("invalid-mfl", "conflicting-limits"))
    parser.add_argument("listen_port", type=int)
    parser.add_argument("server_port", type=int)
    args = parser.parse_args()

    with socket.create_server(("127.0.0.1", args.listen_port)) as listener:
        client, _ = listener.accept()
        with client, socket.create_connection(("127.0.0.1", args.server_port)) as server:
            record = receive_record(client)
            mutate(record, args.mode)
            server.sendall(record)
            response = receive_record(server)
            client.sendall(response)


if __name__ == "__main__":
    main()
