#!/usr/bin/env python3
"""
WARP4 Charger EV Simulator Launcher

Wrapper around the EcoG-io/iso15118 EVCC simulator for testing
with WARP4 Charger ISO 15118 debug mode.

Usage:
    # Show help
    ./run_evsim.py

    # Specify charger IP for automatic debug mode control
    ./run_evsim.py -c 192.168.0.33

    # Interactive mode
    ./run_evsim.py -i

    # Specify protocol and mode
    ./run_evsim.py -c 192.168.0.33 -p din -m dc

    # Multiple charging modes (AC + DC)
    ./run_evsim.py -c 192.168.0.33 -m ac,dc

    # Multiple energy transfer modes
    ./run_evsim.py -c 192.168.0.33 -e ac_single,ac_three

    # Enable TLS with generated certificates
    ./run_evsim.py -c 192.168.0.33 -p iso2 --tls

    # List available network interfaces
    ./run_evsim.py --list-interfaces
"""

import argparse
import asyncio
import os
import signal
import socket
import sys
from pathlib import Path
from typing import List, Optional, Tuple

# Ensure we can import from the iso15118 package
SCRIPT_DIR = Path(__file__).parent.resolve()
ISO15118_DIR = SCRIPT_DIR / "iso15118"

if not ISO15118_DIR.exists():
    print(f"Error: iso15118 directory not found at {ISO15118_DIR}")
    print("Please clone the repository first:")
    print(f"  cd {SCRIPT_DIR}")
    print("  git clone --depth 1 https://github.com/EcoG-io/iso15118.git")
    sys.exit(1)

sys.path.insert(0, str(ISO15118_DIR))

# Now we can import iso15118
try:
    from iso15118.evcc import EVCCHandler
    from iso15118.evcc.controller.simulator import SimEVController
    from iso15118.evcc.evcc_config import EVCCConfig
    from iso15118.shared.exificient_exi_codec import ExificientEXICodec
    from iso15118.shared.settings import load_shared_settings
except ImportError as e:
    print(f"Error importing iso15118: {e}")
    print("\nMake sure you have installed the requirements:")
    print("  pip install -r requirements.txt")
    print("\nOr activate the virtual environment:")
    print("  source ../../../../../.venv-evsim/bin/activate")
    sys.exit(1)

# Optional: requests for HTTP API calls to charger
try:
    import requests
    HAS_REQUESTS = True
except ImportError:
    HAS_REQUESTS = False


# Protocol mappings - individual protocols
PROTOCOLS = {
    "din": "DIN_SPEC_70121",
    "iso2": "ISO_15118_2",
    "iso20ac": "ISO_15118_20_AC",
    "iso20dc": "ISO_15118_20_DC",
}

# Shorthand combinations
PROTOCOL_SHORTCUTS = {
    "all": ["din", "iso2", "iso20ac", "iso20dc"],
    "iso20": ["iso20ac", "iso20dc"],
}

# Energy transfer mode mappings
ENERGY_TRANSFER_MODES = {
    "dc_core": "DC_core",
    "dc_extended": "DC_extended",
    "ac_single": "AC_single_phase_core",
    "ac_three": "AC_three_phase_core",
}

# Shorthand combinations for energy transfer modes
ENERGY_TRANSFER_SHORTCUTS = {
    "ac": ["ac_single", "ac_three"],
    "dc": ["dc_core", "dc_extended"],
    "all": ["ac_single", "ac_three", "dc_core", "dc_extended"],
}

# Charging mode (energy service) mappings
CHARGING_MODES = {
    "ac": "AC",
    "dc": "DC",
}

# Shorthand combinations for charging modes
CHARGING_MODE_SHORTCUTS = {
    "all": ["ac", "dc"],
    "both": ["ac", "dc"],
}

# Global for cleanup
_charger_ip: Optional[str] = None
_debug_enabled = False

