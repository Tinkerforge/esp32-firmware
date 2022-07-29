/* warp-charger
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C)      2021 Birger Schmidt <bs-warp@netgaroo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "enplus.h"
#ifdef GD_FLASH
#include "enplus_firmware.h"
//#include "enplus_firmware.1.0.1435h"  // RFID, 1 Ampere limit steps
#include "enplus_firmware.1.1.212.h"  // no RFID but climatization possible after charging completed, charging limits 8A/10A/13A/16A only
//#include "enplus_firmware.1.1.258.h"  // RFID, no climatization possible after charging completed, 1 Ampere limit steps
//#include "enplus_firmware.1.1.538.h"  // RFID, no climatization possible after charging completed, 1 Ampere limit steps
//#include "enplus_firmware.1.1.805.h"  // RFID, no climatization possible after charging completed, 1 Ampere limit steps
#include "enplus_firmware.1.1.812.h"  // RFID, no climatization possible after charging completed, 1 Ampere limit steps
#endif

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"
#include "modules.h"
#include "HardwareSerial.h"
#include "Time/TimeLib.h"

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern WebServer server;

extern API api;
extern bool firmware_update_allowed;

bool ready_for_next_chunk = false;
size_t MAXLENGTH;
byte flash_seq;
uint32_t last_flash = 0;

// Charging profile:
// 10A ESP> W (2021-06-06 11:05:10) [PRIV_COMM, 1859]: Tx(cmd_AD len:122) :  FA 03 00 00 AD 1D 70 00 00 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15 06 06 0B 05 0A 00 00 00 00 0A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01 00 00 00 00 00 00 00 00 00 00 CE 75
// 12A ESP> W (2021-06-03 18:37:19) [PRIV_COMM, 1859]: Tx(cmd_AD len:122) :  FA 03 00 00 AD 19 70 00 00 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15 06 03 12 25 14 00 00 00 00 0C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01 00 00 00 00 00 00 00 00 00 00 BF 11
// 11A ESP> W (2021-06-04 08:07:58) [PRIV_COMM, 1859]: Tx(cmd_AD len:122) :  FA 03 00 00 AD 39 70 00 00 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15 06 04 08 07 32 00 00 00 00 0B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01 00 00 00 00 00 00 00 00 00 00 3A 0C
//

// Commands
// First byte = command code, then payload bytes, no crc bytes
byte Init1[] = {0xAC, 0x11, 0x0B, 0x01, 0x00, 0x00};
byte Init2[] = {0xAC, 0x11, 0x09, 0x01, 0x00, 0x01};
byte Init3[] = {0xAC, 0x11, 0x0A, 0x01, 0x00, 0x00};
byte Init4[] = {0xAC, 0x11, 0x0C, 0x01, 0x00, 0x00};
byte Init5[] = {0xAA, 0x18, 0x3E, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
byte Init6[] = {0xAC, 0x11, 0x0D, 0x04, 0x00, 0xB8, 0x0B, 0x00, 0x00};
byte Init7[] = {0xAA, 0x18, 0x3F, 0x04, 0x00, 0x1E, 0x00, 0x00, 0x00};
byte Init8[] = {0xAA, 0x18, 0x25, 0x0E, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02};

byte Init9[] = {0xAA, 0x18, 0x12, 0x01, 0x00, 0x03, 0x7B, 0x89};
byte Init10[] = {0xAA, 0x18, 0x12, 0x01, 0x00, 0x03, 0x3B, 0x9C}; // is Init10 the same as Init12? Probably 0x3B, 0x9C accidently copied from crc

// ctrl_cmd set ack done, type:0
//[2019-01-01 03:36:46] cmd_AA [privCommCmdAACfgCtrl]!
//[2019-01-01 03:36:46] cfg ctrl  addr:18 size:1 set:1 gun_id:0 len:1
//[2019-01-01 03:36:46] cfg ctrl_ack start_addr:18 end_addr:19 now_addr:18 set:1 gun_id:0 len:1
//[2019-01-01 03:36:46] ctrl_cmd:18 setType:1 [cmdAACtrlSetReset]!
byte Init12[] = {0xAA, 0x18, 0x12, 0x01, 0x00, 0x03}; // this triggers 0x02 SN, Hardware, Version
//W (1970-01-01 00:08:47) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 03 05 00 18 12 01 00 03 FB F6
//W (1970-01-01 00:08:48) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 03 05 00 14 12 01 00 00 53 F1
//I (1970-01-01 00:08:48) [PRIV_COMM, 51]: ctrl_cmd set ack done, type:0
//W (1970-01-01 00:08:48) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 04 05 00 18 12 01 00 03 BA 10
//W (1970-01-01 00:08:48) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 04 05 00 14 12 01 00 00 12 17
//I (1970-01-01 00:08:48) [PRIV_COMM, 51]: ctrl_cmd set ack done, type:0
//W (1970-01-01 00:08:48) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 05 05 00 18 12 01 00 03 7B DC
//W (1970-01-01 00:08:49) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 05 05 00 14 12 01 00 00 D3 DB
//I (1970-01-01 00:08:49) [PRIV_COMM, 51]: ctrl_cmd set ack done, type:0

// cmdAACtrlcantestsetAck test cancom...111
byte Init11[] = {0xAA, 0x18, 0x2A, 0x00, 0x00};

byte Init13[] = {0xA2, 0x00}; // is this just an ack for 0x02?
//ack for 03  //byte Init14[] = {0xA3, 0x18, 0x02, 0x06, 0x00, 0x15, 0x06, 0x0A, 0x07, 0x08, 0x26};
byte TimeAck[] = {'c', 'a', 'y', 'm', 'd', 'h', 'm', 's', 0, 0, 0, 0};

// ctrl_cmd set start power mode done
byte Init15[] = {0xAA, 0x18, 0x09, 0x01, 0x00, 0x00};
//W (2021-04-11 18:36:27) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 40 05 00 18 09 01 00 00 F9 36
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 40 05 00 14 09 01 00 00 11 30
//I (2021-04-11 18:36:27) [PRIV_COMM, 279]: ctrl_cmd set start power mode done -> minpower: 3150080

//privCommCmdA7StartTransAck GD Firmware before 1.1.212?
byte StartChargingA6[] = {0xA6, 'W', 'A', 'R', 'P', ' ', 'c', 'h', 'a', 'r', 'g', 'e', 'r', ' ', 'f', 'o', 'r', ' ', 'E', 'N', '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0};
byte StopChargingA6[]  = {0xA6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '0', '0', '0', '0', '0', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40, 0, 0, 0, 0, 0, 0, 0, 0};

byte StartChargingA7[] = {0xA7, '0', '0', '0', '0', '0', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte StopChargingA7[]  = {0xA7, '0', '0', '0', '0', '0', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10, 0};

byte TransactionAck[] = {0xA9, 'W', 'A', 'R', 'P', ' ', 'c', 'h', 'a', 'r', 'g', 'e', 'r', ' ', 'f', 'o', 'r', ' ', 'E', 'N', '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//privCommCmdAFSmartCurrCtl
byte ChargingLimit1[] = {0xAF, 0, 'y', 'm', 'd', 'h', 'm', 's', 0x80, 0x51, 0x01, 0, 0x01, 0, 0, 0, 0, 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte ChargingLimit2[] = {0xAD, 0, 0x44, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x02, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'y', 'm', 'd', 'h', 'm', 's', 0, 0, 0, 0, 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x03, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte ChargingLimit3[] = {0xAD, 1, 0x91, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x02, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 'Y', 'm', 'd', 'h', 'm', 's', 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 'A', 0, 0, 0, 0x03, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//byte ChargingLimit1b[] = {0xAF, 0, 'y', 'm', 'd', 'h', 'm', 's', 0x80, 0x51, 0x01, 0, 0x02, 0, 0, 0, 0, 0x10, 0x01, 0, 0, 0, 'A', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//GD1.1.538  [2,"e3031518-5180-4c86-98ea-77fe478b09d1","SetChargingProfile",{"connectorId":1,"csChargingProfiles":{"chargingProfileKind":"Relative","chargingProfilePurpose": "TxDefaultProfile","chargingSchedule":{"startSchedule":"2022-01-10T20:29:10Z","chargingSchedulePeriod":[{"startPeriod":0,"limit":6.0,"numberPhases":3}],"chargingRateUnit":"A"},"chargingProfileId":2013,"stackLevel":0}}]
// Tx(cmd_AD len:122) : 0xAD,     0, 0xDD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x02, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, y, m, d, h, m, s, 0, 0, 0, 0, ChargeLimitA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x03, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// Rx(cmd_0D, len:13) : 0x0D,     0, 0, 0
// Rx(cmd_0F, len:41) : 0x0F,     0, y, m, d, h, m, s, 0x80, 0x51, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// Tx(cmd_AF, len:292) : 0xAF,    0, y, m, d, h, m, s, 0x80, 0x51, 0x01, 0, 0x02, 0, 0, 0, 0, 0x10, 0x01, 0, 0, 0, ChargeLimitA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

//privCommCmdADSmartchargeCtl
//byte ChargingLimit3[] = {0xAD, 0x01, 0x91, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x02, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0, 0, 0,  0, 0x79, 0x08, 0x16, 0x10, 0x04, 0x10, 0, 0, 0,  0, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0, 0, 0, 0x03, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//byte ChargingLimit4[] = {0xAD,    0, 0xDD,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x02, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 'y', 'm',  'd',  'h',  'm',  's',    0, 0, 0, 0, 'A',  0,    0,    0,    0,    0,  0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x03, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Enter boot mode: Tx(cmd_AB len: 20): FA 03 00 00 AB 14 0A 00 00 00 00 00 00 00 05 00 00 00 62 B2
// ESP> W (2021-10-04 10:04:35) [PRIV_COMM, 1875]: Tx(cmd_AB len:20) :  FA 03 00 00 AB 15 0A 00 00 00 00 00 00 00 05 00 00 00 60 33  ^M
// Enter boot mode acknowledge: Rx(cmd_0B len: 16): FA 03 00 00 0B 14 06 00 00 00 05 00 00 00 F1 3A
// erase flash
// ESP> W (2021-10-04 10:04:39) [PRIV_COMM, 1875]: Tx(cmd_AB len:20) :  FA 03 00 00 AB 17 0A 00 00 00 00 00 00 00 02 00 00 00 66 05  ^M
//byte EnterBootMode[] = {0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00};
byte RemoteUpdate[] = {0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00}; // #7 is the operation command [0-5]
// Exit boot mode, enter application mode: Tx (cmd_AB len: 24): FA 03 00 00 AB 5A 0E 00 00 00 03 08 FC FF 00 00 02 00 4F 4B 00 00 E1 98
byte EnterAppMode[] = {0xAB, 0x00, 0x00, 0x03, 0x08, 0xFC, 0xFF, 0x00, 0x00, 0x02, 0x00, 0x4F, 0x4B, 0x00, 0x00};
//Handshake: //Tx (cmd_AB len:20): FA 03 00 00 AB 15 0A 00 00 00 00 00 00 00 01 00 00 00 61 03
// ESP> W (2021-10-04 10:04:39) [PRIV_COMM, 1875]: Tx(cmd_AB len:20) :  FA 03 00 00 AB 16 0A 00 00 00 00 00 00 00 01 00 00 00 64 C0  ^M
byte Handshake[] =   {0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
//Flash verify:
//byte FlashVerify[] = {0xAB, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 40, 0x00, /* 40 words */
byte FlashVerify[811] = {0xAB, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 0x90, 0x01, /* 400 words */
     /*  0x00  */     0x68, 0x16, 0x00, 0x20, 0x1d, 0x25, 0x00, 0x08, 0x3b, 0x0e, 0x00, 0x08, 0x3d, 0x0e, 0x00, 0x08, /* example data */
     /*  0x10  */     0x41, 0x0e, 0x00, 0x08, 0x45, 0x0e, 0x00, 0x08, 0x49, 0x0e, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
     /*  0x20  */     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4d, 0x0e, 0x00, 0x08,
     /*  0x30  */     0x4f, 0x0e, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x51, 0x0e, 0x00, 0x08, 0x29, 0x0b, 0x00, 0x08,
     /*  0x40  */     0x33, 0x25, 0x00, 0x08, 0x33, 0x25, 0x00, 0x08, 0x33, 0x25, 0x00, 0x08, 0x33, 0x25, 0x00, 0x08  /* filled with zeros automatically */ };

/*

37977                 000: FA 03 00 00 AB 5C 5A 00 00 00 00 00 00 00 04 00 28 00 68 16
                      020: 00 20 1D 25 00 08 3B 0E 00 08 3D 0E 00 08 41 0E 00 08 45 0E
                      040: 00 08 49 0E 00 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                      060: 00 00 4D 0E 00 08 4F 0E 00 08 00 00 00 00 51 0E 00 08 29 0B
                      080: 00 08 33 25 00 08 33 25 00 08 33 25 00 08 33 25 00 08 E8 14
38006       Tx cmd_AB seq:5C, len:100, crc:14E8
38073                 000: FA 03 00 00 0B 5C 06 00 00 00 04 00 01 00 A7 55
38073       Rx cmd_0B seq:5C len:6 crc:55A7

*/

byte sendSequenceNumber = 1;
int transactionNumber = 100000;

/* experimental: JSON data sender */
#include <HTTPClient.h>
const char* JSON_DECODER = "";  // e.g. "http://xxx.xxx.xxx.xxx/test.php", should be defined via configuration settings
HTTPClient httpClient;  // for sending JSON data

void send_http (String jmessage) {
    if (JSON_DECODER[0] != '\0') {  // if target url present
        httpClient.begin(JSON_DECODER);
        httpClient.addHeader("Content-Type", "application/json");

        int httpResponseCode = httpClient.POST("{\"version\":1,\"device\":\"" + String((uint32_t)ESP.getEfuseMac(), HEX) + "\"" + jmessage);  // prepend chip ID and send data
        logger.printfln("JSON message sent. Response code: %d", httpResponseCode);
        //logger.printfln("Response: %s", http.getString().c_str());  // print response
        httpClient.end();  // disconnect
    }
}

/* experimental: Test command receiver */
#include <WiFiUdp.h>
#define UDP_RX_PACKET_MAX_SIZE 1024
WiFiUDP UdpListener;
unsigned int commandPort = 43211;  // local port to listen on
char receiveCommandBuffer[UDP_RX_PACKET_MAX_SIZE];  // buffer to hold incoming UDP data
/* end experimental */

