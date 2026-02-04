# ISO 15118 Module

The ISO15118 module is the coordinator for all versions and layers of the ISO 15118 standard. It enables high-level communication between the WARP Charger and electric vehicles for advanced features like State of Charge (SoC) reading, EV fingerprinting (additionally PnC + bidirectional charging planned for the long term).

## Architecture

The module coordinates between the following protocols and layers:

| Layer | Protocol | Description |
|-------|----------|-------------|
| Hardware | SPI/QCA700X | Low-level SPI communication with the QCA700X HomePlug modem |
| Data Link | SLAC | Signal-Level-Attenuation-Characterization (ISO 15118-3) |
| Network | IPv6/ICMPv6/NDP | Neighbor Discovery Protocol with Neighbor Solicitation/Advertisement |
| Transport | IPv6/UDP/SDP | SECC Discovery Protocol |
| Application | IPv6/TCP | DIN SPEC 70121, ISO 15118-2, ISO 15118-20 |

The EXI message encoding/decoding is based on [libcbv2g](https://github.com/AudreyMusic/cbv2g).

## Supported Standards

### IEC 61851-1 (Fallback)
- Basic charging via PWM on Control Pilot (CP)
- Used when ISO 15118 communication fails or is not supported by the EV

### DIN SPEC 70121
- Read EV MAC address (+maybe other fingerprinting)
- Read SoC via DC charging session

### ISO 15118-2
- Read EV MAC address (+maybe other fingerprinting)
- Read SoC via DC charging session
- AC charging support
- Optional TLS encryption

### ISO 15118-20
- Read EV MAC address (+maybe other fingerprinting)
- Read SoC and charge via AC (continuous)
- Requires TLS encryption
- Plug & Charge (PnC) support (long term feature)
- Bidirectional charging support (long term feature)

## Communication Flow

```
┌────────────────────────────────────────────────────────────────────┐
│                         EV Connected                               │
└────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌────────────────────────────────────────────────────────────────────┐
│                    Attempt SLAC Connection                         │
│            (Signal-Level-Attenuation-Characterization)             │
└────────────────────────────────────────────────────────────────────┘
                                │
                ┌───────────────┴───────────────┐
                │                               │
                ▼                               ▼
        ┌───────────────┐               ┌───────────────┐
        │ SLAC Success  │               │ SLAC Failed   │
        │ (MAC read)    │               │               │
        └───────────────┘               └───────────────┘
                │                               │
                ▼                               ▼
┌─────────────────────────────────────┐ ┌───────────────┐
│              SDP                    │ │  IEC 61851    │
│  (SECC Discovery Protocol)          │ │  (PWM only)   │
│                                     │ └───────────────┘
│  EV announces supported standards:  │
│  • DIN 70121 (no TLS)               │
│  • ISO 15118-2 (with/without TLS)   │
│  • ISO 15118-20 (with TLS)          │
└─────────────────────────────────────┘
                │
                │ No supported standard?
                ├────────────────────────────────────► IEC 61851
                │
                ▼
┌──────────────────────────────────────────────────────────────────┐
│                    WARP Priority Selection                       │
│      ISO 15118-20 > ISO 15118-2/TLS > ISO 15118-2 > DIN 70121    │
└──────────────────────────────────────────────────────────────────┘
                │
     ┌──────────┼────────────┬───────────┐
     │          │            │           │
     ▼          ▼            ▼           ▼
┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐
│ISO 15118│ │ISO 15118│ │ISO 15118│ │  DIN    │
│   -20   │ │ -2/TLS  │ │   -2    │ │ 70121   │
└─────────┘ └─────────┘ └─────────┘ └─────────┘
     │           │           │           │
     ▼           ▼           ▼           ▼
┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐
│Cont.    │ │Start DC │ │Start DC │ │Start DC │
│SoC read │ │Read SoC │ │Read SoC │ │Read SoC │
│& charge │ │Stop DC  │ │Stop DC  │ │Stop DC  │
│via ISO20│ │Charge   │ │Charge   │ │         │
│         │ │via ISO2 │ │via ISO2 │ │         │
└─────────┘ └─────────┘ └─────────┘ └─────────┘
                                         │
                                         ▼
                                  ┌─────────────┐
                                  │  IEC 61851  │
                                  │  (PWM only) │
                                  └─────────────┘
```

## User Configuration

| Option | Type | Description |
|--------|------|-------------|
| Plug & Charge | Yes/No | Enable Autocharge via ISO 15118 |
| Read SoC | Yes/No | Read State of Charge via ISO 15118 |
| Charge via ISO 15118 | Yes/No | Enable charging via ISO 15118 (allows arbitrary phase switching and currents) |
| Minimum Charge Current | 1000-22000 mA | Minimum charging current |

## System Interface

### SoC Reporting
- SoC values are written to the `meters` module into the wallbox meter

### EV Fingerprinting
- The ISO 15118 module collects the last 8 MAC addresses
- MAC addresses can be used for EV identification/authentication

### Current Control
- Charging current is controlled via the EVSE module
- EVSE allows currents below 6A only when using ISO 15118
- When using IEC 61851 currents below 6A result in charging stop

### Phase Switching
- Phase switching is negotiated between the EVSE and ISO 15118 module
- ISO 15118 charging allows arbitrary phase switching during session