# Certificate paths - relative to this script
# ISO 15118-2 certs (secp256r1, TLS 1.2)
ISO2_CERTS_DIR = SCRIPT_DIR.parent / "certs" / "output" / "iso2" / "certs"
ISO2_PRIVATE_KEYS_DIR = SCRIPT_DIR.parent / "certs" / "output" / "iso2" / "private_keys"
# ISO 15118-20 certs (secp521r1, TLS 1.3)
ISO20_CERTS_DIR = SCRIPT_DIR.parent / "certs" / "output" / "iso20" / "certs"
ISO20_PRIVATE_KEYS_DIR = SCRIPT_DIR.parent / "certs" / "output" / "iso20" / "private_keys"

# Legacy aliases (default to ISO 15118-2)
CERTS_DIR = ISO2_CERTS_DIR
PRIVATE_KEYS_DIR = ISO2_PRIVATE_KEYS_DIR
V2G_ROOT_CA = CERTS_DIR / "v2gRootCACert.pem"


def parse_protocols(protocol_str: str) -> List[str]:
    """
    Parse protocol string into list of protocol names.
    Supports comma-separated values and shortcuts.

    Examples:
        "din" -> ["DIN_SPEC_70121"]
        "din,iso2" -> ["DIN_SPEC_70121", "ISO_15118_2"]
        "iso20" -> ["ISO_15118_20_AC", "ISO_15118_20_DC"]
        "all" -> all protocols
    """
    protocols = []
    parts = [p.strip().lower() for p in protocol_str.split(",")]

    for part in parts:
        if part in PROTOCOL_SHORTCUTS:
            # Expand shortcut
            for p in PROTOCOL_SHORTCUTS[part]:
                if PROTOCOLS[p] not in protocols:
                    protocols.append(PROTOCOLS[p])
        elif part in PROTOCOLS:
            if PROTOCOLS[part] not in protocols:
                protocols.append(PROTOCOLS[part])
        else:
            raise ValueError(f"Unknown protocol: {part}")

    return protocols


def parse_charging_modes(mode_str: str) -> List[str]:
    """
    Parse charging mode string into list of energy service names.
    Supports comma-separated values and shortcuts.

    Examples:
        "ac" -> ["AC"]
        "dc" -> ["DC"]
        "ac,dc" or "all" or "both" -> ["AC", "DC"]
    """
    modes = []
    parts = [p.strip().lower() for p in mode_str.split(",")]

    for part in parts:
        if part in CHARGING_MODE_SHORTCUTS:
            # Expand shortcut
            for m in CHARGING_MODE_SHORTCUTS[part]:
                if CHARGING_MODES[m] not in modes:
                    modes.append(CHARGING_MODES[m])
        elif part in CHARGING_MODES:
            if CHARGING_MODES[part] not in modes:
                modes.append(CHARGING_MODES[part])
        else:
            raise ValueError(f"Unknown charging mode: {part}")

    return modes


def parse_energy_transfer_modes(transfer_str: str) -> List[str]:
    """
    Parse energy transfer mode string into list of mode names.
    Supports comma-separated values and shortcuts.

    Examples:
        "ac_single" -> ["AC_single_phase_core"]
        "dc_extended" -> ["DC_extended"]
        "ac_single,dc_extended" -> ["AC_single_phase_core", "DC_extended"]
        "ac" -> ["AC_single_phase_core", "AC_three_phase_core"]
        "dc" -> ["DC_core", "DC_extended"]
        "all" -> all modes
    """
    modes = []
    parts = [p.strip().lower() for p in transfer_str.split(",")]

    for part in parts:
        if part in ENERGY_TRANSFER_SHORTCUTS:
            # Expand shortcut
            for m in ENERGY_TRANSFER_SHORTCUTS[part]:
                if ENERGY_TRANSFER_MODES[m] not in modes:
                    modes.append(ENERGY_TRANSFER_MODES[m])
        elif part in ENERGY_TRANSFER_MODES:
            if ENERGY_TRANSFER_MODES[part] not in modes:
                modes.append(ENERGY_TRANSFER_MODES[part])
        else:
            raise ValueError(f"Unknown energy transfer mode: {part}")

    return modes


def _symlink_to_pki(source: Path, target: Path) -> None:
    """
    Create a relative symlink from target to source.
    Removes any existing file or symlink at the target path first.
    """
    if target.is_symlink() or target.exists():
        target.unlink()
    rel = os.path.relpath(source, target.parent)
    target.symlink_to(rel)