uint16_t crc16_modbus(uint8_t *buffer, uint32_t length) {
        //uint16_t crc = 0xFFFF;
        uint16_t crc = 0x0;

        while (length--) {
                uint8_t tmp = *buffer++ ^ crc;
                crc >>= 8;
                crc ^= crc16_modbus_table[tmp];
        }

        return crc;
}

#define PRIVCOMM_MAGIC      0
#define PRIVCOMM_VERSION    1
#define PRIVCOMM_ADDR       2
#define PRIVCOMM_CMD        3
#define PRIVCOMM_SEQ        4
#define PRIVCOMM_LEN        5
#define PRIVCOMM_PAYLOAD    6
#define PRIVCOMM_CRC        7

#define PayloadStart        8

String ENplus::get_hex_privcomm_line(byte *data) {
    #define LOG_LEN 4048 //TODO work without such a big buffer by writing line by line
    char log[LOG_LEN] = {0};
    char *local_log = log;
    uint16_t len = (uint16_t)(data[7] << 8 | data[6]) + 10; // payload length + 10 bytes header and crc
    if (len > 1000) { // mqtt buffer is the limiting factor
        logger.printfln("ERROR: buffer (%d bytes) too big for mqtt buffer (max 1000).", len);
        len = 1000;
    }

    int offset = 0;
    for(uint16_t i = 0; i < len; i++) {
        #define BUFFER_CHUNKS 20
        if(i % BUFFER_CHUNKS == 0) {
            if(i>0) { local_log += snprintf(local_log, LOG_LEN - (local_log - log), "\r\n"); }
            if(i>=BUFFER_CHUNKS) { local_log += snprintf(local_log, LOG_LEN - (local_log - log), "            "); }
            local_log += snprintf(local_log, LOG_LEN - (local_log - log), "          %.3d: ", offset);
            offset = offset + BUFFER_CHUNKS;
        }
        local_log += snprintf(local_log, LOG_LEN - (local_log - log), "%.2X ", data[i]);
    }
    local_log += snprintf(local_log, LOG_LEN - (local_log - log), "\r\n");
    logger.write(log, local_log - log);
    return String(log);
}

void ENplus::Serial2write(byte *data, int size) {
    int bytes_to_send = size;
    int offset = 0;
    uint32_t start_time = millis();
    while(bytes_to_send > 0 || millis() - start_time >= 1000) {
        int afw = Serial2.availableForWrite();
        if(afw < bytes_to_send) { // send chunk
            bytes_to_send = bytes_to_send - Serial2.write(PrivCommTxBuffer + offset, afw);
            //logger.printfln("    send: Tx afw:%d bytes_to_send:%d offset:%d", afw, bytes_to_send, offset);
            offset = offset + afw;
            //logger.printfln("    could not send in one shot... delay(1250)");
            //delay(1250);
        } else { // send reminder
            bytes_to_send = bytes_to_send - Serial2.write(PrivCommTxBuffer + offset, bytes_to_send);
            //logger.printfln("    sEND: Tx afw:%d bytes_to_send:%d offset:%d", afw, bytes_to_send, offset);
        }
    }
    if(bytes_to_send > 0) {
        logger.printfln("ERR Tx time out, but still %d bytes_to_send, which where discarded now", bytes_to_send);
    }
}

char timeString[20];  // global since local variable could not be used as return value
const char* ENplus::timeStr(byte *data, uint8_t offset=0) {
    sprintf(timeString, "%04d/%02d/%02d %02d:%02d:%02d",
        data[offset++]+2000, data[offset++], data[offset++],
        data[offset++], data[offset++], data[offset]
    );
    return timeString;
}

void ENplus::sendCommand(byte *data, int datasize, byte sendSequenceNumber) {
    PrivCommTxBuffer[4] = data[0]; // command code
    PrivCommTxBuffer[5] = sendSequenceNumber;
    PrivCommTxBuffer[6] = (datasize-1) & 0xFF;
    PrivCommTxBuffer[7] = (datasize-1) >> 8;
    memcpy(PrivCommTxBuffer+8, data+1, datasize-1);

    uint16_t crc = crc16_modbus(PrivCommTxBuffer, datasize + 7);

    PrivCommTxBuffer[datasize+7] = crc & 0xFF;
    PrivCommTxBuffer[datasize+8] = crc >> 8;

    get_hex_privcomm_line(PrivCommTxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
    String cmdText = "";
    switch (PrivCommTxBuffer[4]) {
        case 0xA3: cmdText = "- Status data ack"; break;
        case 0xA4:
            cmdText = "- Heartbeat ack " + String(timeStr(PrivCommTxBuffer+9));
            break;
        case 0xA6: if (PrivCommTxBuffer[72] == 0x40) cmdText = "- Stop charging request"; else cmdText = "- Start charging request"; break;
        case 0xA7: if (PrivCommTxBuffer[40] == 0x10) cmdText = "- Stop charging command"; else cmdText = "- Start charging command"; break;
        case 0xA8: cmdText = "- Power data ack"; break;
        case 0xA9: cmdText = "- Transaction data ack"; break;
        case 0xAF: cmdText = "- Limit1: " + String(PrivCommTxBuffer[24]) + " Ampere"; break;
        case 0xAD: if (PrivCommTxBuffer[8] == 0) cmdText = "- Limit2: " + String(PrivCommTxBuffer[72]) + " Ampere"; else cmdText = "- Limit3: " + String(PrivCommTxBuffer[77]) + " Ampere"; break;
    }
    logger.printfln("Tx cmd_%.2X seq:%.2X len:%d crc:%.4X %s", PrivCommTxBuffer[4], PrivCommTxBuffer[5], datasize+9, crc, cmdText.c_str());

    Serial2write(data, datasize + 9);
    evse_privcomm.get("TX")->updateString(PrivCommHexBuffer);
}

void ENplus::PrivCommSend(byte cmd, uint16_t datasize, byte *data) {
    // the first 4 bytes never change and should be set already
    data[4] = cmd;
    //data[5] = sendSequence-1;
    data[6] = datasize & 0xFF;
    data[7] = datasize >> 8;
    //for (int i=1; i<=datasize; i++) {
    //    PrivCommTxBuffer[i+7] = payload[i];
    //}

    uint16_t crc = crc16_modbus(data, datasize + 8);

    data[datasize+8] = crc & 0xFF;
    data[datasize+9] = crc >> 8;

    get_hex_privcomm_line(data); // PrivCommHexBuffer now holds the hex representation of the buffer
    logger.printfln("Tx cmd_%.2X seq:%.2X, len:%d, crc:%.4X", cmd, data[5], datasize, crc);

    Serial2write(data, datasize + 10);
    evse_privcomm.get("TX")->updateString(PrivCommHexBuffer);
}

void ENplus::PrivCommAck(byte cmd, byte *data) {
    // the first 4 bytes never change and should be set already
    data[4] = cmd ^ 0xA0;  // complement command for ack
    data[6] = 1; //len
    //data[5]=seq; //should already be copied from RxBuffer
    data[7] = 0; //len
    data[8] = 0; //payload
    uint16_t crc = crc16_modbus(data, 9);
    data[9] = crc & 0xFF;
    data[10] = crc >> 8;

    get_hex_privcomm_line(data); // PrivCommHexBuffer now holds the hex representation of the buffer
    logger.printfln("Tx cmd_%.2X seq:%.2X, crc:%.4X", data[4], data[5], crc);

    Serial2write(data, 11);
    evse_privcomm.get("TX")->updateString(PrivCommHexBuffer);
}

void ENplus::sendTime(byte cmd, byte action, byte len, byte sendSequenceNumber) {
    TimeAck[0] = cmd;
    time_t t = now(); // get current time
    TimeAck[1] = action;
    TimeAck[2] = year(t) -2000;
    TimeAck[3] = month(t);
    TimeAck[4] = day(t);
    TimeAck[5] = hour(t);
    TimeAck[6] = minute(t);
    TimeAck[7] = second(t);
    // TimeAck[8] to TimeAck[11] are always 0
    sendCommand(TimeAck, len, sendSequenceNumber);
}

void ENplus::sendTimeLong(byte sendSequenceNumber) {
    PrivCommTxBuffer[5] = sendSequenceNumber;
    time_t t = now(); // get current time
    PrivCommTxBuffer[PayloadStart + 0] = 0x18;
    PrivCommTxBuffer[PayloadStart + 1] = 0x02;
    PrivCommTxBuffer[PayloadStart + 2] = 0x06;
    PrivCommTxBuffer[PayloadStart + 3] = 0x00;
    PrivCommTxBuffer[PayloadStart + 4] = year(t) -2000;
    PrivCommTxBuffer[PayloadStart + 5] = month(t);
    PrivCommTxBuffer[PayloadStart + 6] = day(t);
    PrivCommTxBuffer[PayloadStart + 7] = hour(t);
    PrivCommTxBuffer[PayloadStart + 8] = minute(t);
    PrivCommTxBuffer[PayloadStart + 9] = second(t);
    PrivCommSend(0xAA, 10, PrivCommTxBuffer);
}

void ENplus::sendChargingLimit1(time_t t, uint8_t currentLimit, byte sendSequenceNumber) {  // AF 00 date/time
    ChargingLimit1[2] = year(t) -2000;
    ChargingLimit1[3] = month(t);
    ChargingLimit1[4] = day(t);
    ChargingLimit1[5] = hour(t);
    ChargingLimit1[6] = minute(t);
    ChargingLimit1[7] = second(t);
    ChargingLimit1[17] = currentLimit;
    sendCommand(ChargingLimit1, sizeof(ChargingLimit1), sendSequenceNumber);
}

void ENplus::sendChargingLimit2(time_t t, uint8_t currentLimit, byte sendSequenceNumber) {  // AD 00
//    ChargingLimit2[2] = 8;  // charging profile ID - 0x41 for 1.0.1435 ?
    ChargingLimit2[55] = year(t) -2000;
    ChargingLimit2[56] = month(t);
    ChargingLimit2[57] = day(t);
    ChargingLimit2[58] = hour(t);
    ChargingLimit2[59] = minute(t);
    ChargingLimit2[60] = second(t);
    ChargingLimit2[65] = currentLimit;
    sendCommand(ChargingLimit2, sizeof(ChargingLimit2), sendSequenceNumber);  // triggers 0F charging limit1 request
}

void ENplus::sendChargingLimit3(time_t t, uint8_t currentLimit, byte sendSequenceNumber) {  //  AD 01 91
    ChargingLimit3[56] = year(t) -2000 +100;  // ?
    ChargingLimit3[57] = month(t);
    ChargingLimit3[58] = day(t);
    ChargingLimit3[59] = hour(t);
    ChargingLimit3[60] = minute(t);
    ChargingLimit3[61] = second(t);
    ChargingLimit3[70] = currentLimit;
    sendCommand(ChargingLimit3, sizeof(ChargingLimit3), sendSequenceNumber);  // triggers 0F charging limit1 request
}

ENplus::ENplus()
{
    evse_config = ConfigRoot{Config::Object({
        {"auto_start_charging", Config::Bool(true)},
        {"managed", Config::Bool(true)},
        {"max_current_configured", Config::Uint16(0)}
    })};

    evse_state = ConfigRoot{Config::Object({
        {"iec61851_state", Config::Uint8(0)},
        {"vehicle_state", Config::Uint8(0)},
        {"GD_state", Config::Uint8(0)},
        {"contactor_state", Config::Uint8(0)},
        {"contactor_error", Config::Uint8(0)},
        {"charge_release", Config::Uint8(0)},
        {"allowed_charging_current", Config::Uint16(0)},
        {"error_state", Config::Uint8(0)},
        {"lock_state", Config::Uint8(0)},
        {"time_since_state_change", Config::Uint32(0)},
        {"last_state_change", Config::Uint32(0)},
        {"uptime", Config::Uint32(0)}
    })};

    evse_privcomm = ConfigRoot{Config::Object({
        {"RX", Config::Str("", 0, 1000)},
        {"TX", Config::Str("", 0, 1000)}
    })};

    evse_hardware_configuration = ConfigRoot{Config::Object({
        {"Hardware", Config::Str("", 0, 20)},
        {"FirmwareVersion", Config::Str("", 0, 20)},
        {"SerialNumber", Config::Str("", 0, 20)},
        {"evse_found", Config::Bool(false)},
        {"initialized", Config::Bool(false)},
        {"GDFirmwareVersion", Config::Uint16(0)},
        {"jumper_configuration", Config::Uint8(3)}, // 3 = 16 Ampere = 11KW for the EN+ wallbox
        {"has_lock_switch", Config::Bool(false)}    // no key lock switch
    })};

    evse_low_level_state = ConfigRoot{Config::Object ({
        {"low_level_mode_enabled", Config::Bool(false)},
        {"led_state", Config::Uint8(0)},
        {"cp_pwm_duty_cycle", Config::Uint16(0)},
        {"adc_values", Config::Array({
                Config::Uint16(0),
                Config::Uint16(0),
            }, new Config{Config::Uint16(0)}, 2, 2, Config::type_id<Config::ConfUint>())
        },
        {"voltages", Config::Array({
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
            }, new Config{Config::Int16(0)}, 3, 3, Config::type_id<Config::ConfInt>())
        },
        {"resistances", Config::Array({
                Config::Uint32(0),
                Config::Uint32(0),
            }, new Config{Config::Uint32(0)}, 2, 2, Config::type_id<Config::ConfUint>())
        },
        {"gpio", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false),Config::Bool(false), Config::Bool(false)}, new Config{Config::Bool(false)}, 5, 5, Config::type_id<Config::ConfBool>())},
        {"hardware_version", Config::Uint8(0)},
        {"charging_time", Config::Uint32(0)},
    })};

    evse_max_charging_current = ConfigRoot{Config::Object ({
        {"max_current_configured", Config::Uint16(0)},
        {"max_current_incoming_cable", Config::Uint16(16000)},
        {"max_current_outgoing_cable", Config::Uint16(16000)},
        {"max_current_managed", Config::Uint16(0)},
    })};

    evse_auto_start_charging = ConfigRoot{Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    })};

    evse_auto_start_charging_update = ConfigRoot{Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    })};
    evse_current_limit = ConfigRoot{Config::Object({
        {"current", Config::Uint(32000, 0000, 32000)}
    })};

    evse_stop_charging = ConfigRoot{Config::Null()};
    evse_start_charging = ConfigRoot{Config::Null()};

    evse_managed_current = ConfigRoot{Config::Object ({
        {"current", Config::Uint16(0)}
    })};

    evse_managed = ConfigRoot{Config::Object({
        {"managed", Config::Bool(false)}
    })};

    evse_managed_update = ConfigRoot{Config::Object({
        {"managed", Config::Bool(false)},
        {"password", Config::Uint32(0)}
    })};

    evse_user_calibration = ConfigRoot{Config::Object({
        {"user_calibration_active", Config::Bool(false)},
        {"voltage_diff", Config::Int16(0)},
        {"voltage_mul", Config::Int16(0)},
        {"voltage_div", Config::Int16(0)},
        {"resistance_2700", Config::Int16(0)},
        {"resistance_880", Config::Array({
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
            }, new Config{Config::Int16(0)}, 14, 14, Config::type_id<Config::ConfInt>())},
    })};
}

