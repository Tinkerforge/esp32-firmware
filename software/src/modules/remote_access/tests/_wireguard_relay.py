"""Multiplex test WireGuard interfaces behind the relay's single UDP port.

Run inside the test container. The device's cleartext port-discovery packet
selects the user tunnel; endpoints without a discovery packet use management.
Each WireGuard interface has its own key and loopback listening port.
"""

import selectors
import socket
import struct


MANAGEMENT_PORT = 51821
USER_PORT_BASE = 52000


def main() -> None:
    selector = selectors.DefaultSelector()
    transport = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    transport.bind(("0.0.0.0", 51820))
    selector.register(transport, selectors.EVENT_READ)
    endpoints: dict[tuple[str, int], tuple[socket.socket, int]] = {}
    print("ready", flush=True)

    while True:
        for event, _ in selector.select():
            if event.fileobj is not transport:
                # Discovery can replace an endpoint whose old socket was also
                # readable in this batch of events.
                if endpoints.get(event.data, (None, 0))[0] is not event.fileobj:
                    continue
                packet = event.fileobj.recv(65535)
                transport.sendto(packet, event.data)
                continue

            packet, address = transport.recvfrom(65535)
            port = MANAGEMENT_PORT
            if len(packet) == 24:
                # port_discovery_packet: charger ID, connection number, UUID.
                _, connection_no, _ = struct.unpack("<Ii16s", packet)
                if not 0 <= connection_no < 256:
                    continue
                port = USER_PORT_BASE + connection_no
                previous = endpoints.pop(address, None)
                if previous is not None:
                    selector.unregister(previous[0])
                    previous[0].close()

            if address not in endpoints:
                peer = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                # Leave the socket unconnected: a user interface may not exist
                # yet, and an ICMP port-unreachable must not stop the relay.
                endpoints[address] = peer, port
                selector.register(peer, selectors.EVENT_READ, address)
                peer.bind(("127.0.0.1", 0))
            else:
                peer, port = endpoints[address]

            if len(packet) != 24:
                peer.sendto(packet, ("127.0.0.1", port))


if __name__ == "__main__":
    main()