def setup_pki_for_tls(use_tls13: bool = False) -> Optional[Path]:
    """
    Set up the PKI directory structure expected by iso15118 library.
    Creates symlinks from the PKI directory to our generated certificates.

    The iso15118 library expects certificates under {PKI_PATH}/iso15118_2/certs/
    regardless of TLS version. We select the appropriate source certificates
    based on whether TLS 1.3 (ISO 15118-20, secp521r1) or TLS 1.2
    (ISO 15118-2, secp256r1) is being used.

    Args:
        use_tls13: If True, use ISO 15118-20 certificates (secp521r1/TLS 1.3).
                   If False, use ISO 15118-2 certificates (secp256r1/TLS 1.2).

    Returns:
        Path to the PKI directory if successful, None otherwise.
    """
    if use_tls13:
        source_certs_dir = ISO20_CERTS_DIR
        source_keys_dir = ISO20_PRIVATE_KEYS_DIR
        cert_label = "ISO 15118-20 (secp521r1, TLS 1.3)"
    else:
        source_certs_dir = ISO2_CERTS_DIR
        source_keys_dir = ISO2_PRIVATE_KEYS_DIR
        cert_label = "ISO 15118-2 (secp256r1, TLS 1.2)"

    v2g_root_ca = source_certs_dir / "v2gRootCACert.pem"

    if not v2g_root_ca.exists():
        print(f"Error: V2G Root CA certificate not found at {v2g_root_ca}")
        print("Please run the certificate generation script first:")
        print(f"  cd {source_certs_dir.parent.parent}")
        print("  ./generate_certs.sh")
        return None

    # Create PKI directory structure expected by iso15118 library
    # {PKI_PATH}/iso15118_2/certs/...
    # {PKI_PATH}/iso15118_2/private_keys/...
    # Note: The library always uses "iso15118_2" as subdirectory name,
    # even for ISO 15118-20 certificates.
    pki_path = SCRIPT_DIR / "pki_tls"
    certs_path = pki_path / "iso15118_2" / "certs"
    keys_path = pki_path / "iso15118_2" / "private_keys"
    certs_path.mkdir(parents=True, exist_ok=True)
    keys_path.mkdir(parents=True, exist_ok=True)

    # Symlink V2G Root CA.
    # Always force-replace the target to avoid stale symlinks pointing to a
    # different TLS version (e.g. iso2 Root CA left over when switching
    # to iso20, which would cause "unable to get local issuer certificate").
    target_v2g_root = certs_path / "v2gRootCACert.pem"
    _symlink_to_pki(v2g_root_ca, target_v2g_root)
    print(f"  Linked V2G Root CA -> {v2g_root_ca}")

    # For TLS 1.3, the iso15118 library requires OEM certificates for mutual TLS.
    # The charger does NOT verify client certs (MBEDTLS_SSL_VERIFY_NONE), but the
    # library's get_ssl_context() returns None if these files are missing, causing
    # the connection to fail.
    if use_tls13:
        oem_cert_chain = source_certs_dir / "oemCertChain.pem"
        oem_leaf_key = source_keys_dir / "oemLeaf.key"
        oem_leaf_password = source_keys_dir / "oemLeafPassword.txt"
        secc_leaf_password = source_keys_dir / "seccLeafPassword.txt"

        missing = []
        if not oem_cert_chain.exists():
            missing.append(str(oem_cert_chain))
        if not oem_leaf_key.exists():
            missing.append(str(oem_leaf_key))
        if not oem_leaf_password.exists():
            missing.append(str(oem_leaf_password))

        if missing:
            print("Error: OEM certificates required for TLS 1.3 not found:")
            for f in missing:
                print(f"    {f}")
            print("Please regenerate certificates:")
            print(f"  cd {source_certs_dir.parent.parent}")
            print("  ./generate_certs.sh")
            return None

        # Symlink OEM cert chain
        target_oem_chain = certs_path / "oemCertChain.pem"
        _symlink_to_pki(oem_cert_chain, target_oem_chain)
        print(f"  Linked OEM cert chain -> {oem_cert_chain}")

        # Symlink OEM leaf key
        target_oem_key = keys_path / "oemLeaf.key"
        _symlink_to_pki(oem_leaf_key, target_oem_key)
        print(f"  Linked OEM leaf key -> {oem_leaf_key}")

        # Symlink password files
        target_oem_password = keys_path / "oemLeafPassword.txt"
        _symlink_to_pki(oem_leaf_password, target_oem_password)
        print(f"  Linked OEM leaf password -> {oem_leaf_password}")

        if secc_leaf_password.exists():
            target_secc_password = keys_path / "seccLeafPassword.txt"
            _symlink_to_pki(secc_leaf_password, target_secc_password)
            print(f"  Linked SECC leaf password -> {secc_leaf_password}")

    print(f"  Using {cert_label} certificates")
    print(f"  PKI directory: {pki_path}")
    return pki_path