int ENplus::bs_evse_start_charging() {
    uint8_t allowed_charging_current = uint8_t(evse_state.get("allowed_charging_current")->asUint()/1000);
    logger.printfln("EVSE start charging with max %d Ampere", allowed_charging_current);

    switch (evse_hardware_configuration.get("GDFirmwareVersion")->asUint()) {
        case 212:
            sendCommand(StartChargingA6, sizeof(StartChargingA6), sendSequenceNumber++);  // max current is switched to 16A when plugged out to ensure full current range available
            sendChargingLimit3(now(), allowed_charging_current, sendSequenceNumber++);  // reduce to intended value
            break;
        default:
            logger.printfln("Unknown firmware version. Trying commands for latest version.");
        case 258:
        case 538:
        case 805:
        case 812:
        case 1435:
            sendCommand(StartChargingA6, sizeof(StartChargingA6), sendSequenceNumber++);
            break;
    }
    return 0;
}

int ENplus::bs_evse_stop_charging() {
    logger.printfln("EVSE stop charging");
    sendCommand(StopChargingA6, sizeof(StopChargingA6), sendSequenceNumber++);
    return 0;
}

int ENplus::bs_evse_persist_config() {
    String error = api.callCommand("evse/config_update", Config::ConfUpdateObject{{
        {"auto_start_charging", evse_auto_start_charging.get("auto_start_charging")->asBool()},
        {"max_current_configured", evse_max_charging_current.get("max_current_configured")->asUint()},
        {"managed", evse_managed.get("managed")->asBool()}
    }});
    if (error != "") {
        logger.printfln("Failed to save config: %s", error.c_str());
        return 500;
    } else {
	logger.printfln("saved config - auto_start_charging: %s, managed: %s, max_current_configured: %d",
            evse_config.get("auto_start_charging")->asBool() ?"true":"false",
            evse_config.get("managed")->asBool() ?"true":"false",
            evse_config.get("max_current_configured")->asUint());
        return 0;
    }
}

int ENplus::bs_evse_set_charging_autostart(bool autostart) {
    logger.printfln("EVSE set auto start charging to %s", autostart ? "true" :"false");
    evse_auto_start_charging.get("auto_start_charging")->updateBool(autostart);
    bs_evse_persist_config();
    return 0;
}

int ENplus::bs_evse_set_max_charging_current(uint16_t max_current) {
    evse_max_charging_current.get("max_current_configured")->updateUint(max_current);
    bs_evse_persist_config();
    update_evse_state();
    uint8_t allowed_charging_current = evse_state.get("allowed_charging_current")->asUint()/1000;
    logger.printfln("EVSE set configured charging limit to %d Ampere", uint8_t(max_current/1000));
    logger.printfln("EVSE calculated allowed charging limit is %d Ampere", allowed_charging_current);
    switch (evse_hardware_configuration.get("GDFirmwareVersion")->asUint()) {
        case 212:
            sendChargingLimit3(now(), allowed_charging_current, sendSequenceNumber++);
            break;
        default:
            logger.printfln("Unknown firmware version. Trying commands for latest version.");
        case 258:
        case 538:
        case 805:
        case 812:
        case 1435:
            sendChargingLimit1(now(), allowed_charging_current, sendSequenceNumber++);
    }
    return 0;
}

int ENplus::bs_evse_get_state(uint8_t *ret_iec61851_state, uint8_t *ret_vehicle_state, uint8_t *ret_contactor_state, uint8_t *ret_contactor_error, uint8_t *ret_charge_release, uint16_t *ret_allowed_charging_current, uint8_t *ret_error_state, uint8_t *ret_lock_state, uint32_t *ret_time_since_state_change, uint32_t *ret_uptime) {
//    bool response_expected = true;
//    tf_tfp_prepare_send(evse->tfp, TF_EVSE_FUNCTION_GET_STATE, 0, 17, response_expected);
    uint32_t allowed_charging_current;

    *ret_iec61851_state = evse_state.get("iec61851_state")->asUint();
    *ret_vehicle_state = evse_state.get("iec61851_state")->asUint(); // == 1 ? // charging ? 2 : 1; // 1 verbunden 2 leadt
    *ret_contactor_state = 2;
    *ret_contactor_error = 0;
    *ret_charge_release = 1; // manuell 0 automatisch
    // find the charging current maximum
    allowed_charging_current = min(
        evse_max_charging_current.get("max_current_incoming_cable")->asUint(),
        evse_max_charging_current.get("max_current_outgoing_cable")->asUint());
    if(evse_managed.get("managed")->asBool()) {
        allowed_charging_current = min(
            allowed_charging_current,
            evse_max_charging_current.get("max_current_managed")->asUint());
    }
    *ret_allowed_charging_current = min(
        allowed_charging_current,
        evse_max_charging_current.get("max_current_configured")->asUint());
    *ret_error_state = 0;
    *ret_lock_state = 0;
    *ret_time_since_state_change = evse_state.get("time_since_state_change")->asUint();
    *ret_uptime = millis();

    return TF_E_OK;
}

void ENplus::setup()
{
    UdpListener.begin(commandPort); // experimental
    setup_evse();
    if(!api.restorePersistentConfig("evse/config", &evse_config)) {
        logger.printfln("EVSE error, could not restore persistent storage config");
    } else {
        evse_auto_start_charging.get("auto_start_charging")     -> updateBool(evse_config.get("auto_start_charging")->asBool());
        evse_max_charging_current.get("max_current_configured") -> updateUint(evse_config.get("max_current_configured")->asUint());
        evse_managed.get("managed")                             -> updateBool(evse_config.get("managed")->asBool());
	logger.printfln("restored config - auto_start_charging: %s, managed: %s, max_current_configured: %d",
            evse_config.get("auto_start_charging")->asBool() ?"true":"false",
            evse_config.get("managed")->asBool() ?"true":"false",
            evse_config.get("max_current_configured")->asUint());
    }

    task_scheduler.scheduleWithFixedDelay("update_evse_state", [this](){
        update_evse_state();
    }, 0, 1000);

    task_scheduler.scheduleWithFixedDelay("update_evse_low_level_state", [this](){
        update_evse_low_level_state();
    }, 0, 1000);

    task_scheduler.scheduleWithFixedDelay("update_evse_user_calibration", [this](){
        update_evse_user_calibration();
    }, 0, 10000);

    task_scheduler.scheduleWithFixedDelay("update_evse_charge_stats", [this](){
        update_evse_charge_stats();
    }, 0, 10000);

#ifdef MODULE_CM_NETWORKING_AVAILABLE
    cm_networking.register_client([this](uint16_t current){
        set_managed_current(current);
        //evse_managed.get("managed")->updateBool(true);
	logger.printfln("evse_managed: %s, current: %d", evse_managed.get("managed")->asBool() ?"true":"false", current);
    });

    task_scheduler.scheduleWithFixedDelay("evse_send_cm_networking_client", [this](){
        cm_networking.send_client_update(
            evse_state.get("iec61851_state")->asUint(),
            evse_state.get("vehicle_state")->asUint(),
            evse_state.get("error_state")->asUint(),
            evse_state.get("charge_release")->asUint(),
            evse_state.get("uptime")->asUint(),
            evse_low_level_state.get("charging_time")->asUint(),
            evse_state.get("allowed_charging_current")->asUint(),
            min(evse_max_charging_current.get("max_current_configured")->asUint(),
                min(evse_max_charging_current.get("max_current_incoming_cable")->asUint(),
                    evse_max_charging_current.get("max_current_outgoing_cable")->asUint())),
            evse_managed.get("managed")->asBool()
        );
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay("evse_managed_current_watchdog", [this]() {
        if (!deadline_elapsed(this->last_current_update + 30000))
            return;
        if(!this->shutdown_logged)
            logger.printfln("Got no managed current update for more than 30 seconds. Setting managed current to 0");
        this->shutdown_logged = true;
        evse_managed_current.get("current")->updateUint(0);
    }, 1000, 1000);
#endif
}

String ENplus::get_evse_debug_header() {
    return "millis,iec,vehicle,contactor,_error,charge_release,allowed_current,error,lock,t_state_change,uptime,low_level_mode_enabled,led,cp_pwm,adc_pe_cp,adc_pe_pp,voltage_pe_cp,voltage_pe_pp,voltage_pe_cp_max,resistance_pe_cp,resistance_pe_pp,gpio_in,gpio_out,gpio_motor_in,gpio_relay,gpio_motor_error\n";
}

String ENplus::get_evse_debug_line() {
    if(!initialized)
        return "EVSE is not initialized!";

    uint8_t iec61851_state, vehicle_state, contactor_state, contactor_error, charge_release, error_state, lock_state;
    uint16_t allowed_charging_current;
    uint32_t time_since_state_change, uptime;

    int rc = bs_evse_get_state(
        &iec61851_state,
        &vehicle_state,
        &contactor_state,
        &contactor_error,
        &charge_release,
        &allowed_charging_current,
        &error_state,
        &lock_state,
        &time_since_state_change,
        &uptime);

    if(rc != TF_E_OK) {
        return String("evse_get_state failed: rc: ") + String(rc);
    }

    bool low_level_mode_enabled;
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;

    uint16_t adc_values[2];
    int16_t voltages[3];
    uint32_t resistances[2];
    bool gpio[5];

//    rc = tf_evse_get_low_level_state(
//        &low_level_mode_enabled,
//        &led_state,
//        &cp_pwm_duty_cycle,
//        adc_values,
//        voltages,
//        resistances,
//        gpio);

    if(rc != TF_E_OK) {
        return String("evse_get_low_level_state failed: rc: ") + String(rc);
    }

    char line[150] = {0};
    snprintf(line, sizeof(line)/sizeof(line[0]), "%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%c,%u,%u,%u,%u,%d,%d,%d,%u,%u,%c,%c,%c,%c,%c\n",
        millis(),
        iec61851_state,
        vehicle_state,
        contactor_state,
        contactor_error,
        charge_release,
        allowed_charging_current,
        error_state,
        lock_state,
        time_since_state_change,
        uptime,
        low_level_mode_enabled ? '1' : '0',
        led_state,
        cp_pwm_duty_cycle,
        adc_values[0],adc_values[1],
        voltages[0],voltages[1],voltages[2],
        resistances[0],resistances[1],
        gpio[0] ? '1' : '0',gpio[1] ? '1' : '0',gpio[2] ? '1' : '0',gpio[3] ? '1' : '0',gpio[4] ? '1' : '0');

    return String(line);
}

void ENplus::set_managed_current(uint16_t current) {
    //is_in_bootloader(tf_evse_set_managed_current(current));
    evse_managed_current.get("current")->updateUint(current);
    evse_max_charging_current.get("max_current_managed")->updateUint(current);
    this->last_current_update = millis();
    this->shutdown_logged = false;
}

void ENplus::register_urls()
{
    api.addPersistentConfig("evse/config", &evse_config, {}, 1000);

    api.addState("evse/state", &evse_state, {}, 1000);
    api.addState("evse/hardware_configuration", &evse_hardware_configuration, {}, 1000);
    api.addState("evse/low_level_state", &evse_low_level_state, {}, 1000);
    api.addState("evse/max_charging_current", &evse_max_charging_current, {}, 1000);
    api.addState("evse/auto_start_charging", &evse_auto_start_charging, {}, 1000);
    api.addState("evse/privcomm", &evse_privcomm, {}, 1000);

    api.addCommand("evse/auto_start_charging_update", &evse_auto_start_charging_update, {}, [this](){
        bs_evse_set_charging_autostart(evse_auto_start_charging_update.get("auto_start_charging")->asBool());
    }, false);

    api.addCommand("evse/current_limit", &evse_current_limit, {}, [this](){
        bs_evse_set_max_charging_current(evse_current_limit.get("current")->asUint());
    }, false);

    api.addCommand("evse/stop_charging", &evse_stop_charging, {}, [this](){bs_evse_stop_charging();}, true);
    api.addCommand("evse/start_charging", &evse_start_charging, {}, [this](){bs_evse_start_charging();}, true);

    api.addCommand("evse/managed_current_update", &evse_managed_current, {}, [this](){
        this->set_managed_current(evse_managed_current.get("current")->asUint());
    }, true);

    api.addState("evse/managed", &evse_managed, {}, 1000);
    api.addCommand("evse/managed_update", &evse_managed_update, {"password"}, [this](){
        evse_managed.get("managed")->updateBool(evse_managed_update.get("managed")->asBool());
        bs_evse_persist_config();
    }, true);

    api.addState("evse/user_calibration", &evse_user_calibration, {}, 1000);
    api.addCommand("evse/user_calibration_update", &evse_user_calibration, {}, [this](){

    }, true);

#ifdef GD_FLASH
    server.on("/update_gd", HTTP_GET, [this](WebServerRequest request){
        //request.send(200, "text/html", "<form><input id=\"firmware\"type=\"file\"> <button id=\"u_firmware\"type=\"button\"onclick='u(\"firmware\")'>Flash GD Firmware</button> <label id=\"p_firmware\"></label><button id=\"u_verify\"type=\"button\"onclick='u(\"verify\")'>Verify GD Firmware</button> <label id=\"p_verify\"></label></form><script>function u(e){var t,n,d,o=document.getElementById(\"firmware\").files;0==o.length?alert(\"No file selected!\"):(document.getElementById(\"firmware\").disabled=!0,document.getElementById(\"u_firmware\").disabled=!0,document.getElementById(\"u_verify\").disabled=!0,t=o[0],n=new XMLHttpRequest,d=document.getElementById(\"p_\"+e),n.onreadystatechange=function(){4==n.readyState&&(200==n.status?(document.open(),document.write(n.responseText),document.close()):(0==n.status?alert(\"Server closed the connection abruptly!\"):alert(n.status+\" Error!\\n\"+n.responseText),location.reload()))},n.upload.addEventListener(\"progress\",function(e){e.lengthComputable&&(d.innerHTML=e.loaded/e.total*100+\"% (\"+e.loaded+\" / \"+e.total+\")\")},!1),n.open(\"POST\",\"/flash_\"+e,!0),n.send(t))}</script>");
        request.send(200, "text/html", "<form><input id=\"gd_firmware\"type=\"file\"> <button id=\"u_firmware\"type=\"button\"onclick='u(\"gd_firmware\")'>Upload GD Firmware</button> <label id=\"p_gd_firmware\"></label></form><form><input id=\"verify\"type=\"file\"> <button id=\"u_verify\"type=\"button\"onclick='u(\"verify\")'>Verify GD Firmware</button> <label id=\"p_verify\"></label></form><script>function u(e){var t,n,d,o=document.getElementById(e).files;0==o.length?alert(\"No file selected!\"):(document.getElementById(\"gd_firmware\").disabled=!0,document.getElementById(\"u_firmware\").disabled=!0,document.getElementById(\"verify\").disabled=!0,document.getElementById(\"u_verify\").disabled=!0,t=o[0],n=new XMLHttpRequest,d=document.getElementById(\"p_\"+e),n.onreadystatechange=function(){4==n.readyState&&(200==n.status?(document.open(),document.write(n.responseText),document.close()):(0==n.status?alert(\"Server closed the connection abruptly!\"):alert(n.status+\" Error!\\n\"+n.responseText),location.reload()))},n.upload.addEventListener(\"progress\",function(e){e.lengthComputable&&(d.innerHTML=e.loaded/e.total*100+\"% (\"+e.loaded+\" / \"+e.total+\")\")},!1),n.open(\"POST\",\"/flash_\"+e,!0),n.send(t))}</script>");
    });
    server.on("/flash_gd_firmware", HTTP_POST, [this](WebServerRequest request){
        if (update_aborted)
            return;
        this->firmware_update_running = false;
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            return;
        }
        /* request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK"); */
        request.send(200, "text/plain", "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            this->firmware_update_running = false;
            return false;
        }
        this->firmware_update_running = true;
        return handle_update_chunk1(3, request, index, data, len, final, request.contentLength());
    });

    server.on("/flash_verify", HTTP_POST, [this](WebServerRequest request){
        if (update_aborted)
            return;
        this->firmware_update_running = false;
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            return;
        }
        /* request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK"); */
        request.send(200, "text/plain", "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            this->firmware_update_running = false;
            return false;
        }
        this->firmware_update_running = true;
        return handle_update_chunk2(3, request, index, data, len, final, request.contentLength());
    });

    /* server.on("/flash_verify", HTTP_POST, [this](WebServerRequest request){ */
    /*     request.send(200, "text/plain", "Update OK"); */
    /* },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){ */
    /*     return handle_update_chunk(4, request, index, data, len, final, request.contentLength()); */
    /* }); */
#endif

#ifdef MODULE_WS_AVAILABLE
    server.on("/evse/start_debug", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.scheduleOnce("enable evse debug", [this](){
            ws.pushStateUpdate(this->get_evse_debug_header(), "evse/debug_header");
            debug = true;
        }, 0);
        request.send(200);
    });

    server.on("/evse/stop_debug", HTTP_GET, [this](WebServerRequest request){
        task_scheduler.scheduleOnce("enable evse debug", [this](){
            debug = false;
        }, 0);
        request.send(200);
    });
