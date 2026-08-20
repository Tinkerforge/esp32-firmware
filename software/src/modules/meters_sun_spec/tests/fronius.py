#!/usr/bin/env -S uv run --script
#
# /// script
# dependencies = [
#     "pymodbus == 3.12.1",
# ]
# ///

import logging
import struct
from argparse import ArgumentParser
from pymodbus import pymodbus_apply_logging_config
from pymodbus.server import StartTcpServer
from pymodbus.datastore import ModbusSparseDataBlock, ModbusDeviceContext, ModbusServerContext


def string(string, length):
    registers = [0] * length

    for i, c in enumerate(string[: length * 2]):
        if i % 2 == 0:
            registers[i // 2] = ord(c) << 8
        else:
            registers[i // 2] |= ord(c)

    return tuple(registers)


def uint32be(value):
    return struct.unpack('>2H', struct.pack('>I', value))


def int16(value):
    return struct.unpack('>H', struct.pack('>h', value))[0]


def main():
    parser = ArgumentParser()
    parser.add_argument('-d', '--debug', action='store_true')
    parser.add_argument('-p', '--port', type=int, default=5020)
    parser.add_argument('model', choices=['gen24-int', 'gen24plus-int', 'verto-int', 'vertoplus-int'])

    args = parser.parse_args()

    if args.debug:
        print('Enabling debug logging')
        pymodbus_apply_logging_config(logging.DEBUG)
        logging.basicConfig(level=logging.DEBUG, format='%(asctime)s %(levelname)s %(message)s')

    data = ModbusSparseDataBlock()

    data.setValues(
        40000 + 1,
        [
            0x5375,
            0x6E53,  # Sun Spec ID
        ],
    )

    if args.model == 'gen24-int':
        data.setValues(
            40002 + 1,
            [
                1,                              # ID
                65,                             # L
                *string('Fronius', 16),         # Mn
                *string('Symo GEN24 8.0', 16),  # Md
                *string('', 8),                 # Opt
                *string('1.8.10-0', 8),         # Vr
                *string('1234', 16),            # SN
                1,                              # DA
            ],
        )

        data.setValues(
            40069 + 1,
            [
                103,  # ID
                50,   # L
            ],
        )

        data.setValues(
            40121 + 1,
            [
                120,  # ID
                26,   # L
            ],
        )

        data.setValues(
            40149 + 1,
            [
                121,  # ID
                30,   # L
            ],
        )

        data.setValues(
            40181 + 1,
            [
                122,  # ID
                44,   # L
            ],
        )

        data.setValues(
            40227 + 1,
            [
                123,  # ID
                24,   # L
            ],
        )

        data.setValues(
            40253 + 1,
            [
                160,                   # ID
                48,                    # L
                0,                     # DCA_SF
                0,                     # DCV_SF
                0,                     # DCW_SF
                0,                     # DCWH_SF
                *uint32be(0),          # Evt
                2,                     # N
                0,                     # TmsPer
                1,                     # module/1/ID
                *string('MPPT 1', 8),  # module/1/IDStr
                5,                     # module/1/DCA
                200,                   # module/1/DCV
                1000,                  # module/1/DCW
                *uint32be(987654321),  # module/1/DCWH
                *uint32be(0),          # module/1/Tms
                0x8000,                # module/1/Tmp
                0,                     # module/1/DCSt
                *uint32be(0),          # module/1/DCEvt
                2,                     # module/2/ID
                *string('MPPT 2', 8),  # module/2/IDStr
                7,                     # module/2/DCA
                100,                   # module/2/DCV
                700,                   # module/2/DCW
                *uint32be(123456789),  # module/2/DCWH
                *uint32be(0),          # module/2/Tms
                0x8000,                # module/2/Tmp
                0,                     # module/2/DCSt
                *uint32be(0),          # module/2/DCEvt
            ],
        )

        data.setValues(
            40303 + 1,
            [
                0xFFFF,  # ID
                0,       # L
            ],
        )
    elif args.model == 'gen24plus-int':
        data.setValues(
            40002 + 1,
            [
                1,                               # ID
                65,                              # L
                *string('Fronius', 16),          # Mn
                *string('Symo GEN24 10.0', 16),  # Md
                *string('', 8),                  # Opt
                *string('1.8.10-0', 8),          # Vr
                *string('1234-Plus', 16),        # SN
                1,                               # DA
            ],
        )

        data.setValues(
            40069 + 1,
            [
                103,  # ID
                50,   # L
            ],
        )

        data.setValues(
            40121 + 1,
            [
                120,  # ID
                26,   # L
            ],
        )

        data.setValues(
            40149 + 1,
            [
                121,  # ID
                30,   # L
            ],
        )

        data.setValues(
            40181 + 1,
            [
                122,  # ID
                44,   # L
            ],
        )

        data.setValues(
            40227 + 1,
            [
                123,  # ID
                24,   # L
            ],
        )

        data.setValues(
            40253 + 1,
            [
                160,                       # ID
                88,                        # L
                0,                         # DCA_SF
                0,                         # DCV_SF
                0,                         # DCW_SF
                0,                         # DCWH_SF
                *uint32be(0),              # Evt
                2,                         # N
                0,                         # TmsPer
                1,                         # module/1/ID
                *string('MPPT 1', 8),      # module/1/IDStr
                5,                         # module/1/DCA
                200,                       # module/1/DCV
                1000,                      # module/1/DCW
                *uint32be(987654321),      # module/1/DCWH
                *uint32be(0),              # module/1/Tms
                0x8000,                    # module/1/Tmp
                0,                         # module/1/DCSt
                *uint32be(0),              # module/1/DCEvt
                2,                         # module/2/ID
                *string('MPPT 2', 8),      # module/2/IDStr
                7,                         # module/2/DCA
                100,                       # module/2/DCV
                700,                       # module/2/DCW
                *uint32be(123456789),      # module/2/DCWH
                *uint32be(0),              # module/2/Tms
                0x8000,                    # module/2/Tmp
                0,                         # module/2/DCSt
                *uint32be(0),              # module/2/DCEvt
                3,                         # module/3/ID
                *string('StCha 3', 8),     # module/3/IDStr
                10,                        # module/3/DCA
                300,                       # module/3/DCV
                3000,                      # module/3/DCW
                *uint32be(345678912),      # module/3/DCWH
                *uint32be(0),              # module/3/Tms
                0x8000,                    # module/3/Tmp
                0,                         # module/3/DCSt
                *uint32be(0),              # module/3/DCEvt
                4,                         # module/4/ID
                *string('StDisCha 4', 8),  # module/4/IDStr
                0,                         # module/4/DCA
                300,                       # module/4/DCV
                0,                         # module/4/DCW
                *uint32be(234567891),      # module/4/DCWH
                *uint32be(0),              # module/4/Tms
                0x8000,                    # module/4/Tmp
                0,                         # module/4/DCSt
                *uint32be(0),              # module/4/DCEvt
            ],
        )

        data.setValues(
            40343 + 1,
            [
                124,        # ID
                24,         # L
                0,          # WChaMax
                100,        # WChaGra
                0,          # WDisChaGra
                0,          # StorCtl_Mod [R/W]
                0,          # VAChaMax [R/W]
                0,          # MinRsvPct [R/W]
                4223,       # ChaState
                0xFFFF,     # StorAval
                0xFFFF,     # InBatV
                3,          # ChaSt
                10000,      # OutWRte
                10000,      # InWRte
                0xFFFF,     # InOutWRte_WinTms
                28800,      # InOutWRte_RvrtTms
                0xFFFF,     # InOutWRte_RmpTms
                0,          # ChaGriSet
                0,          # WChaMax_SF
                0,          # WChaDisChaGra_SF
                0x8000,     # VAChaMax_SF
                int16(-2),  # MinRsvPct_SF
                int16(-2),  # ChaState_SF
                0x8000,     # StorAval_SF
                0x8000,     # InBatV_SF
                int16(-2),  # InOutWRte_SF
            ],
        )

        data.setValues(
            40369 + 1,
            [
                0xFFFF,  # ID
                0,       # L
            ],
        )
    elif args.model == 'verto-int':
        data.setValues(
            40002 + 1,
            [
                1,                          # ID
                65,                         # L
                *string('Fronius', 16),     # Mn
                *string('Verto 15.0', 16),  # Md
                *string('', 8),             # Opt
                *string('1.8.10-0', 8),     # Vr
                *string('1234', 16),        # SN
                1,                          # DA
            ],
        )

        data.setValues(
            40069 + 1,
            [
                103,  # ID
                50,   # L
            ],
        )

        data.setValues(
            40121 + 1,
            [
                120,  # ID
                26,   # L
            ],
        )

        data.setValues(
            40149 + 1,
            [
                121,  # ID
                30,   # L
            ],
        )

        data.setValues(
            40181 + 1,
            [
                122,  # ID
                44,   # L
            ],
        )

        data.setValues(
            40227 + 1,
            [
                123,  # ID
                24,   # L
            ],
        )

        data.setValues(
            40253 + 1,
            [
                160,                   # ID
                48,                    # L
                0,                     # DCA_SF
                0,                     # DCV_SF
                0,                     # DCW_SF
                0,                     # DCWH_SF
                *uint32be(0),          # Evt
                2,                     # N
                0,                     # TmsPer
                1,                     # module/1/ID
                *string('MPPT 1', 8),  # module/1/IDStr
                5,                     # module/1/DCA
                200,                   # module/1/DCV
                1000,                  # module/1/DCW
                *uint32be(987654321),  # module/1/DCWH
                *uint32be(0),          # module/1/Tms
                0x8000,                # module/1/Tmp
                0,                     # module/1/DCSt
                *uint32be(0),          # module/1/DCEvt
                2,                     # module/2/ID
                *string('MPPT 2', 8),  # module/2/IDStr
                7,                     # module/2/DCA
                100,                   # module/2/DCV
                700,                   # module/2/DCW
                *uint32be(123456789),  # module/2/DCWH
                *uint32be(0),          # module/2/Tms
                0x8000,                # module/2/Tmp
                0,                     # module/2/DCSt
                *uint32be(0),          # module/2/DCEvt
                3,                     # module/3/ID
                *string('MPPT 3', 8),  # module/3/IDStr
                9,                     # module/3/DCA
                100,                   # module/3/DCV
                900,                   # module/3/DCW
                *uint32be(111111111),  # module/3/DCWH
                *uint32be(0),          # module/3/Tms
                0x8000,                # module/3/Tmp
                0,                     # module/3/DCSt
                *uint32be(0),          # module/3/DCEvt
            ],
        )

        data.setValues(
            40303 + 1,
            [
                0xFFFF,  # ID
                0,       # L
            ],
        )
    elif args.model == 'vertoplus-int':
        data.setValues(
            40002 + 1,
            [
                1,                          # ID
                65,                         # L
                *string('Fronius', 16),     # Mn
                *string('Verto 24.0', 16),  # Md
                *string('', 8),             # Opt
                *string('1.8.10-0', 8),     # Vr
                *string('1234-Plus', 16),   # SN
                1,                          # DA
            ],
        )

        data.setValues(
            40069 + 1,
            [
                103,  # ID
                50,   # L
            ],
        )

        data.setValues(
            40121 + 1,
            [
                120,  # ID
                26,   # L
            ],
        )

        data.setValues(
            40149 + 1,
            [
                121,  # ID
                30,   # L
            ],
        )

        data.setValues(
            40181 + 1,
            [
                122,  # ID
                44,   # L
            ],
        )

        data.setValues(
            40227 + 1,
            [
                123,  # ID
                24,   # L
            ],
        )

        data.setValues(
            40253 + 1,
            [
                160,                       # ID
                108,                       # L
                0,                         # DCA_SF
                0,                         # DCV_SF
                0,                         # DCW_SF
                0,                         # DCWH_SF
                *uint32be(0),              # Evt
                2,                         # N
                0,                         # TmsPer
                1,                         # module/1/ID
                *string('MPPT 1', 8),      # module/1/IDStr
                5,                         # module/1/DCA
                200,                       # module/1/DCV
                1000,                      # module/1/DCW
                *uint32be(987654321),      # module/1/DCWH
                *uint32be(0),              # module/1/Tms
                0x8000,                    # module/1/Tmp
                0,                         # module/1/DCSt
                *uint32be(0),              # module/1/DCEvt
                2,                         # module/2/ID
                *string('MPPT 2', 8),      # module/2/IDStr
                7,                         # module/2/DCA
                100,                       # module/2/DCV
                700,                       # module/2/DCW
                *uint32be(123456789),      # module/2/DCWH
                *uint32be(0),              # module/2/Tms
                0x8000,                    # module/2/Tmp
                0,                         # module/2/DCSt
                *uint32be(0),              # module/2/DCEvt
                3,                         # module/3/ID
                *string('MPPT 3', 8),      # module/3/IDStr
                9,                         # module/3/DCA
                100,                       # module/3/DCV
                900,                       # module/3/DCW
                *uint32be(111111111),      # module/3/DCWH
                *uint32be(0),              # module/3/Tms
                0x8000,                    # module/3/Tmp
                0,                         # module/3/DCSt
                *uint32be(0),              # module/3/DCEvt
                4,                         # module/4/ID
                *string('StCha 4', 8),     # module/4/IDStr
                10,                        # module/4/DCA
                300,                       # module/4/DCV
                3000,                      # module/4/DCW
                *uint32be(345678912),      # module/4/DCWH
                *uint32be(0),              # module/4/Tms
                0x8000,                    # module/4/Tmp
                0,                         # module/4/DCSt
                *uint32be(0),              # module/4/DCEvt
                5,                         # module/5/ID
                *string('StDisCha 5', 8),  # module/5/IDStr
                0,                         # module/5/DCA
                300,                       # module/5/DCV
                0,                         # module/5/DCW
                *uint32be(234567891),      # module/5/DCWH
                *uint32be(0),              # module/5/Tms
                0x8000,                    # module/5/Tmp
                0,                         # module/5/DCSt
                *uint32be(0),              # module/5/DCEvt
            ],
        )

        data.setValues(
            40363 + 1,
            [
                124,        # ID
                24,         # L
                0,          # WChaMax
                100,        # WChaGra
                0,          # WDisChaGra
                0,          # StorCtl_Mod [R/W]
                0,          # VAChaMax [R/W]
                0,          # MinRsvPct [R/W]
                4223,       # ChaState
                0xFFFF,     # StorAval
                0xFFFF,     # InBatV
                3,          # ChaSt
                10000,      # OutWRte
                10000,      # InWRte
                0xFFFF,     # InOutWRte_WinTms
                28800,      # InOutWRte_RvrtTms
                0xFFFF,     # InOutWRte_RmpTms
                0,          # ChaGriSet
                0,          # WChaMax_SF
                0,          # WChaDisChaGra_SF
                0x8000,     # VAChaMax_SF
                int16(-2),  # MinRsvPct_SF
                int16(-2),  # ChaState_SF
                0x8000,     # StorAval_SF
                0x8000,     # InBatV_SF
                int16(-2),  # InOutWRte_SF
            ],
        )

        data.setValues(
            40389 + 1,
            [
                0xFFFF,  # ID
                0,       # L
            ],
        )

    device_ctx = ModbusDeviceContext(hr=data)
    server_ctx = ModbusServerContext(devices=device_ctx, single=True)

    print(f'Using model {args.model}')
    print(f'Listening on port {args.port}')
    StartTcpServer(server_ctx, address=('', args.port))


if __name__ == '__main__':
    main()
