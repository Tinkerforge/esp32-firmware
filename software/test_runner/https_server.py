from collections.abc import Callable
from http.server import HTTPServer, BaseHTTPRequestHandler
import os
import shutil
import ssl
import tempfile
import threading


# Response handler: (method, path, request_body) -> (status_code, response_body)
type ResponseFn = Callable[[str, str, bytes], tuple[int, str]]

class _TestHTTPServer(HTTPServer):
    response_body: str = '{"error":"no data configured"}'
    response_status: int = 404
    response_fn: ResponseFn | None = None

class _TestHandler(BaseHTTPRequestHandler):
    server: _TestHTTPServer  # type: ignore[assignment]

    def _handle_request(self, method: str):
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length) if content_length > 0 else b''

        if self.server.response_fn is not None:
            status, response_body = self.server.response_fn(method, self.path, body)
        else:
            status = self.server.response_status
            response_body = self.server.response_body

        encoded = response_body.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded)))
        self.end_headers()
        self.wfile.write(encoded)

    def do_GET(self):
        self._handle_request("GET")

    def do_POST(self):
        self._handle_request("POST")

    def do_PUT(self):
        self._handle_request("PUT")

    def log_message(self, format, *args):
        pass

class TestHTTPSServer:
    """HTTPS test server with configurable responses.

    Responses can be set in two ways:
    - Static: server.set_response('{"key": "value"}', status=200)
    - Dynamic: server.set_response_fn(lambda method, path, body: (200, '{"ok":true}'))
    """

    def __init__(self, cert_pem: str, key_pem: str, local_ip: str, bind_ip: str = "0.0.0.0"):
        self._cert_pem = cert_pem
        self._key_pem = key_pem
        self._local_ip = local_ip
        self._bind_ip = bind_ip
        self._httpd: _TestHTTPServer | None = None
        self._thread: threading.Thread | None = None
        self._tmpdir: str | None = None

    @property
    def port(self) -> int:
        assert self._httpd is not None, "Server not started"
        return self._httpd.server_address[1]

    @property
    def url(self) -> str:
        return f"https://{self._local_ip}:{self.port}/"

    def start(self) -> None:
        self._tmpdir = tempfile.mkdtemp()
        cert_path = os.path.join(self._tmpdir, "cert.pem")
        key_path = os.path.join(self._tmpdir, "key.pem")
        with open(cert_path, "w") as f:
            f.write(self._cert_pem)
        with open(key_path, "w") as f:
            f.write(self._key_pem)

        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ctx.load_cert_chain(cert_path, key_path)

        self._httpd = _TestHTTPServer((self._bind_ip, 0), _TestHandler)
        self._httpd.socket = ctx.wrap_socket(self._httpd.socket, server_side=True)

        self._thread = threading.Thread(target=self._httpd.serve_forever, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        if self._httpd:
            self._httpd.shutdown()
            self._httpd = None
        if self._thread:
            self._thread.join(timeout=5)
            self._thread = None
        if self._tmpdir:
            shutil.rmtree(self._tmpdir, ignore_errors=True)
            self._tmpdir = None

    def set_response(self, body: str, status: int = 200) -> None:
        """Set a static response returned for every request."""
        assert self._httpd is not None, "Server not started"
        self._httpd.response_body = body
        self._httpd.response_status = status
        self._httpd.response_fn = None

    def set_response_fn(self, fn: ResponseFn) -> None:
        """Set a dynamic response handler: (method, path, request_body) -> (status, body)."""
        assert self._httpd is not None, "Server not started"
        self._httpd.response_fn = fn