#endif
}

void ENplus::loop()
{
    static uint32_t last_check = 0;
    static uint32_t nextMillis = 2000;
    static uint8_t evseStatus = 0;
    static uint8_t cmd;
    static uint8_t seq;
    static uint16_t len;
    uint16_t crc;
    static bool cmd_to_process = false;
    static byte PrivCommRxState = PRIVCOMM_MAGIC;
    static int PrivCommRxBufferPointer = 0;
    byte rxByte;

    if(evse_found && !initialized && deadline_elapsed(last_check + 10000)) {
        last_check = millis();
        setup_evse();
    }

#ifdef MODULE_WS_AVAILABLE
    static uint32_t last_debug = 0;
    if(debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushStateUpdate(this->get_evse_debug_line(), "evse/debug");
    }
#endif

    if( Serial2.available() > 0 && !cmd_to_process) {
        do {
            rxByte = Serial2.read();
            PrivCommRxBuffer[PrivCommRxBufferPointer++] = rxByte;
            //Serial.print(rxByte, HEX);
            //Serial.print(" ");
            switch( PrivCommRxState ) {
                // Magic Header (0xFA) Version (0x03) Address (0x0000) CMD (0x??) Seq No. (0x??) Length (0x????) Payload (0-1015) Checksum (crc16)
                case PRIVCOMM_MAGIC:
                    PrivCommRxBufferPointer=1;
                    if(rxByte == 0xFA) {
                        PrivCommRxState = PRIVCOMM_VERSION;
                    } else {
                        logger.printfln("PRIVCOMM ERR: out of sync byte: %.2X", rxByte);
                    }
                    break;
                case PRIVCOMM_VERSION:
                    if(rxByte == 0x03) {
                        PrivCommRxState = PRIVCOMM_ADDR;
                    } else {
                        logger.printfln("PRIVCOMM ERR: got Rx Packet with wrong Version %.2X.", rxByte);
                        PrivCommRxState = PRIVCOMM_MAGIC;
                    }
                    break;
                case PRIVCOMM_ADDR:
                    if(rxByte == 0x00) {
                        if(PrivCommRxBufferPointer == 4) { // this was the second byte of the address, move on
                            PrivCommRxState = PRIVCOMM_CMD;
                        }
                    } else {
                        logger.printfln("PRIVCOMM ERR: got Rx Packet with wrong Address %.2X%.2X. PrivCommRxBufferPointer: %d", PrivCommRxBuffer[2], PrivCommRxBuffer[3], PrivCommRxBufferPointer);
                        PrivCommRxState = PRIVCOMM_MAGIC;
                    }
                    break;
                case PRIVCOMM_CMD:
                    PrivCommRxState = PRIVCOMM_SEQ;
                    cmd = rxByte;
                    break;
                case PRIVCOMM_SEQ:
                    PrivCommRxState = PRIVCOMM_LEN;
                    seq = rxByte;
                    PrivCommTxBuffer[5] = seq;
                    break;
                case PRIVCOMM_LEN:
                    if(PrivCommRxBufferPointer == 8) { // this was the second byte of the length, move on
                        PrivCommRxState = PRIVCOMM_PAYLOAD;
                        len = (uint16_t)(PrivCommRxBuffer[7] << 8 | PrivCommRxBuffer[6]);
                        //logger.printfln("PRIVCOMM INFO: len: %d cmd:%.2X", len, cmd);
                    }
                    break;
                case PRIVCOMM_PAYLOAD:
                    if(PrivCommRxBufferPointer == len + 8) {
                        //final byte of Payload received.
                        PrivCommRxState = PRIVCOMM_CRC;
                    } else {//if(cmd == 0x09) {
                        // this is an ugly hack to deal with non conforming 0x09 "upload log" packets
                        // cmd09 messeges are always too short and with no crc??? wtf?
                        // hopefully there are no more packets like this, or enough to recognize a pattern
                        if(PrivCommRxBuffer[PrivCommRxBufferPointer-4] == 0xFA &&
                           PrivCommRxBuffer[PrivCommRxBufferPointer-3] == 0x03 &&
                           PrivCommRxBuffer[PrivCommRxBufferPointer-2] == 0x00 &&
                           PrivCommRxBuffer[PrivCommRxBufferPointer-1] == 0x00) {
                            logger.printfln("PRIVCOMM BUG: process the next command albeit the last one was not finished. Buggy! cmd:%.2X len:%d cut off:%d", cmd, len, PrivCommRxBufferPointer-4);
                            PrivCommRxState = PRIVCOMM_CMD;
                            PrivCommRxBufferPointer = 4;
                            get_hex_privcomm_line(PrivCommRxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
                            evse_privcomm.get("RX")->updateString(PrivCommHexBuffer);
                            cmd_to_process = true;
                        }
                    }
                    break;
                case PRIVCOMM_CRC:
                    if(PrivCommRxBufferPointer == len + 10) {
            //Serial.println();
                        PrivCommRxState = PRIVCOMM_MAGIC;
                        get_hex_privcomm_line(PrivCommRxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
                        crc = (uint16_t)(PrivCommRxBuffer[len + 9] << 8 | PrivCommRxBuffer[len + 8]);
                        uint16_t checksum = crc16_modbus(PrivCommRxBuffer, len+8);
                        if(crc == checksum) {
                            if(!evse_found) {
                                logger.printfln("EN+ GD EVSE found. Enabling EVSE support.");
                                evse_found = true;
                                evse_hardware_configuration.get("evse_found")->updateBool(evse_found);
                            }
                        } else {
                            logger.printfln("CRC ERROR Rx cmd_%.2X seq:%.2X len:%d crc:%.4X checksum:%.4X", cmd, seq, len, crc, checksum);
                            break;
                        }
                        // log the whole packet?, but logger.printfln only writes 128 bytes / for now Serial.print on Serial2.read it is.
                        evse_privcomm.get("RX")->updateString(PrivCommHexBuffer);
                        cmd_to_process = true;
                    }
                    break;
            }  //switch read packet
        } while((Serial2.available() > 0) && !cmd_to_process && PrivCommRxBufferPointer<sizeof(PrivCommRxBuffer)/sizeof(PrivCommRxBuffer[0])); // one command at a time
    }

    char str[20];
    uint8_t allowed_charging_current = uint8_t(evse_state.get("allowed_charging_current")->asUint()/1000);
    time_t t=now();     // get current time

    if(cmd_to_process) {
        switch( cmd ) {
            case 0x02: // Info: Serial number, Version
//W (1970-01-01 00:08:52) [PRIV_COMM, 1919]: Rx(cmd_02 len:135) :  FA 03 00 00 02 26 7D 00 53 4E 31 30 30 35 32 31 30 31 31 39 33 35 37 30 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 24 D1 00 41 43 30 31 31 4B 2D 41 55 2D 32 35 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 31 2E 31 2E 32 37 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 00 00 00 00 00 5A 00 1E 00 00 00 00 00 00 00 00 00 D9 25
//W (1970-01-01 00:08:52) [PRIV_COMM, 1764]: Tx(cmd_A2 len:11) :  FA 03 00 00 A2 26 01 00 00 99 E0
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Serial number and version.", cmd, seq, len, crc);
                sprintf(str, "%s", PrivCommRxBuffer+8);
                evse_hardware_configuration.get("SerialNumber")->updateString(str);
                sprintf(str, "%s",PrivCommRxBuffer+43);
                evse_hardware_configuration.get("Hardware")->updateString(str);
                sprintf(str, "%s",PrivCommRxBuffer+91);
                evse_hardware_configuration.get("FirmwareVersion")->updateString(str);
                logger.printfln("EVSE serial: %s hw: %s fw: %s", 
                    evse_hardware_configuration.get("SerialNumber")->asString().c_str(),
                    evse_hardware_configuration.get("Hardware")->asString().c_str(),
                    evse_hardware_configuration.get("FirmwareVersion")->asString().c_str());
                if(!evse_hardware_configuration.get("initialized")->asBool()) {
                    initialized =
                        ((evse_hardware_configuration.get("Hardware")->asString().compareTo("AC011K-AU-25") == 0 || evse_hardware_configuration.get("Hardware")->asString().compareTo("AC011K-AE-25") == 0)
                         && (evse_hardware_configuration.get("FirmwareVersion")->asString().startsWith("1.1.", 0)  // known working: 1.1.27, 1.1.212, 1.1.258, 1.1.525, 1.1.538, 1.1.805, 1.1.812
                             && evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt() <= 812  // higest known working version (we assume earlier versions work as well)
                            )
                            || (evse_hardware_configuration.get("FirmwareVersion")->asString().startsWith("1.0.", 0)  // known working: 1.0.1435
                                && evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt() <= 1435  // higest known working version (we assume earlier versions work as well)
                            )
                        );
                    evse_hardware_configuration.get("initialized")->updateBool(initialized);
                    evse_hardware_configuration.get("GDFirmwareVersion")->updateUint(evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt());
                    if(initialized) {
                        logger.printfln("EN+ GD EVSE initialized.");
                    } else {
                        logger.printfln("EN+ GD EVSE Firmware Version or Hardware is not supported.");
                    }
                }
                PrivCommAck(cmd, PrivCommTxBuffer); // privCommCmdA2InfoSynAck
                break;

            case 0x03:
//W (1970-01-01 00:08:52) [PRIV_COMM, 1919]: Rx(cmd_03 len:24) :  FA 03 00 00 03 27 0E 00 00 09 09 0D 00 00 02 00 00 00 00 00 04 00 80 BC
//W (1970-01-01 00:08:52) [PRIV_COMM, 1764]: Tx(cmd_A3 len:17) :  FA 03 00 00 A3 27 07 00 00 E2 01 01 00 08 34 CF 2F
// 00 00 02 02 00 00 00 00 00 00 00 00 00
/* 853540                000: FA 03 00 00 03 0C 0E 00 50 02 02 05 00 00 00 00 00 00 00 00 */
/* 853540      Rx cmd_03 seq:0C len:14 crc:3DE8 */
/* (2021-10-04 10:04:39) [PRIV_COMM, 2033]: Rx(cmd_03 len:24) :  FA 03 00 00 03 03 0E 00 00 02 02 00 00 00 00 00 00 00 00 00 00 00 9C 52 */
/* (2021-10-04 10:04:40) [PRIV_COMM, 1875]: Tx(cmd_A3 len:17) :  FA 03 00 00 A3 03 07 00 10 15 0A 04 0A 04 27 4A D4 */
                evseStatus = PrivCommRxBuffer[9];
                if( PrivCommRxBuffer[8]==0x50 ) { // TODO this 0x50 is a wild guess, I've seen it work, and I'm sure there is more than just the evseStatus byte needed for a well founded decission
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Status %d: %s", cmd, seq, len, crc, evseStatus, evse_status_text[evseStatus]);
                    update_evseStatus(evseStatus);
                } else {
                    //TODO figure out what substatus (PrivCommRxBuffer[8]) is or should be
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Status %d: %s but substatus %.2X not 0x50.", cmd, seq, len, crc, evseStatus, evse_status_text[evseStatus], PrivCommRxBuffer[8]);
                    if(evse_hardware_configuration.get("initialized")->asBool()) { // only update the EVSE status if we support the hardware
                        update_evseStatus(evseStatus);
                    }
                }
                sendTime(0xA3, 0x10, 8, seq);  // send ack
                break;

            case 0x04: // time request / ESP32-GD32 communication heartbeat
//W (1970-01-01 00:08:52) [PRIV_COMM, 1919]: Rx(cmd_04 len:16) :  FA 03 00 00 04 28 06 00 09 00 00 00 00 00 C0 5E
//W (1970-01-01 00:08:52) [PRIV_COMM, 1764]: Tx(cmd_A4 len:17) :  FA 03 00 00 A4 28 07 00 00 E2 01 01 00 08 34 E5 6B
                evseStatus = PrivCommRxBuffer[8];
                update_evseStatus(evseStatus);
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Heartbeat request, Status %d: %s, value:%d", cmd, seq, len, crc, evseStatus, evse_status_text[evseStatus], PrivCommRxBuffer[12]);
// experimental:
                send_http(String(",\"type\":\"en+04\",\"data\":{")
                    +"\"status\":"+String(PrivCommRxBuffer[8])  // status
                    +",\"value\":"+String(PrivCommRxBuffer[12])
                    +"}}"
                );
// end experimental
                sendTime(0xA4, 0x01, 8, seq);  // send ack
                break;

            case 0x05:
//                                                                                        2  0  9  d  e  e  e  1                                                                          10             zeit
//W (2021-08-07 07:43:39) [PRIV_COMM, 1919]: Rx(cmd_05 len:57) :  FA 03 00 00 05 E3 2F 00 32 30 39 64 65 65 65 31 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 15 08 07 07 2B 26 01 00 00 00 F7 F7
//W (2021-08-07 07:43:39) [EN_WSS, 677]: send[0:41] [2,"87","Authorize",{"idTag":"209deee1"}]
//W (2021-08-07 07:43:39) [EN_WSS, 712]: recv[0:44] [3,"87",{"idTagInfo":{"status":"Accepted"}}]
//                                                                                                               // 40 = accept RFID charging
//                                                                                                                                                                                        40
//W (2021-08-07 07:43:39) [PRIV_COMM, 1764]: Tx(cmd_A5 len:47) :  FA 03 00 00 A5 19 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 D5 41
//
//"idTag":"50a674e1"
//"status":"Invalid"
//                                                                                        5  0  A  6  7  4  E  1                                                                          10             zeit
//W (2021-08-07 07:47:56) [PRIV_COMM, 1919]: Rx(cmd_05 len:57) :  FA 03 00 00 05 FA 2F 00 35 30 61 36 37 34 65 31 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 15 08 07 07 2F 37 01 00 00 00 2E C2
// D0 = reject RFID charging ?
//                                                                                                                                                                                        D0
//W (2021-08-07 07:47:56) [PRIV_COMM, 1764]: Tx(cmd_A5 len:47) :  FA 03 00 00 A5 1D 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D0 00 00 00 00 A4 86
//
		// der GD sendet nur ein cmd_05 wenn er in einem bestimmten mode ist?
		// vor dem cmd_05 sagt er dann: online_net_ok_start (das "create window fail" kommt aber immer)
//[2021-08-07 07:52:33]
//[m1] get_sn->5: 59 50 A6 74 E1
//[2021-08-07 07:52:33]
//[m1] auth_keyA->7: 06 59 FF FF FF FF FF
//[2021-08-07 07:52:33] [win] create window !!!
//[2021-08-07 07:52:33] [win] window had not init, create window fail
//[2021-08-07 07:52:33] online_net_ok_start
//                                                                  5  0  A  6  7  4  E  1                                                                          10             zeit
//[2021-08-07 07:52:33] Tx(cmd_05 len:57) : FA 03 00 00 05 06 2F 00 35 30 61 36 37 34 65 31 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 15 08 07 07 34 21 01 00 00 00 13 DC
// D0 = reject ?
//[2021-08-07 07:52:34] Rx(cmd_A5 len:47) : FA 03 00 00 A5 20 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D0 00 00 00 00 99 D0
//[2021-08-07 07:52:34] cmd_A5 [privCommCmdA5CardAuthAck]!
//[2021-08-07 07:52:34] cmdA5 countsta=1,leakmoneysta=3
//[2021-08-07 07:52:34] cmdA5 order_id=
//[2021-08-07 07:52:34] illegality_card
//
                // Command 05, payload 37 30 38 36 36 31 65 31 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                //Tx(cmd_A5 len:47) :  FA 03 00 00 A5 1D 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 D0 00 00 00 00 A4 86
		// privCommCmdA5CardAuthAck PrivCommTxBuffer+40 = 0x40; // allow charging
		// privCommCmdA5CardAuthAck PrivCommTxBuffer+40 = 0xD0; // decline charging

                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - RFID card detected. ID: %s", cmd, seq, len, crc, PrivCommRxBuffer + PayloadStart); //str);
                // Start/stop test with any RFID card:
                if (evseStatus == 2 || evseStatus == 6) sendCommand(StartChargingA6, sizeof(StartChargingA6), sendSequenceNumber++); 
                if (evseStatus == 3 || evseStatus == 4) sendCommand(StopChargingA6, sizeof(StopChargingA6), sendSequenceNumber++);
		break;

            case 0x06:
		// ack for cmd_A6 srvOcppRemoteStartTransactionReq (triggers cmd_07)
                //logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - cp call srv_ocpp ack srv remote ctrl ack", cmd, seq, len, crc);
//ESP> W (2021-08-22 16:04:37) [EN_WSS, 712]: recv[0:111] [2,"50d4459e-1d1e-42d9-b932-ee2f4784bd38","RemoteStartTransaction",{"connectorId":1,"idTag":"19800020490_APP"}]^M
//------ JSON RemoteStartTransaction --> idTag: 19800020490_APP
//ESP> D (2021-08-22 16:04:37) [MIDDLE_OPT, 285]: esp_mesh_is_root:  hw_opt_read,len:111^M
//ESP> D (2021-08-22 16:04:37) [OCPP_SRV, 3435]: chan[0] uniqId:50d4459e-1d1e-42d9-b932-ee2f4784bd38 actType:20^M
//ESP> D (2021-08-22 16:04:37) [OCPP_SRV, 3442]: srvCallfuns:20 [srvOcppRemoteStartTransactionReq]!^M
//ESP> D (2021-08-22 16:04:37) [OCPP_SRV, 2058]: chan[0] recv srv remote start transaction req info^M
//ESP> D (2021-08-22 16:04:37) [OCPP_SRV, 145]: srvUniqIdListPush add map:  srv:50d4459e-1d1e-42d9-b932-ee2f4784bd38 <==> cp:0x4C^M
//ESP> W (2021-08-22 16:04:37) [PRIV_COMM, 1779]: Tx(cmd_A6 len:83) :  FA 03 00 00 A6 4C 49 00 31 39 38 30 30 30 32 30 34 39 30 5F 41 50 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 00 00 00 00 00 00 00 00 D0 2A  ^M
//ESP> W (2021-08-22 16:04:38) [PRIV_COMM, 1934]: Rx(cmd_06 len:76) :  FA 03 00 00 06 4C 42 00 31 39 38 30 30 30 32 30 34 39 30 5F 41 50 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 30 00 78 37  ^M
//ESP> E (2021-08-22 16:04:38) [PRIV_COMM, 931]: gun_id:0 recv rmt ctrl start charging ack^M
//ESP> D (2021-08-22 16:04:38) [OCPP_SRV, 3648]: ocpp_sevice_ntc_evt: 6, chan:0,sts:8^M
//ESP> D (2021-08-22 16:04:38) [OCPP_SRV, 3121]: chan[0] cp call srv_ocpp ack srv remote ctrl ack^M
//ESP> D (2021-08-22 16:04:38) [OCPP_SRV, 3133]: uid:50d4459e-1d1e-42d9-b932-ee2f4784bd38 actType:20  seqNo:76^M
//ESP> ^M
//ESP> I (2021-08-22 16:04:38) [OCPP_SRV, 3140]: cp[SN10052106061308:1] rmt ctrl(3) succeed^M
//ESP> D (2021-08-22 16:04:38) [MIDDLE_OPT, 306]: esp_mesh_is_root:  hw_opt_send, chan:0^M
//ESP> W (2021-08-22 16:04:38) [EN_WSS, 677]: send[0:64] [3,"50d4459e-1d1e-42d9-b932-ee2f4784bd38",{"status":"Accepted"}]^M
                if (PrivCommRxBuffer[72] == 0x40) logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Stop charging request ack", cmd, seq, len, crc);
                else  logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Start charging request ack", cmd, seq, len, crc);
                break;

            case 0x07:
                //if (PrivCommRxBuffer[72] == 0x10) cmdText = "- Stop charging approval"; else cmdText = "- Start charging approval";
                if (PrivCommRxBuffer[72] == 0) {
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Start charging approval", cmd, seq, len, crc);
                    sendCommand(StartChargingA7, sizeof(StartChargingA7), seq);
// experimental:
                    send_http(String(",\"type\":\"en+07\",\"data\":{")
                      +"\"transaction\":"+String(transactionNumber)
                      +",\"meterStart\":"+String(PrivCommRxBuffer[79]+256*PrivCommRxBuffer[80])  // status ?
                      //+",\"startMode\":"+String(PrivCommRxBuffer[72])
                      +"}}"
                    );
// end experimental
                } else {
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Stop charging approval", cmd, seq, len, crc);
                    sendCommand(StopChargingA7, sizeof(StopChargingA7), seq);
                }
                break;

            case 0x08:
// W (2021-08-07 07:55:19) [PRIV_COMM, 1764]: Tx(cmd_A8 len:21) :  FA 03 00 00 A8 25 0B 00 40 15 08 07 07 37 13 00 00|[2021-08-07 07:55:18] Rx(cmd_A8 len:21) : FA 03 00 00 A8 25 0B 00 40 15 08 07 07 37 13 00 00 00 00 1B BE 00 00 1B BE                                                                                                      |[2021-08-07 07:55:18] cmd_A8 [privCommCmdA8RTDataAck]!
//D (2021-08-07 07:55:19) [OCPP_SRV, 3550]: ocpp_sevice_ntc_evt: 9, chan:0,sts:8                                    |[2021-08-07 07:55:18] charger A8 settime:21-8-7 7:55:19
//D (2021-08-07 07:55:19) [OCPP_SRV, 3031]: startMode(0:app 1:card 2:vin):1, stopreson:Remote timestamp:2021-08-07T0|[2021-08-07 07:55:19] [comm] cmd03 cpNowSts=0, gunNowSts=1,gunPreSts=0,chargerreson=6
//7:55:17Z idTag:50a674e1                                                                                           |[2021-08-07 07:55:19] Tx(cmd_03 len:24) : FA 03 00 00 03 18 0E 00 80 01 01 06 00 00 00 00 [2021-08-07 07:55:19] [
                if (PrivCommRxBuffer[77] < 10) {  // statistics
                    // TODO is it true that PrivCommRxBuffer[77] is the evseStatus?
                    evseStatus = PrivCommRxBuffer[77];
                    update_evseStatus(evseStatus);
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Status %d: %s", cmd, seq, len, crc, evseStatus, evse_status_text[evseStatus]);
                    logger.printfln("\t%dWh\t%d\t%dWh\t%d\t%d\t%d\t%dW\t%d\t%.1fV\t%.1fV\t%.1fV\t%.1fA\t%.1fA\t%.1fA\t%d minutes",
                              PrivCommRxBuffer[84]+256*PrivCommRxBuffer[85],  // charged energy Wh
                              PrivCommRxBuffer[86]+256*PrivCommRxBuffer[87],
                              PrivCommRxBuffer[88]+256*PrivCommRxBuffer[89],  // charged energy Wh
                              PrivCommRxBuffer[90]+256*PrivCommRxBuffer[91],
                              PrivCommRxBuffer[92]+256*PrivCommRxBuffer[93],
                              PrivCommRxBuffer[94]+256*PrivCommRxBuffer[95],
                              PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97],  // charging power
                              PrivCommRxBuffer[98]+256*PrivCommRxBuffer[99],
                              float(PrivCommRxBuffer[100]+256*PrivCommRxBuffer[101])/10,  // L1 plug voltage * 10
                              float(PrivCommRxBuffer[102]+256*PrivCommRxBuffer[103])/10,  // L2 plug voltage * 10
                              float(PrivCommRxBuffer[104]+256*PrivCommRxBuffer[105])/10,  // L3 plug voltage * 10
                              float(PrivCommRxBuffer[106]+256*PrivCommRxBuffer[107])/10,  // L1 charging current * 10
                              float(PrivCommRxBuffer[108]+256*PrivCommRxBuffer[109])/10,  // L2 charging current * 10
                              float(PrivCommRxBuffer[110]+256*PrivCommRxBuffer[111])/10,  // L3 charging current * 10
                              PrivCommRxBuffer[113]+256*PrivCommRxBuffer[114]
                              );
// experimental:
                    send_http(String(",\"type\":\"en+08\",\"data\":{")
                        +"\"status\":"+String(PrivCommRxBuffer[77])  // status
                        +",\"transaction\":"+String(transactionNumber)
                        +",\"energy1\":"+String(PrivCommRxBuffer[84]+256*PrivCommRxBuffer[85])  // charged energy Wh
                        +",\"aaa\":"+String(PrivCommRxBuffer[86]+256*PrivCommRxBuffer[87])
                        +",\"energy2\":"+String(PrivCommRxBuffer[88]+256*PrivCommRxBuffer[89])  // charged energy Wh
                        +",\"bbb\":"+String(PrivCommRxBuffer[90]+256*PrivCommRxBuffer[91])
                        +",\"ccc\":"+String(PrivCommRxBuffer[92]+256*PrivCommRxBuffer[93])
                        +",\"ddd\":"+String(PrivCommRxBuffer[94]+256*PrivCommRxBuffer[95])
                        +",\"power\":"+String(PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97])  // charging power
                        +",\"eee\":"+String(PrivCommRxBuffer[98]+256*PrivCommRxBuffer[99])
                        +",\"u1\":"+String(float(PrivCommRxBuffer[100]+256*PrivCommRxBuffer[101])/10,1)  // L1 plug voltage * 10
                        +",\"u2\":"+String(float(PrivCommRxBuffer[102]+256*PrivCommRxBuffer[103])/10,1)  // L2 plug voltage * 10
                        +",\"u3\":"+String(float(PrivCommRxBuffer[104]+256*PrivCommRxBuffer[105])/10,1)  // L3 plug voltage * 10
                        +",\"i1\":"+String(float(PrivCommRxBuffer[106]+256*PrivCommRxBuffer[107])/10,1)  // L1 charging current * 10
                        +",\"i2\":"+String(float(PrivCommRxBuffer[108]+256*PrivCommRxBuffer[109])/10,1)  // L2 charging current * 10
                        +",\"i3\":"+String(float(PrivCommRxBuffer[110]+256*PrivCommRxBuffer[111])/10,1)  // L3 charging current * 10
                        +",\"minutes\":"+String(PrivCommRxBuffer[113]+256*PrivCommRxBuffer[114])
                        +"}}"
                    );
// end experimental
                } else {
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - type:%.2X", cmd, seq, len, crc, PrivCommRxBuffer[77]);
                    if (PrivCommRxBuffer[77] == 0x10) {  // RFID card
                        String rfid = "";
                        for (int i=0; i<8; i++) {rfid += PrivCommRxBuffer[40 + i];}  // Card number in bytes 40..47
                        logger.printfln("RFID Card %s", rfid);
                    }
                }
                sendTime(0xA8, 0x40, 12, seq); // send ack
                break;

            case 0x09:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Charging stop reason: %d - %s",
                    cmd, seq, len, crc,
                    PrivCommRxBuffer[77], stop_reason_text[PrivCommRxBuffer[77]]);  // "stopreson": 1 = Remote, 3 = EVDisconnected
                logger.printfln("start:%s stop:%s meter:%dWh value1:%d value2:%d value3:%d",
                    timeStr(PrivCommRxBuffer+80),
                    timeStr(PrivCommRxBuffer+86),
                    PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97],
                    PrivCommRxBuffer[78]+256*PrivCommRxBuffer[79],
                    PrivCommRxBuffer[92]+256*PrivCommRxBuffer[93],
                    PrivCommRxBuffer[94]+256*PrivCommRxBuffer[95]);
// experimental:
                send_http(String(",\"type\":\"en+09\",\"data\":{")
                    +"\"transaction\":"+String(transactionNumber)
                    +",\"meterStop\":"+String(PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97])  // status
                    +",\"stopReason\":"+String(PrivCommRxBuffer[77])
                    +"}}"
                );
                //PrivCommAck(cmd, PrivCommTxBuffer); // privCommCmdA9RecordAck
                sendCommand(TransactionAck, sizeof(TransactionAck), seq);
