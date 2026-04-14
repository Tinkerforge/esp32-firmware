import asyncio
import threading
from dataclasses import dataclass
import websockets
from typing import Callable, cast

from ocpp.v16 import ChargePoint

@dataclass
class TestOcppCentral: # uses tls when certfile/keyfile are provided
    handler_factory: Callable[[str,  websockets.ServerConnection], ChargePoint]
    listen_address: str = '0.0.0.0'
    port: int = 9000
    certfile: str | None = None
    keyfile: str | None = None
    _loop: asyncio.AbstractEventLoop | None = None
    _thread: threading.Thread | None = None


    def start(self) -> None:
        self._loop = asyncio.new_event_loop()
        self._thread = threading.Thread(target=self._run_loop, daemon=True)
        self._thread.start()

        asyncio.run_coroutine_threadsafe(self._start_central(), self._loop)

    async def on_connect(self, connection: websockets.ServerConnection):
        """
        For every new connection, create a new ChargePoint instance,
        and start listening for messages.
        """
        assert connection.request is not None
        charge_point_id = connection.request.path.split("/")[-1]
        charge_point = self.handler_factory(charge_point_id, connection)

        await charge_point.start()

    async def _start_central(self) -> None:
        server = await websockets.serve(
            self.on_connect,
            self.listen_address,
            self.port,
            subprotocols=[cast(websockets.Subprotocol, 'ocpp1.6')],
        )
        print(f"serving on {self.listen_address}:{self.port}")
        await server.wait_closed()

    def stop(self) -> None:
        if self._loop:
            self._loop.call_soon_threadsafe(self._loop.stop)
        if self._thread:
            self._thread.join(timeout=5)

    def _run_loop(self) -> None:
        assert self._loop is not None
        asyncio.set_event_loop(self._loop)
        self._loop.run_forever()
