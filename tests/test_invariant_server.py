import pytest
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from software.web.server import app


@pytest.fixture
def client():
    app.config['TESTING'] = True
    with app.test_client() as client:
        yield client


@pytest.mark.parametrize("path,method,headers", [
    # No authentication at all
    ("/", "GET", {}),
    # Missing token header
    ("/api/config", "GET", {"Content-Type": "application/json"}),
    # Malformed/invalid token
    ("/api/device/control", "POST", {"Authorization": "Bearer invalid_token_xyz"}),
    # Expired token format
    ("/settings", "PUT", {"Authorization": "Bearer expired.token.here"}),
])
def test_protected_endpoints_reject_unauthenticated_requests(client, path, method, headers):
    """Invariant: Protected endpoints must reject unauthenticated requests with 401 or 403"""
    if method == "GET":
        response = client.get(path, headers=headers)
    elif method == "POST":
        response = client.post(path, headers=headers, data=b'{}')
    elif method == "PUT":
        response = client.put(path, headers=headers, data=b'{}')
    
    # Security invariant: unauthenticated requests should be rejected
    assert response.status_code in (401, 403), (
        f"Endpoint {path} accepted unauthenticated {method} request with status {response.status_code}. "
        f"Expected 401 or 403 for requests without valid authentication."
    )