// end experimental
                break;

            case 0x0A:
                switch( PrivCommRxBuffer[9] ) { // 8: always 14, 9: answertype?
                    case 0x02: // answer to set time
//W (2021-04-11 18:36:27) [PRIV_COMM, 1764]: Tx(cmd_AA len:20) :  FA 03 00 00 AA 09 0A 00 18 02 06 00 15 04 0B 12 24 1B 5C 78
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:20) :  FA 03 00 00 0A 09 0A 00 14 02 06 00 15 04 0B 12 24 1B 3C E7
//I (2021-04-11 18:36:27) [PRIV_COMM, 94]: ctrl_cmd set time done -> time: 2021-04-11 18:36:27
    // ctrl_cmd set start power mode done
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Set Time done", cmd, seq, len, crc);
                        break;
                    case 0x08: // answer to set hb timeout
//W (1970-01-01 00:08:53) [PRIV_COMM, 1764]: Tx(cmd_AA len:16) :  FA 03 00 00 AA 07 06 00 18 08 02 00 1E 00 95 80
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:16) :  FA 03 00 00 0A 07 06 00 14 08 02 00 1E 00 93 CE
//I (2021-04-11 18:36:27) [PRIV_COMM, 249]: ctrl_cmd set heart beat time out done -> 30      (=1E)
//
// [2021-08-07 07:55:18] Rx(cmd_A8 len:21) : FA 03 00 00 A8 25 0B 00 40 15 08 07 07 37 13 00 00 00 00 1B BE
// [2021-08-07 07:55:18] cmd_A8 [privCommCmdA8RTDataAck]!
// [2021-08-07 07:55:18] charger A8 settime:21-8-7 7:55:19
//
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Heartbeat Timeout:%ds", cmd, seq, len, crc, PrivCommRxBuffer[12]);
                        break;
                    case 0x09: // answer to ctrl_cmd set start power mode
