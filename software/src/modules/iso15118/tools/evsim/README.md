# ISO 15118 EV simulator for WARP4 Charger in debug mode

Setup for using the [EcoG-io/iso15118](https://github.com/EcoG-io/iso15118) library
to simulate an EV for testing the WARP4 Charger ISO 15118 debug mode.

## Prerequisites

- Python 3.9+ (tested with Python 3.12)
- Java Runtime Environment (JRE) for the EXI codec
- Network interface connected to the same network as the WARP4 charger
- Linux (tested on Ubuntu/Debian)

## Quick Start

### 1. Clone the iso15118 repository

```bash
cd src/modules/iso15118/tools/evsim
git clone --depth 1 https://github.com/EcoG-io/iso15118.git
```

### 2. Set up Python virtual environment

The virtual environment is placed in `software/.venv-evsim` to avoid PlatformIO trying
to compile C files from Python packages.

```bash
# From the software/ directory:
python3 -m venv .venv-evsim
source .venv-evsim/bin/activate
pip install -r src/modules/iso15118/tools/evsim/requirements.txt
```

### 3. Install Java (if not already installed)

```bash
sudo apt update && sudo apt install -y default-jre
```

### 4. Show help

```bash
./run_evsim.py
```
