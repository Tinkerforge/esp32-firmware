"""Helpers for test suites that run external services (e.g. Home Assistant, evcc) in Docker containers.

Containers run in the host network: The DUT can reach services on the test machine
via TestContext.get_local_ip() and the test can reach them on 127.0.0.1.
"""

import os
import socket
import subprocess
import tempfile
import time


class ContainerError(Exception):
    pass


def docker_available() -> bool:
    try:
        return subprocess.run(["docker", "info"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=30).returncode == 0
    except (OSError, subprocess.TimeoutExpired):
        return False


def pull_image_if_missing(image: str) -> None:
    # docker create would pull silently, making startup timeouts include the download.
    if subprocess.run(["docker", "image", "inspect", image], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode != 0:
        subprocess.run(["docker", "pull", image], check=True, stdout=subprocess.DEVNULL)


class Container:
    """A container in the host network. Files (container path -> content) are copied into it before it starts."""

    def __init__(self, name: str, image: str, files: dict[str, str] | None = None, args: list[str] | None = None, command: list[str] | None = None):
        self.name = name
        self.image = image
        self.files = files or {}
        self.args = args or []
        self.command = command or []

    def start(self) -> None:
        pull_image_if_missing(self.image)
        self.remove()
        subprocess.run(["docker", "create", "--name", self.name, "--network", "host", *self.args, self.image, *self.command], check=True, stdout=subprocess.DEVNULL)

        with tempfile.TemporaryDirectory() as d:
            for container_path, content in self.files.items():
                path = os.path.join(d, os.path.basename(container_path))
                with open(path, "w") as f:
                    f.write(content)
                subprocess.run(["docker", "cp", path, f"{self.name}:{container_path}"], check=True, stdout=subprocess.DEVNULL)

        subprocess.run(["docker", "start", self.name], check=True, stdout=subprocess.DEVNULL)

    def restart(self) -> None:
        subprocess.run(["docker", "restart", "-t", "0", self.name], check=True, stdout=subprocess.DEVNULL)

    def remove(self) -> None:
        subprocess.run(["docker", "rm", "-f", self.name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    def running(self) -> bool:
        state = subprocess.run(["docker", "inspect", "-f", "{{.State.Running}}", self.name], capture_output=True, text=True)
        return state.stdout.strip() == "true"

    def logs(self) -> str:
        result = subprocess.run(["docker", "logs", self.name], capture_output=True, text=True, errors="replace")
        return result.stdout + result.stderr

    def assert_running(self) -> None:
        if not self.running():
            raise ContainerError(f"Container {self.name} stopped:\n{self.logs()[-3000:]}")

    def wait_for_port(self, port: int, timeout: float) -> None:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            self.assert_running()
            try:
                with socket.create_connection(("127.0.0.1", port), timeout=1):
                    return
            except OSError:
                time.sleep(0.2)
        raise ContainerError(f"Container {self.name} did not open port {port} within {timeout}s")