//W (2021-04-11 18:36:27) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 40 05 00 18 09 01 00 00 F9 36
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 40 05 00 14 09 01 00 00 11 30
//I (2021-04-11 18:36:27) [PRIV_COMM, 279]: ctrl_cmd set start power mode done -> minpower:  3.150.080

//W (2021-04-11 18:36:30) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 42 05 00 18 09 01 00 00 78 EF
//W (2021-04-11 18:36:31) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 42 05 00 14 09 01 00 00 90 E9
//I (2021-04-11 18:36:31) [PRIV_COMM, 279]: ctrl_cmd set start power mode done -> minpower: 15.306.752
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - ctrl_cmd set start power mode done", cmd, seq, len, crc);
                        break;
                    case 0x12: // ctrl_cmd set ack done, type:0
//W (1970-01-01 00:08:53) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 08 05 00 18 12 01 00 03 BA 45
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 08 05 00 14 12 01 00 00 12 42
//I (2021-04-11 18:36:27) [PRIV_COMM, 51]: ctrl_cmd set ack done, type:0
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - ctrl_cmd set ack done, type:0", cmd, seq, len, crc);
                        break;
                    case 0x2A: // answer to cmdAACtrlcantestsetAck test cancom...111
//W (1970-01-01 00:00:03) [PRIV_COMM, 1764]: Tx(cmd_AA len:14) :  FA 03 00 00 AA 02 04 00 18 2A 00 00 DB 76
//W (1970-01-01 00:00:03) [PRIV_COMM, 1919]: Rx(cmd_0A len:14) :  FA 03 00 00 0A 02 04 00 14 2A 00 00 D2 5E
//E (1970-01-01 00:00:03) [PRIV_COMM, 78]: cmdAACtrlcantestsetAck test cancom...111
    // cmdAACtrlcantestsetAck test cancom...111
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - cmdAACtrlcantestsetAck test cancom...111 done", cmd, seq, len, crc);
                        break;
                    default:
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X -  I don't know what %.2X means.", cmd, seq, len, crc, PrivCommRxBuffer[9]);
                        break;
                }  //switch cmdAA answer processing
                break;

            case 0x0B:
                if( PrivCommRxBuffer[12]==0 ) { // success
                    switch( PrivCommRxBuffer[10] ) {
                        case 5: // reset into boot mode
                            if(this->firmware_update_running) {
                                logger.printfln("   reset into boot mode complete, handshake next");
                                //sendCommand(Handshake, sizeof(Handshake), sendSequenceNumber++);
                                RemoteUpdate[7] = 1; // handshake
                                sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++);
                            } else {
                                logger.printfln("   reset into app mode complete");
                            }
                            break;
                        case 1: // handshake
                            if(FlashVerify[7] == 3) { // flash write
                                logger.printfln("   handshake complete, flash erase next");
                                RemoteUpdate[7] = 2; // flash erase
                                sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++);
                            } else if(FlashVerify[7] == 4) { // flash verify
                                logger.printfln("   handshake complete, flash verify next");
                                ready_for_next_chunk = true;
                            }
                            break;
                        case 2: // flash erase
                            logger.printfln("   flash erase complete, flash write next");
                            ready_for_next_chunk = true;
                            break;
                        case 3: // flash write
                            logger.printfln("   flash write fine");
                            ready_for_next_chunk = true;
                            break;
                        case 4: // verify
                            logger.printfln("   verify fine");
                            ready_for_next_chunk = true;
                            break;
                        default:
                            logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - privCommCmdABUpdateReq: %.2X%.2X%.2X%.2X  %d (%s) - %s", cmd, seq, len, crc, FlashVerify[3], FlashVerify[4], FlashVerify[5], FlashVerify[6], PrivCommRxBuffer[10], cmd_0B_text[PrivCommRxBuffer[10]], PrivCommRxBuffer[12]==0 ?"success":"failure");
                            logger.printfln("   getting the GD chip back into app mode");
                            sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);
                            break;
                    }  //switch privCommCmdABUpdateReq success
                } else {
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - privCommCmdABUpdateReq: %.2X%.2X%.2X%.2X  %d (%s) - %s", cmd, seq, len, crc, FlashVerify[3], FlashVerify[4], FlashVerify[5], FlashVerify[6], PrivCommRxBuffer[10], cmd_0B_text[PrivCommRxBuffer[10]], PrivCommRxBuffer[12]==0 ?"success":"failure");
                    logger.printfln("   getting the GD chip back into app mode");
                    update_aborted = true;
                    sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);
                }
                break;

            case 0x0D:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Limit ack", cmd, seq, len, crc);
                break;

            case 0x0E:
// [2021-08-07 07:55:05] Tx(cmd_0E len:76) : FA 03 00 00 0E 11 42 00 00 00 00 00 00 00 00 00 00 0A 01 77 02 37 35 32 30 33 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 66 08 97 08 14 00 7A 01 01 00 00 00 00 00 00 00 00 00 00 DE 91
// [2021-08-07 07:55:05] cmd0E_DutyData pwmMax:266
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Duty data duty:%d cpVolt:%d",
                    cmd, seq, len, crc,
                    PrivCommRxBuffer[17]+256*PrivCommRxBuffer[18],
                    PrivCommRxBuffer[19]+256*PrivCommRxBuffer[20]);
                logger.printfln("power factors:%d/%d/%d/%d offset0:%d offset1:%d leakcurr:%d AMBTemp:%d lock:%d",
                    PrivCommRxBuffer[9]+256*PrivCommRxBuffer[10],PrivCommRxBuffer[11]+256*PrivCommRxBuffer[12],PrivCommRxBuffer[13]+256*PrivCommRxBuffer[14],PrivCommRxBuffer[15]+256*PrivCommRxBuffer[16],
                    PrivCommRxBuffer[55]+256*PrivCommRxBuffer[56],
                    PrivCommRxBuffer[57]+256*PrivCommRxBuffer[58],
                    PrivCommRxBuffer[59]+256*PrivCommRxBuffer[60],
                    PrivCommRxBuffer[61]+256*PrivCommRxBuffer[62],
                    PrivCommRxBuffer[63]);
                //PrivCommAck(cmd, PrivCommTxBuffer); // Ack?
// experimental:
                send_http(String(",\"type\":\"en+0E\",\"data\":{")
                    +"\"transaction\":"+String(transactionNumber)
                    +",\"duty\":"+String(PrivCommRxBuffer[17]+256*PrivCommRxBuffer[18])
                    +",\"cpVolt\":"+String(PrivCommRxBuffer[19]+256*PrivCommRxBuffer[20])
                    +",\"pf1\":"+String(PrivCommRxBuffer[9]+256*PrivCommRxBuffer[10])  // power factor 1
                    +",\"pf2\":"+String(PrivCommRxBuffer[11]+256*PrivCommRxBuffer[12])  // power factor 2
                    +",\"pf3\":"+String(PrivCommRxBuffer[13]+256*PrivCommRxBuffer[14])  // power factor 3
                    +",\"pft\":"+String(PrivCommRxBuffer[15]+256*PrivCommRxBuffer[16])  // power factor total
                    +",\"offset0\":"+String(PrivCommRxBuffer[55]+256*PrivCommRxBuffer[56])
                    +",\"offset1\":"+String(PrivCommRxBuffer[57]+256*PrivCommRxBuffer[58])
                    +",\"leakcurr\":"+String(PrivCommRxBuffer[59]+256*PrivCommRxBuffer[60])
                    +",\"AMBTemp\":"+String(PrivCommRxBuffer[61]+256*PrivCommRxBuffer[62])
                    +",\"lock\":"+String(PrivCommRxBuffer[63])
                    +"}}"
                );
// end experimental
                break;

            case 0x0F:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Charging limit request", cmd, seq, len, crc);
                sendChargingLimit1(now(), allowed_charging_current, seq);
                break;

            default:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - I don't know what to do about it.", cmd, seq, len, crc);
                break;
        }  //switch process cmd
        cmd_to_process = false;
    }

    evse_state.get("time_since_state_change")->updateUint(millis() - evse_state.get("last_state_change")->asUint());
    
    //resend flash commands if needed
    if(this->firmware_update_running && flash_seq == PrivCommTxBuffer[5] && !ready_for_next_chunk && deadline_elapsed(last_flash + 2000)) {
        last_flash = millis();
        logger.printfln("resend the last chunk fseq: %d, seq: %d rfnc: %s", flash_seq, PrivCommTxBuffer[5], ready_for_next_chunk?"true":"false");
        sendCommand(FlashVerify, MAXLENGTH+11, sendSequenceNumber++); // next chunk (11 bytes header) 
    }


// experimental: UDP command receiver for testing
    int packetSize = UdpListener.parsePacket();
    if (packetSize) {
        logger.printfln("Received UDP packet of size %d from %s:%d.", packetSize, UdpListener.remoteIP().toString().c_str(), UdpListener.remotePort());
        // read the packet into packetBufffer
        UdpListener.read(receiveCommandBuffer, UDP_RX_PACKET_MAX_SIZE);
        receiveCommandBuffer[packetSize] = '\0';
        logger.printfln("Content: %s", receiveCommandBuffer);
        switch (receiveCommandBuffer[0]) {
            case 'L':
                uint8_t limit;
                if (receiveCommandBuffer[2] >= 'A') limit = receiveCommandBuffer[2]+10-'A';  // e.g. L2F for 15 Ampere;
                else limit = receiveCommandBuffer[2]-'0';  // e.g. L29 for 9A

                if (receiveCommandBuffer[1] == '1') sendChargingLimit1(now(), limit, sendSequenceNumber++);  // working for all except for 1.1.212
                else if (receiveCommandBuffer[1] == '2') sendChargingLimit2(now(), limit, sendSequenceNumber++);
                else if (receiveCommandBuffer[1] == '3') sendChargingLimit3(now(), limit, sendSequenceNumber++);  // working for 1.1.212

                if (receiveCommandBuffer[1] == 'P') {
                    ChargingLimit3[74] = 1;
                    sendChargingLimit3(now(), limit, sendSequenceNumber++);  // e.g. LPA for 1 phase, 10 Ampere. Sadly not working here :-( 
                    ChargingLimit3[74] = 3;
                }
                allowed_charging_current = limit;
                break;
            case 'C':
                if (receiveCommandBuffer[1] == '6') {
                    if (receiveCommandBuffer[2] == '0') sendCommand(StopChargingA6, sizeof(StopChargingA6), sendSequenceNumber++);  // stop charging handshake
                    else if (receiveCommandBuffer[2] == '1') sendCommand(StartChargingA6, sizeof(StartChargingA6), sendSequenceNumber++);  // start charging handshake
                } else if (receiveCommandBuffer[1] == '7') {
                    if (receiveCommandBuffer[2] == '0') sendCommand(StopChargingA7, sizeof(StopChargingA7), sendSequenceNumber++);  // stop charging directly
                    else if (receiveCommandBuffer[2] == '1') sendCommand(StartChargingA7, sizeof(StartChargingA7), sendSequenceNumber++);  // start charging directly
                }
                break;
            case 'O':
                // output as comment
                break;
            case 'R':
                RemoteUpdate[7] = 5; // Reset GD into boot mode
                //sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++);
                sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);  // Restart GD
                break;
            case 'V':
                sendCommand(Init12, sizeof(Init12), sendSequenceNumber++);  // triggers 0x02 reply: SN, Hardware, Version
        }
    }