def get_network_interfaces() -> List[Tuple[str, str]]:
    """
    Get list of available network interfaces with their IPv4 addresses.
    Returns list of (interface_name, ip_address) tuples.
    """
    import netifaces

    interfaces = []
    for iface in netifaces.interfaces():
        addrs = netifaces.ifaddresses(iface)
        # Get IPv4 address if available
        if netifaces.AF_INET in addrs:
            ip = addrs[netifaces.AF_INET][0].get('addr', 'N/A')
            interfaces.append((iface, ip))
        elif netifaces.AF_INET6 in addrs:
            # Fallback to IPv6 if no IPv4
            ip = addrs[netifaces.AF_INET6][0].get('addr', 'N/A').split('%')[0]
            interfaces.append((iface, f"IPv6: {ip}"))

    # Filter out loopback and virtual interfaces for cleaner list
    filtered = []
    for iface, ip in interfaces:
        if iface.startswith(("lo", "veth", "br-", "docker")):
            continue
        filtered.append((iface, ip))

    return filtered if filtered else interfaces


def enable_debug_mode(charger_ip: str) -> bool:
    """
    Enable debug mode on the WARP charger via HTTP API.
    Returns True if successful.
    """
    global _debug_enabled

    if not HAS_REQUESTS:
        print("Warning: 'requests' module not installed, cannot control charger debug mode")
        print("  Install with: pip install requests")
        return False

    url = f"http://{charger_ip}/iso15118/debug_update"
    try:
        print(f"Enabling debug mode on {charger_ip}...")
        response = requests.put(url, json={"enable": True, "current": 6000, "phases": 3}, timeout=5)
        if response.status_code == 200:
            print("  Debug mode enabled successfully")
            _debug_enabled = True
            return True
        else:
            print(f"  Failed: HTTP {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        print(f"  Failed: Could not connect to {charger_ip}")
        return False
    except requests.exceptions.Timeout:
        print(f"  Failed: Connection timeout")
        return False
    except Exception as e:
        print(f"  Failed: {e}")
        return False


def disable_debug_mode(charger_ip: str) -> bool:
    """
    Disable debug mode on the WARP charger via HTTP API.
    Returns True if successful.
    """
    global _debug_enabled

    if not HAS_REQUESTS:
        return False

    url = f"http://{charger_ip}/iso15118/debug_update"
    try:
        print(f"\nDisabling debug mode on {charger_ip}...")
        response = requests.put(url, json={"enable": False, "current": 6000, "phases": 3}, timeout=5)
        if response.status_code == 200:
            print("  Debug mode disabled successfully")
            _debug_enabled = False
            return True
        else:
            print(f"  Failed: HTTP {response.status_code}")
            return False
    except Exception as e:
        print(f"  Failed: {e}")
        return False


def build_evcc_config(
    protocols: List[str],
    energy_services: List[str],
    energy_transfer_mode: str,
    charge_loop_cycles: int = 10,
    use_tls: bool = False,
    enforce_tls: bool = False,
) -> EVCCConfig:
    """
    Build an EVCCConfig object from parameters.

    Args:
        protocols: List of protocol names to support
        energy_services: List of energy services (AC/DC)
        energy_transfer_mode: Energy transfer mode string
        charge_loop_cycles: Number of charge loop iterations
        use_tls: Request TLS in SDP (security byte = 0x00)
        enforce_tls: Require TLS even if SECC doesn't offer it
    """
    config_dict = {
        "supportedProtocols": protocols,
        "supportedEnergyServices": energy_services,
        "energyTransferMode": energy_transfer_mode,
        "isCertInstallNeeded": False,
        "useTls": use_tls,
        "enforceTls": enforce_tls,
        "chargeLoopCycle": charge_loop_cycles,
        "sdpRetryCycles": 3,
        "maxContractCerts": 3,
        "maxSupportingPoints": 1024,
    }

    config = EVCCConfig(**config_dict)
    config.load_raw_values()
    return config


async def run_evcc(config: EVCCConfig, interface: str):
    """
    Run the EVCC simulator.
    """
    print("\n" + "=" * 60)
    print("EVCC Configuration:")
    print("=" * 60)
    for key, value in config.dict().items():
        if not key.startswith("raw"):
            print(f"  {key:30}: {value}")
    print("=" * 60 + "\n")

    # Set up environment for iso15118
    os.environ["NETWORK_INTERFACE"] = interface
    os.environ["LOG_LEVEL"] = os.environ.get("LOG_LEVEL", "INFO")

    # Load shared settings (required for EXI codec and message logging)
    load_shared_settings()

    # Create and start the EVCC handler
    handler = EVCCHandler(
        evcc_config=config,
        iface=interface,
        exi_codec=ExificientEXICodec(),
        ev_controller=SimEVController(config),
    )

    await handler.start()


def select_from_list(prompt: str, options: List[str], default: int = 0) -> int:
    """
    Display a list of options and let user select one.
    Returns the index of the selected option.
    """
    print(f"\n{prompt}")
    for i, option in enumerate(options, 1):
        marker = " [default]" if i - 1 == default else ""
        print(f"  {i}. {option}{marker}")

    while True:
        try:
            choice = input(f"\nSelect [1-{len(options)}] (default: {default + 1}): ").strip()
            if not choice:
                return default
            idx = int(choice) - 1
            if 0 <= idx < len(options):
                return idx
            print(f"Please enter a number between 1 and {len(options)}")
        except ValueError:
            print("Please enter a valid number")


def select_multiple_from_list(prompt: str, options: List[str], defaults: Optional[List[int]] = None) -> List[int]:
    """
    Display a list of options and let user select multiple.
    Returns the list of selected indices.
    """
    if defaults is None:
        defaults = [0]

    print(f"\n{prompt}")
    for i, option in enumerate(options, 1):
        marker = " [default]" if i - 1 in defaults else ""
        print(f"  {i}. {option}{marker}")

    default_str = ",".join(str(d + 1) for d in defaults)
    while True:
        try:
            choice = input(f"\nSelect [1-{len(options)}] (comma-separated, default: {default_str}): ").strip()
            if not choice:
                return defaults
            indices: List[int] = []
            valid = True
            for part in choice.split(","):
                idx = int(part.strip()) - 1
                if 0 <= idx < len(options):
                    if idx not in indices:
                        indices.append(idx)
                else:
                    print(f"Invalid selection: {part.strip()}. Please enter numbers between 1 and {len(options)}")
                    valid = False
                    break
            if valid:
                return indices
        except ValueError:
            print("Please enter valid numbers separated by commas")


def input_with_default(prompt: str, default: str) -> str:
    """Get input with a default value."""
    result = input(f"{prompt} [{default}]: ").strip()
    return result if result else default


def interactive_mode() -> argparse.Namespace:
    """
    Interactive configuration wizard.
    Returns an argparse.Namespace with all settings.
    """
    print("\n" + "=" * 60)
    print("   WARP Charger EV Simulator - Interactive Setup")
    print("=" * 60)

    # Charger IP
    print("\n--- Charger Connection ---")
    charger_ip = input("Charger IP (leave empty to skip auto debug mode): ").strip()

    # Network interface
    print("\n--- Network Interface ---")
    interfaces = get_network_interfaces()

    if interfaces:
        print("Available interfaces:")
        iface_options = [f"{iface} ({ip})" for iface, ip in interfaces]
        iface_idx = select_from_list("Select network interface:", iface_options, default=0)
        interface = interfaces[iface_idx][0]
    else:
        interface = input_with_default("Network interface", "eth0")

    # Protocol
    print("\n--- Protocol Selection ---")
    protocol_options = [
        "DIN SPEC 70121 only",
        "ISO 15118-2 only",
        "DIN + ISO 15118-2",
        "ISO 15118-20 AC",
        "ISO 15118-20 DC",
        "ISO 15118-20 (AC + DC)",
        "All protocols",
    ]
    protocol_values = ["din", "iso2", "din,iso2", "iso20ac", "iso20dc", "iso20", "all"]
    protocol_idx = select_from_list("Select protocol:", protocol_options, default=2)
    protocol = protocol_values[protocol_idx]

    # Charging mode
    print("\n--- Charging Mode (Energy Services) ---")
    mode_options = ["AC charging", "DC charging"]
    mode_indices = select_multiple_from_list("Select charging mode(s):", mode_options, defaults=[0])
    mode_values = ["ac", "dc"]
    mode = ",".join(mode_values[i] for i in mode_indices)

    # Energy transfer mode - show options based on selected charging modes
    print("\n--- Energy Transfer Mode ---")
    transfer_options = []
    transfer_values = []
    if 0 in mode_indices:  # AC selected
        transfer_options.extend(["AC_single_phase_core", "AC_three_phase_core"])
        transfer_values.extend(["ac_single", "ac_three"])
    if 1 in mode_indices:  # DC selected
        transfer_options.extend(["DC_core", "DC_extended"])
        transfer_values.extend(["dc_core", "dc_extended"])

    # Default to last option (typically "extended" or "three_phase")
    default_transfer = [len(transfer_options) - 1] if transfer_options else [0]
    transfer_indices = select_multiple_from_list("Select energy transfer mode(s):", transfer_options, defaults=default_transfer)
    energy_transfer = ",".join(transfer_values[i] for i in transfer_indices)

    # Charge loop cycles
    print("\n--- Simulation Settings ---")
    loops_str = input_with_default("Charge loop cycles", "10")
    try:
        loops = int(loops_str)
    except ValueError:
        loops = 10

    # TLS options
    print("\n--- TLS Security ---")
    tls_options = ["No TLS (plain TCP)", "TLS (encrypted)"]
    tls_idx = select_from_list("Select security mode:", tls_options, default=0)
    use_tls = tls_idx == 1

    # Create namespace
    args = argparse.Namespace(
        interactive=False,  # Already processed
        charger=charger_ip if charger_ip else None,
        protocol=protocol,
        mode=mode,
        energy_transfer=energy_transfer,
        interface=interface,
        loops=loops,
        tls=use_tls,
        no_debug=False,
        list_interfaces=False,
    )

    # Summary
    print("\n" + "=" * 60)
    print("Configuration Summary:")
    print("=" * 60)
    print(f"  Charger IP:          {args.charger or '(not set)'}")
    print(f"  Network interface:   {args.interface}")
    print(f"  Protocol:            {args.protocol}")
    print(f"  Charging modes:      {args.mode}")
    print(f"  Energy transfer:     {args.energy_transfer}")
    print(f"  Charge loop cycles:  {args.loops}")
    print(f"  TLS enabled:         {args.tls}")
    print("=" * 60)

    confirm = input("\nStart simulator? [Y/n]: ").strip().lower()
    if confirm and confirm != 'y':
        print("Aborted.")
        sys.exit(0)

    return args


def cleanup_handler(signum, frame):
    """Signal handler for cleanup on exit."""
    global _charger_ip, _debug_enabled

    print("\n\nReceived interrupt signal, cleaning up...")

    if _charger_ip and _debug_enabled:
        disable_debug_mode(_charger_ip)

    sys.exit(0)


def main():
    global _charger_ip

    parser = argparse.ArgumentParser(
        description="WARP Charger EV Simulator - Test ISO 15118 debug mode",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Protocols:
  din      - DIN SPEC 70121
  iso2     - ISO 15118-2
  iso20ac  - ISO 15118-20 AC
  iso20dc  - ISO 15118-20 DC
  iso20    - ISO 15118-20 AC + DC (shortcut)
  all      - All protocols

Charging modes (energy services):
  ac       - AC charging
  dc       - DC charging
  all/both - AC + DC (let SECC choose)

Energy transfer modes:
  ac_single  - AC_single_phase_core
  ac_three   - AC_three_phase_core
  dc_core    - DC_core
  dc_extended - DC_extended
  ac         - All AC modes (shortcut)
  dc         - All DC modes (shortcut)
  all        - All modes

Examples:
  %(prog)s -c 192.168.0.33                       # Default: DIN + ISO2, AC three-phase
  %(prog)s -c 192.168.0.33 -p din                # DIN only
  %(prog)s -c 192.168.0.33 -p iso2,din           # ISO2 + DIN
  %(prog)s -c 192.168.0.33 -p iso20dc -m dc      # ISO 15118-20 DC
  %(prog)s -c 192.168.0.33 -m ac,dc              # Support both AC and DC
  %(prog)s -c 192.168.0.33 -e dc_extended        # Request DC extended mode
  %(prog)s -c 192.168.0.33 -e ac_single,ac_three # Support both AC modes
  %(prog)s -c 192.168.0.33 -p all -m all -e all  # Support everything
  %(prog)s -c 192.168.0.33 --tls                 # With TLS encryption (TLS 1.2)
  %(prog)s -i                                    # Interactive mode
  %(prog)s --list-interfaces                     # Show available interfaces
        """,
    )

    parser.add_argument(
        "-i", "--interactive",
        action="store_true",
        help="Interactive configuration mode",
    )

    parser.add_argument(
        "-c", "--charger",
        type=str,
        metavar="IP",
        help="Charger IP address (enables automatic debug mode control)",
    )

    parser.add_argument(
        "-p", "--protocol",
        type=str,
        default="din,iso2",
        metavar="PROTO",
        help="Protocol(s): din, iso2, iso20ac, iso20dc, iso20, all (comma-separated, default: din,iso2)",
    )

    parser.add_argument(
        "-m", "--mode",
        type=str,
        default="ac",
        metavar="MODE",
        help="Charging mode(s): ac, dc, all/both (comma-separated, default: ac)",
    )

    parser.add_argument(
        "-e", "--energy-transfer",
        type=str,
        default=None,
        metavar="TRANSFER",
        help="Energy transfer mode(s): ac_single, ac_three, dc_core, dc_extended, ac, dc, all (comma-separated, default: based on --mode)",
    )

    parser.add_argument(
        "-n", "--interface",
        type=str,
        metavar="IFACE",
        help="Network interface (auto-detected if not specified)",
    )

    parser.add_argument(
        "--loops",
        type=int,
        default=10,
        help="Number of charge loop cycles (default: 10)",
    )

    parser.add_argument(
        "--no-debug",
        action="store_true",
        help="Don't automatically control charger debug mode",
    )

    parser.add_argument(
        "--tls",
        action="store_true",
        help="Enable TLS encryption (TLS 1.3 auto-enabled for ISO 15118-20 protocols)",
    )

    parser.add_argument(
        "--list-interfaces",
        action="store_true",
        help="List available network interfaces and exit",
    )

    args = parser.parse_args()

    # Show help if no arguments provided (require at least -c, -i, or --list-interfaces)
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(0)

    # List interfaces mode
    if args.list_interfaces:
        interfaces = get_network_interfaces()
        if interfaces:
            print("Available network interfaces:")
            for iface, ip in interfaces:
                print(f"  {iface:20} {ip}")
        else:
            print("No network interfaces found")
        sys.exit(0)

    # Interactive mode
    if args.interactive:
        args = interactive_mode()

    # Auto-detect interface if not specified
    if not args.interface:
        interfaces = get_network_interfaces()
        if interfaces:
            args.interface = interfaces[0][0]
            print(f"Auto-detected network interface: {args.interface}")
        else:
            print("Error: No network interface found. Please specify with --interface")
            sys.exit(1)

    # Default energy transfer mode based on charging mode
    if not args.energy_transfer:
        args.energy_transfer = "dc_extended" if args.mode == "dc" else "ac_three"

    # Parse protocols
    try:
        protocols = parse_protocols(args.protocol)
    except ValueError as e:
        print(f"Error: {e}")
        print("Valid protocols: din, iso2, iso20ac, iso20dc, iso20, all")
        sys.exit(1)

    # Parse charging modes
    try:
        energy_services = parse_charging_modes(args.mode)
    except ValueError as e:
        print(f"Error: {e}")
        print("Valid charging modes: ac, dc, all, both")
        sys.exit(1)

    # Parse energy transfer modes
    try:
        energy_transfer_modes = parse_energy_transfer_modes(args.energy_transfer)
    except ValueError as e:
        print(f"Error: {e}")
        print("Valid energy transfer modes: ac_single, ac_three, dc_core, dc_extended, ac, dc, all")
        sys.exit(1)

    # The first energy transfer mode is the one we'll request
    # (EVCCConfig only supports a single energyTransferMode)
    primary_energy_transfer_mode = energy_transfer_modes[0]

    # Setup signal handlers for cleanup
    signal.signal(signal.SIGINT, cleanup_handler)
    signal.signal(signal.SIGTERM, cleanup_handler)

    # Setup TLS if requested
    # Note: TLS is always supported by the charger. When the EV requests TLS in SDP,
    # the charger will respond with TLS. No need to enable it on the charger side.
    use_tls = getattr(args, 'tls', False)
    pki_path = None

    # Detect if any ISO 15118-20 protocol is selected, which requires TLS 1.3
    ISO20_PROTOCOLS = {"ISO_15118_20_AC", "ISO_15118_20_DC"}
    has_iso20 = bool(ISO20_PROTOCOLS.intersection(protocols))

    if use_tls:
        print("\n--- TLS Setup ---")

        # Auto-enable TLS 1.3 when ISO 15118-20 protocols are selected
        # ISO 15118-20 requires TLS 1.3 per [V2G20-2356]
        if has_iso20:
            os.environ["ENABLE_TLS_1_3"] = "true"
            print("  TLS 1.3 enabled (required for ISO 15118-20)")

        pki_path = setup_pki_for_tls(use_tls13=has_iso20)
        if not pki_path:
            print("Error: TLS setup failed. Cannot continue.")
            sys.exit(1)
        # Set PKI_PATH environment variable for iso15118 library
        os.environ["PKI_PATH"] = str(pki_path)
        print(f"  TLS enabled, PKI_PATH={pki_path}")
    elif has_iso20:
        print("\nWarning: ISO 15118-20 protocols selected without --tls.")
        print("  ISO 15118-20 requires TLS 1.3. Consider adding --tls.")

    # Enable debug mode on charger if requested
    if args.charger and not args.no_debug:
        _charger_ip = args.charger
        if not enable_debug_mode(args.charger):
            print("\nWarning: Could not enable debug mode on charger")
            print("The simulator will still try to connect...")

    # Build configuration
    # Note: EVCCConfig only supports a single energyTransferMode, so we use the first one
    # The energy_services list is used to indicate what the EV supports (AC, DC, or both)

    config = build_evcc_config(
        protocols=protocols,
        energy_services=energy_services,
        energy_transfer_mode=primary_energy_transfer_mode,
        charge_loop_cycles=args.loops,
        use_tls=use_tls,
        enforce_tls=use_tls,  # If TLS requested, enforce it
    )

    # Print what we're offering
    print(f"\nEnergy services: {', '.join(energy_services)}")
    print(f"Energy transfer modes available: {', '.join(energy_transfer_modes)}")
    print(f"Primary energy transfer mode: {primary_energy_transfer_mode}")

    # Run the simulator
    if use_tls:
        tls_version = "TLS 1.3" if has_iso20 else "TLS 1.2"
        tls_info = f" with {tls_version}"
    else:
        tls_info = ""
    print(f"\nStarting EV simulator on interface {args.interface}{tls_info}...")
    print("Press Ctrl+C to stop\n")

    try:
        asyncio.run(run_evcc(config, args.interface))
    except KeyboardInterrupt:
        pass
    finally:
        # Cleanup
        if _charger_ip and _debug_enabled:
            disable_debug_mode(_charger_ip)


if __name__ == "__main__":
    main()