// end experimental
}

void ENplus::setup_evse()
{
    Serial2.setRxBufferSize(1024);
    Serial2.begin(115200, SERIAL_8N1, 26, 27); // PrivComm to EVSE GD32 Chip
    //Serial2.setTimeout(90);
    logger.printfln("Set up PrivComm: 115200, SERIAL_8N1, RX 26, TX 27");

    // TODO start: look out for this on a unconfigured box ( no wifi ) - if it still works, delete the code
    setTime(23,59,00,31,12,2018);
    switch (timeStatus()){
        case timeNotSet:
            logger.printfln("the time has never been set, the clock started on Jan 1, 1970");
            break;
        case timeNeedsSync:
            logger.printfln("the time had been set but a sync attempt did not succeed");
            break;
        case timeSet:
            logger.printfln("the time is set and is synced");
            break;
    }
    logger.printfln("the time is %d", now());
    logger.printfln("the now() call was not blocking");
    // TODO end: look out for this on a unconfigured box ( no wifi ) - if it still works, delete the code


// V3.2.589 init sequence
 /* W (1970-01-01 00:00:00) [PRIV_COMM, 1875]: Tx(cmd_AC len:15) :  FA 03 00 00  AC  01   05 00    11 0B 01 00 00 CA D3 */
 /* W (1970-01-01 00:00:00) [PRIV_COMM, 1875]: Tx(cmd_AC len:15) :  FA 03 00 00  AC  02   05 00    11 09 01 00 01 4A BE */
 /* W (1970-01-01 00:00:00) [PRIV_COMM, 1875]: Tx(cmd_AC len:15) :  FA 03 00 00  AC  03   05 00    11 0A 01 00 00 4A F6 */
 /* W (1970-01-01 00:00:00) [PRIV_COMM, 1875]: Tx(cmd_AC len:15) :  FA 03 00 00  AC  04   05 00    11 0C 01 00 00 0B 98 */
 /* W (1970-01-01 00:00:00) [PRIV_COMM, 1875]: Tx(cmd_AA len:18) :  FA 03 00 00  AA  05   08 00    18 3E 04 00 00 00 00 00 54 F0 */
 /* W (1970-01-01 00:00:00) [PRIV_COMM, 1875]: Tx(cmd_AC len:18) :  FA 03 00 00  AC  06   08 00    11 0D 04 00 B8 0B 00 00 C5 B7 */
 /* W (1970-01-01 00:00:01) [PRIV_COMM, 1875]: Tx(cmd_AA len:18) :  FA 03 00 00  AA  07   08 00    18 3F 04 00 1E 00 00 00 49 A0 */
 /* W (1970-01-01 00:00:01) [PRIV_COMM, 1875]: Tx(cmd_AA len:28) :  FA 03 00 00  AA  08   12 00    18 25 0E 00 05 00 00 00 05 00 00 00 00 03 00 00 00 02 EC 31 */
 /* W (1970-01-01 00:00:01) [PRIV_COMM, 1875]: Tx(cmd_AA len:15) :  FA 03 00 00  AA  09   05 00    18 12 01 00 03 7B 89 */
 /* W (1970-01-01 00:00:03) [PRIV_COMM, 1875]: Tx(cmd_AA len:14) :  FA 03 00 00  AA  0A   04 00    18 2A 00 00 52 B6 */

    sendCommand(Init1,  sizeof(Init1), sendSequenceNumber++);
    sendCommand(Init2,  sizeof(Init2), sendSequenceNumber++);
    sendCommand(Init3,  sizeof(Init3), sendSequenceNumber++);
    sendCommand(Init4,  sizeof(Init4), sendSequenceNumber++);
    sendCommand(Init5,  sizeof(Init5), sendSequenceNumber++);
    sendCommand(Init6,  sizeof(Init6), sendSequenceNumber++);
    sendCommand(Init7,  sizeof(Init7), sendSequenceNumber++);
    sendCommand(Init8,  sizeof(Init8), sendSequenceNumber++);
    sendCommand(Init9,  sizeof(Init9), sendSequenceNumber++);
    sendCommand(Init10, sizeof(Init10), sendSequenceNumber++);  // last two bytes correct?

//W (1970-01-01 00:08:53) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 08 05 00 18 12 01 00 03 BA 45
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 08 05 00 14 12 01 00 00 12 42
//I (2021-04-11 18:36:27) [PRIV_COMM, 51]: ctrl_cmd set ack done, type:0

    // ctrl_cmd set ack done, type:0 // this triggers 0x02 SN, Hardware, Version
    sendCommand(Init12, sizeof(Init12), sendSequenceNumber++);


//W (1970-01-01 00:08:53) [PRIV_COMM, 1764]: Tx(cmd_AA len:16) :  FA 03 00 00 AA 07 06 00 18 08 02 00 1E 00 95 80
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:16) :  FA 03 00 00 0A 07 06 00 14 08 02 00 1E 00 93 CE
//I (2021-04-11 18:36:27) [PRIV_COMM, 249]: ctrl_cmd set heart beat time out done -> 30      (=1E)

    // ctrl_cmd set heart beat time out
    PrivCommTxBuffer[PayloadStart + 0] = 0x18;
    PrivCommTxBuffer[PayloadStart + 1] = 0x08;
    PrivCommTxBuffer[PayloadStart + 2] = 0x02;
    PrivCommTxBuffer[PayloadStart + 3] = 0x00;
    PrivCommTxBuffer[PayloadStart + 4] =  120; // 120 sec hb timeout
    PrivCommTxBuffer[PayloadStart + 5] = 0x00; // hb timeout 16bit?
    PrivCommSend(0xAA, 6, PrivCommTxBuffer);


//W (2021-04-11 18:36:27) [PRIV_COMM, 1764]: Tx(cmd_AA len:20) :  FA 03 00 00 AA 09 0A 00 18 02 06 00 15 04 0B 12 24 1B 5C 78
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:20) :  FA 03 00 00 0A 09 0A 00 14 02 06 00 15 04 0B 12 24 1B 3C E7
//I (2021-04-11 18:36:27) [PRIV_COMM, 94]: ctrl_cmd set time done -> time: 2021-04-11 18:36:27

    sendTimeLong(sendSequenceNumber++);


//W (2021-04-11 18:36:27) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 40 05 00 18 09 01 00 00 F9 36
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 40 05 00 14 09 01 00 00 11 30
//I (2021-04-11 18:36:27) [PRIV_COMM, 279]: ctrl_cmd set start power mode done -> minpower:  3.150.080

//W (2021-04-11 18:36:30) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 42 05 00 18 09 01 00 00 78 EF
//W (2021-04-11 18:36:31) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 42 05 00 14 09 01 00 00 90 E9
//I (2021-04-11 18:36:31) [PRIV_COMM, 279]: ctrl_cmd set start power mode done -> minpower: 15.306.752

    // ctrl_cmd set start power mode done
    sendCommand(Init15, sizeof(Init15), sendSequenceNumber++);


//W (1970-01-01 00:00:03) [PRIV_COMM, 1764]: Tx(cmd_AA len:14) :  FA 03 00 00 AA 02 04 00 18 2A 00 00 DB 76
//W (1970-01-01 00:00:03) [PRIV_COMM, 1919]: Rx(cmd_0A len:14) :  FA 03 00 00 0A 02 04 00 14 2A 00 00 D2 5E
//E (1970-01-01 00:00:03) [PRIV_COMM, 78]: cmdAACtrlcantestsetAck test cancom...111

    // cmdAACtrlcantestsetAck test cancom...111
    sendCommand(Init11, sizeof(Init11), sendSequenceNumber++);

/*
    do { // wait for the first PRIVCOMM signal to decide if we have a GD chip to talk to
        logger.printfln("wait for PrivComm");
        if (Serial2.available() == 0) { delay(250); }
    } while(Serial2.available() == 0 && millis()<10000); // TODO disable EVSE in case of no show
*/


    //int result = tf_evse_create(&evse, uid, &hal);
//    if(result != TF_E_OK) {
//        logger.printfln("Failed to initialize EVSE bricklet. Disabling EVSE support.");
//        return;
//    }

        /* task_scheduler.scheduleOnce("reboot_GD", [this](){ */
        /*     if(!initialized) { */
        /*         logger.printfln("   try to reset GD chip, cycle boot mode, app mode"); */
        /*         RemoteUpdate[7] = 5; // Reset into boot mode */
        /*         sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++); */
        /*         sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++); */
        /*     } */
        /* }, 5000); */

    initialized = true;
}

void ENplus::update_evse_low_level_state() {
    if(!initialized)
        return;

    bool low_level_mode_enabled;
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;

    uint16_t adc_values[2];
    int16_t voltages[3];
    uint32_t resistances[2];
    bool gpio[5];

//    int rc = tf_evse_get_low_level_state(
//        &low_level_mode_enabled,
//        &led_state,
//        &cp_pwm_duty_cycle,
//        adc_values,
//        voltages,
//        resistances,
//        gpio);

        low_level_mode_enabled = true;
        led_state = 1;
        cp_pwm_duty_cycle = 100;
        adc_values[0] = 200;
        adc_values[1] = 201;
        voltages[0] = 300;
        voltages[1] = 301;
        voltages[2] = 302;
        resistances[0] = 400;
        resistances[1] = 401;
        gpio[0] = false;
        gpio[1] = false;
        gpio[2] = false;
        gpio[3] = false;
        gpio[4] = false;

    evse_low_level_state.get("low_level_mode_enabled")->updateBool(low_level_mode_enabled);
    evse_low_level_state.get("led_state")->updateUint(led_state);
    evse_low_level_state.get("cp_pwm_duty_cycle")->updateUint(cp_pwm_duty_cycle);

    for(int i = 0; i < sizeof(adc_values)/sizeof(adc_values[0]); ++i)
        evse_low_level_state.get("adc_values")->get(i)->updateUint(adc_values[i]);

    for(int i = 0; i < sizeof(voltages)/sizeof(voltages[0]); ++i)
        evse_low_level_state.get("voltages")->get(i)->updateInt(voltages[i]);

    for(int i = 0; i < sizeof(resistances)/sizeof(resistances[0]); ++i)
        evse_low_level_state.get("resistances")->get(i)->updateUint(resistances[i]);

    for(int i = 0; i < sizeof(gpio)/sizeof(gpio[0]); ++i)
        evse_low_level_state.get("gpio")->get(i)->updateBool(gpio[i]);
}

void ENplus::update_evse_state() {
    if(!initialized)
        return;
    uint8_t iec61851_state, vehicle_state, contactor_state, contactor_error, charge_release, error_state, lock_state;
    uint16_t last_allowed_charging_current = evse_state.get("allowed_charging_current")->asUint();
    uint16_t allowed_charging_current;
    uint32_t time_since_state_change, uptime;

    int rc = bs_evse_get_state(
        &iec61851_state,
        &vehicle_state,
        &contactor_state,
        &contactor_error,
        &charge_release,
        &allowed_charging_current,
        &error_state,
        &lock_state,
        &time_since_state_change,
        &uptime);

    firmware_update_allowed = vehicle_state == 0;

    evse_state.get("iec61851_state")->updateUint(iec61851_state);
    evse_state.get("vehicle_state")->updateUint(vehicle_state);
//logger.printfln("EVSE: vehicle_state %d", vehicle_state);
    evse_state.get("contactor_state")->updateUint(contactor_state);
    bool contactor_error_changed = evse_state.get("contactor_error")->updateUint(contactor_error);
    evse_state.get("charge_release")->updateUint(charge_release);
    if(last_allowed_charging_current != allowed_charging_current) {
        evse_state.get("allowed_charging_current")->updateUint(allowed_charging_current);
        logger.printfln("EVSE: allowed_charging_current %d", allowed_charging_current);
        //bs_evse_set_max_charging_current(allowed_charging_current);  // Uwe: not needed here since requested by GD during start charging sequence
        logger.printfln("---->   bs_evse_set_max_charging_current function call dropped!");
    }
    bool error_state_changed = evse_state.get("error_state")->updateUint(error_state);
    evse_state.get("lock_state")->updateUint(lock_state);
    //evse_state.get("time_since_state_change")->updateUint(time_since_state_change);
    evse_state.get("uptime")->updateUint(uptime);
}

void ENplus::update_evse_charge_stats() {
    if(!initialized)
        return;

    // trigger status updates from the GD, process them in the regular loop
    if(evse_state.get("iec61851_state")->asUint() == 2) { // if charging
        // TODO this do not seem to work :-( and would be strange if it would
        //PrivCommAck(0x02, PrivCommTxBuffer); // privCommCmdA2InfoSynAck  A2 request status, triggers 03 and 08 answers
        //sendCommand(Init13, sizeof(Init13), sendSequenceNumber++); // as well A2
        // TODO try this
        //sendRequest0E(sendSequence++);  // send A8 40 time 00 00 00 00: trigger 0E answer if charging
    }
}

void ENplus::update_evseStatus(uint8_t evseStatus) {
    uint8_t last_iec61851_state = evse_state.get("iec61851_state")->asUint();
    uint8_t last_evseStatus = evse_state.get("GD_state")->asUint();
    evse_state.get("GD_state")->updateUint(evseStatus);
    switch (evseStatus) {
        case 1:                                              // Available (not engaged)
            evse_state.get("iec61851_state")->updateUint(0); // Nicht verbunden (Sicht des Fahrzeugs)
            break;
        case 2:                                              // Preparing (engaged, not started)
            evse_state.get("iec61851_state")->updateUint(1); // Verbunden
            break;
        case 3:                                              // Charging (charging ongoing, power output)
            evse_state.get("iec61851_state")->updateUint(2); // Lädt
            break;
        case 4:                                              // Suspended by charger (started but no power available)
            evse_state.get("iec61851_state")->updateUint(1); // Verbunden
            break;
        case 5:                                              // Suspended by EV (power available but waiting for the EV response)
            evse_state.get("iec61851_state")->updateUint(1); // Verbunden
            break;
        case 6:                                              // Finishing, charging acomplished (RFID stop or EMS control stop)
            evse_state.get("iec61851_state")->updateUint(1); // Verbunden
            break;
        case 7:                                              // (Reserved)
            evse_state.get("iec61851_state")->updateUint(4);
            break;
        case 8:                                              // (Unavailable)
            evse_state.get("iec61851_state")->updateUint(4);
            break;
        case 9:                                              // Fault (charger in fault condition)
            evse_state.get("iec61851_state")->updateUint(4);
            break;
        default:
            logger.printfln("err: can not determine EVSE status %d", evseStatus);
            break;
    }
    if(last_iec61851_state != evse_state.get("iec61851_state")->asUint()) {
        evse_state.get("last_state_change")->updateUint(millis());
        evse_state.get("time_since_state_change")->updateUint(millis() - evse_state.get("last_state_change")->asUint());
	if((evseStatus != last_evseStatus == 1) && (evseStatus == 1)) { // plugged out
            if (evse_hardware_configuration.get("GDFirmwareVersion")->asUint() == 212)
                //sendChargingLimit2(now(), 16, sendSequenceNumber++);  // hack to ensure full current range is available in next charging session 
                sendChargingLimit3(now(), 16, sendSequenceNumber++);  // hack to ensure full current range is available in next charging session
        }
	if(evseStatus == 2 && last_evseStatus == 1) { // just plugged in
            transactionNumber++;
            char buffer[13];
            sprintf(buffer, "%06d", transactionNumber);
            for (int i=0; i<6; i++) {  // patch transaction number into command templates
                StartChargingA7[i+1] = byte(buffer[i]);
                StopChargingA7[i+1] = byte(buffer[i]);
                StopChargingA6[i+33] = byte(buffer[i]);
            }
            logger.printfln("New transaction number %05d", transactionNumber);
        }
	if(evse_auto_start_charging.get("auto_start_charging")->asBool()
           && evseStatus == 2 || (evseStatus == 6 && last_evseStatus == 0)) { // just plugged in or already plugged in at startup
            logger.printfln("Start charging automatically");
            bs_evse_start_charging();
        }
    }
    evse_state.get("vehicle_state")->updateUint(evse_state.get("iec61851_state")->asUint());
}

void ENplus::update_evse_user_calibration() {
    if(!initialized)
        return;

    bool user_calibration_active;
    int16_t voltage_diff, voltage_mul, voltage_div, resistance_2700, resistance_880[14];

//    int rc = tf_evse_get_user_calibration(
//        &user_calibration_active,
//        &voltage_diff,
//        &voltage_mul,
//        &voltage_div,
//        &resistance_2700,
//        resistance_880);
//
//    evse_user_calibration.get("user_calibration_active")->updateBool(user_calibration_active);
//    evse_user_calibration.get("voltage_diff")->updateInt(voltage_diff);
//    evse_user_calibration.get("voltage_mul")->updateInt(voltage_mul);
//    evse_user_calibration.get("voltage_div")->updateInt(voltage_div);
//    evse_user_calibration.get("resistance_2700")->updateInt(resistance_2700);
//
//    for(int i = 0; i < sizeof(resistance_880)/sizeof(resistance_880[0]); ++i)
//        evse_user_calibration.get("resistance_880")->get(i)->updateInt(resistance_880[i]);
}

bool ENplus::is_in_bootloader(int rc) {
    return false;
}


#ifdef GD_FLASH
/* GD Firmware updater */

bool ENplus::handle_update_chunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length) {

    if(chunk_index == 0) {
 /* [PRIV_COMM, 1875]: Tx(cmd_AB len:820) :  FA 03 00 00 AB 18 2A 03 00 00 00 08 00 00 03 00 90 01 68 16 00 20 1D 25 00 08 3B 0E 00 08 3D 0E 00 08 41 0E 00 08 45 0E 00 08 49 0E 00 08 00 00 00 00 00 00 */
        //sendCommand(EnterBootMode, sizeof(EnterBootMode), sendSequenceNumber++);
        logger.printfln("EVSE RemoteUpdate, reset into boot mode");
        RemoteUpdate[7] = 5; // Reset into boot mode
        sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++);
        /* logger.printfln("Failed to start update: %s", Update.errorString()); */
        /* request.send(400, "text/plain", Update.errorString()); */
        /* update_aborted = true; */
        /* return true; */
    }

    size_t chunk_offset = 0;
    size_t length = chunk_length;

    FlashVerify[7] = command; // flash write (3=write, 4=verify)

    while (length > 0) {
        while (!ready_for_next_chunk) {
            loop(); //TODO make this more elegant
        }

        //calculate maxlength
        size_t maxlength = MIN(length, length % 800); // 800 bytes is the max flash verify/write size
        maxlength = maxlength > 0 ? maxlength : 800;  // process the reminder first, then 800b chunks
        FlashVerify[9]  = (maxlength/2 & 0x000000FF); // number of words to process (therefore divided by 2)
        FlashVerify[10] = (maxlength/2 & 0x0000FF00) >> 8;

        //calculate address
        uint32_t gd_address = chunk_index + chunk_offset + 0x8000000; // 0x8000000 is the start address for the GD chip
        FlashVerify[5] = (gd_address & 0x000000FF);
        FlashVerify[6] = (gd_address & 0x0000FF00) >> 8;
        FlashVerify[3] = (gd_address & 0x00FF0000) >> 16;
        FlashVerify[4] = (gd_address & 0xFF000000) >> 24;

        //logger.printfln("Processing update chunk with: chunk_index %.6X (%d), gd(%.2x %.2x %.2x %.2x) chunk_l %d, chunk_offset %d, complete_l %d, final: %s", chunk_index, chunk_index, FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], chunk_length, chunk_offset, complete_length, final?"true":"false");
        logger.printfln("c_index %d, gd(%.2x %.2x %.2x %.2x) chunk_l %d, chunk_offset %d, l %d, ml %d, ll %d, final: %s", chunk_index, FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], chunk_length, chunk_offset, length, maxlength, complete_length, final?"true":"false");

        if (update_aborted)
            return true;

        // copy data
        memcpy(FlashVerify+11, data + chunk_offset, maxlength);

        MAXLENGTH = maxlength;
        sendCommand(FlashVerify, maxlength+11, sendSequenceNumber++); // next chunk (11 bytes header) 
        flash_seq = PrivCommTxBuffer[5];
        last_flash = millis();
        ready_for_next_chunk = false;

        chunk_offset = chunk_offset + maxlength;
        length = length - maxlength;
    } // iterate through big chunks

    if(final) {
        this->firmware_update_running = false;
        logger.printfln("   scheduling GD chip app mode in 3s");
        // after last chunk, get out of flash mode
        task_scheduler.scheduleOnce("factory_reset", [this](){
            logger.printfln("   getting the GD chip back into app mode");
            sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);
        }, 3000);
    }

    return true;
}

bool ENplus::handle_update_chunk1(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length) {

    if(chunk_index == 0) {
 /* [PRIV_COMM, 1875]: Tx(cmd_AB len:820) :  FA 03 00 00 AB 18 2A 03 00 00 00 08 00 00 03 00 90 01 68 16 00 20 1D 25 00 08 3B 0E 00 08 3D 0E 00 08 41 0E 00 08 45 0E 00 08 49 0E 00 08 00 00 00 00 00 00 */
        //sendCommand(EnterBootMode, sizeof(EnterBootMode), sendSequenceNumber++);
        logger.printfln("EVSE RemoteUpdate, reset into boot mode");
        RemoteUpdate[7] = 5; // Reset into boot mode
        sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++);
        /* logger.printfln("Failed to start update: %s", Update.errorString()); */
        /* request.send(400, "text/plain", Update.errorString()); */
        /* update_aborted = true; */
        /* return true; */

        size_t chunk_offset = 0 + 0x8000;
        size_t length = gd_firmware_len - 0x8000;

        FlashVerify[7] = command; // flash write (3=write, 4=verify)

        while (length > 0) {
            while (!ready_for_next_chunk) {
                loop(); //TODO make this more elegant
            }

            //calculate maxlength
            //size_t maxlength = 800;               // 800 byte chunks
            size_t maxlength = 512;               // 512 byte chunks
            //if (length < 800) maxlength = length; // reminder
            if (length < 512) maxlength = length; // reminder
            FlashVerify[9]  = (maxlength/2 & 0x000000FF); // number of words to process (therefore divided by 2)
            FlashVerify[10] = (maxlength/2 & 0x0000FF00) >> 8;

            //calculate address
            uint32_t gd_address = chunk_index + chunk_offset + 0x8000000; // 0x8000000 is the start address for the GD chip
            FlashVerify[5] = (gd_address & 0x000000FF);
            FlashVerify[6] = (gd_address & 0x0000FF00) >> 8;
            FlashVerify[3] = (gd_address & 0x00FF0000) >> 16;
            FlashVerify[4] = (gd_address & 0xFF000000) >> 24;

            //logger.printfln("Processing update chunk with: chunk_index %.6X (%d), gd(%.2x %.2x %.2x %.2x) chunk_l %d, chunk_offset %d, complete_l %d, final: %s", chunk_index, chunk_index, FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], chunk_length, chunk_offset, complete_length, final?"true":"false");
            logger.printfln("gd(%.2x %.2x %.2x %.2x) binhex(%.2x%.2x) chunk_offset %d, l %d, ml %d, ll %d, final: %s", FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], FlashVerify[6],FlashVerify[5], chunk_offset, length, maxlength, complete_length, final?"true":"false");

            if (update_aborted)
                return true;

            // copy data
            memcpy(FlashVerify+11, gd_firmware_1_1_212 + chunk_offset, maxlength);  // firmware file for upload button

            MAXLENGTH = maxlength;
            sendCommand(FlashVerify, maxlength+11, sendSequenceNumber++); // next chunk (11 bytes header) 
            flash_seq = PrivCommTxBuffer[5];
            last_flash = millis();
            ready_for_next_chunk = false;

            chunk_offset = chunk_offset + maxlength;
            length = length - maxlength;
        } // iterate through big chunks
    } // first chunk

    if(final) {
        this->firmware_update_running = false;
        logger.printfln("   scheduling GD chip app mode in 3s");
        // after last chunk, get out of flash mode
        task_scheduler.scheduleOnce("factory_reset", [this](){
            logger.printfln("   getting the GD chip back into app mode");
            sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);
        }, 3000);
    }

    return true;
}

bool ENplus::handle_update_chunk2(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length) {

    if(chunk_index == 0) {
 /* [PRIV_COMM, 1875]: Tx(cmd_AB len:820) :  FA 03 00 00 AB 18 2A 03 00 00 00 08 00 00 03 00 90 01 68 16 00 20 1D 25 00 08 3B 0E 00 08 3D 0E 00 08 41 0E 00 08 45 0E 00 08 49 0E 00 08 00 00 00 00 00 00 */
        //sendCommand(EnterBootMode, sizeof(EnterBootMode), sendSequenceNumber++);
        logger.printfln("EVSE RemoteUpdate, reset into boot mode");
        RemoteUpdate[7] = 5; // Reset into boot mode
        sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++);
        /* logger.printfln("Failed to start update: %s", Update.errorString()); */
        /* request.send(400, "text/plain", Update.errorString()); */
        /* update_aborted = true; */
        /* return true; */

        size_t chunk_offset = 0 + 0x8000;
        size_t length = gd_firmware_len - 0x8000;

        FlashVerify[7] = command; // flash write (3=write, 4=verify)

        while (length > 0) {
            while (!ready_for_next_chunk) {
                loop(); //TODO make this more elegant
            }

            //calculate maxlength
            //size_t maxlength = 800;               // 800 byte chunks
            size_t maxlength = 512;               // 512 byte chunks
            //if (length < 800) maxlength = length; // reminder
            if (length < 512) maxlength = length; // reminder
            FlashVerify[9]  = (maxlength/2 & 0x000000FF); // number of words to process (therefore divided by 2)
            FlashVerify[10] = (maxlength/2 & 0x0000FF00) >> 8;

            //calculate address
            uint32_t gd_address = chunk_index + chunk_offset + 0x8000000; // 0x8000000 is the start address for the GD chip
            FlashVerify[5] = (gd_address & 0x000000FF);
            FlashVerify[6] = (gd_address & 0x0000FF00) >> 8;
            FlashVerify[3] = (gd_address & 0x00FF0000) >> 16;
            FlashVerify[4] = (gd_address & 0xFF000000) >> 24;

            //logger.printfln("Processing update chunk with: chunk_index %.6X (%d), gd(%.2x %.2x %.2x %.2x) chunk_l %d, chunk_offset %d, complete_l %d, final: %s", chunk_index, chunk_index, FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], chunk_length, chunk_offset, complete_length, final?"true":"false");
            logger.printfln("gd(%.2x %.2x %.2x %.2x) binhex(%.2x%.2x) chunk_offset %d, l %d, ml %d, ll %d, final: %s", FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], FlashVerify[6],FlashVerify[5], chunk_offset, length, maxlength, complete_length, final?"true":"false");

            if (update_aborted)
                return true;

            // copy data
            memcpy(FlashVerify+11, gd_firmware_1_1_812 + chunk_offset, maxlength);  //  firmware file for verify button

            MAXLENGTH = maxlength;
            sendCommand(FlashVerify, maxlength+11, sendSequenceNumber++); // next chunk (11 bytes header) 
            flash_seq = PrivCommTxBuffer[5];
            last_flash = millis();
            ready_for_next_chunk = false;

            chunk_offset = chunk_offset + maxlength;
            length = length - maxlength;
        } // iterate through big chunks
    } // first chunk

    if(final) {
        this->firmware_update_running = false;
        logger.printfln("   scheduling GD chip app mode in 3s");
        // after last chunk, get out of flash mode
        task_scheduler.scheduleOnce("factory_reset", [this](){
            logger.printfln("   getting the GD chip back into app mode");
            sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);
        }, 3000);
    }

    return true;
}
#endif

