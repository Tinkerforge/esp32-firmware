/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2021-2022 Birger Schmidt <bs-warp@netgaroo.com>
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

#include "ac011k.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"
#include "modules.h"
#include "ac011rtc.h"
#include "ac011evse_v2.h"

#include "build.h"

#ifdef GD_FLASH
#include "GD_firmware.1.2.460.h"
#endif

#include "HardwareSerial.h"
#include <time.h>

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern WebServer server;

extern API api;
extern bool firmware_update_allowed;

#ifdef EXPERIMENTAL
/* experimental: Test command receiver */
#include <WiFiUdp.h>
#define UDP_RX_PACKET_MAX_SIZE 1024
WiFiUDP UdpListener;
unsigned int commandPort = 43211;  // local port to listen on
char receiveCommandBuffer[UDP_RX_PACKET_MAX_SIZE];  // buffer to hold incoming UDP data
/* end experimental */
#endif

Rtc rtc;
EVSEV2 evse;

::time_t GetTimeZoneOffset () {
  static const time_t seconds = 0; // any arbitrary value works!
  ::tm tmGMT = {}, tmLocal = {};
  ::gmtime_r(&seconds, &tmGMT); // ::gmtime_s() for WINDOWS
  ::localtime_r(&seconds, &tmLocal);  // ::localtime_s() for WINDOWS
  return ::mktime(&tmGMT) - ::mktime(&tmLocal);
}

bool ready_for_next_chunk = false;
size_t MAXLENGTH;
byte flash_seq;
uint32_t last_flash = 0;
bool phases_active[3];
bool phases_connected[3];
extern char local_uid_str[32];

// Charging profile:
// 10A ESP> W (2021-06-06 11:05:10) [PRIV_COMM, 1859]: Tx(cmd_AD len:122) :  FA 03 00 00 AD 1D 70 00 00 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15 06 06 0B 05 0A 00 00 00 00 0A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01 00 00 00 00 00 00 00 00 00 00 CE 75
// 12A ESP> W (2021-06-03 18:37:19) [PRIV_COMM, 1859]: Tx(cmd_AD len:122) :  FA 03 00 00 AD 19 70 00 00 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15 06 03 12 25 14 00 00 00 00 0C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01 00 00 00 00 00 00 00 00 00 00 BF 11
// 11A ESP> W (2021-06-04 08:07:58) [PRIV_COMM, 1859]: Tx(cmd_AD len:122) :  FA 03 00 00 AD 39 70 00 00 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 15 06 04 08 07 32 00 00 00 00 0B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 01 00 00 00 00 00 00 00 00 00 00 3A 0C
//

// Commands
// First byte = command code, then payload bytes, no crc bytes
byte Init1[] = {0xAC, 0x11, 0x0B, 0x01, 0x00, 0x00}; // 10b cmdACCtrlSetRemoteStart, 0
//byte Init2[] = {0xAC, 0x11, 0x09, 0x01, 0x00, 0x01}; // 109 cmdACCtrlSetS2OPENSTOP, 1 // bs & uwe
byte Init2[] = {0xAC, 0x11, 0x09, 0x01, 0x00, 0x00}; // 109 cmdACCtrlSetS2OPENSTOP, 0 // connie
byte Init3[] = {0xAC, 0x11, 0x0A, 0x01, 0x00, 0x00}; // 10a cmdACCtrlSetS2OPENLOCK, 0
byte Init4[] = {0xAC, 0x11, 0x0C, 0x01, 0x00, 0x00}; // 10c cmdACCtrlSetOfflineStop, 0
byte ClockAlignedDataInterval[] = {0xAA, 0x18, 0x3E, 0x04, 0x00, 10, 0, 0x00, 0x00}; // 10 + 0*256 sec // 13e cmdAACtrlSetRTCTime08, 0
byte Init6[] = {0xAC, 0x11, 0x0D, 0x04, 0x00, 0xB8, 0x0B, 0x00, 0x00}; // 10d cmdACSetOfflineEnergy, 3000
byte Init7[] = {0xAA, 0x18, 0x3F, 0x04, 0x00, 0x1E, 0x00, 0x00, 0x00}; // 13f cmdAACtrlSetGunTime, 30
byte Init8[] = {0xAA, 0x18, 0x25, 0x0E, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02}; // 125 cmdAACtrlSetSmartparam
byte Init12[] = {0xAA, 0x18, 0x12, 0x01, 0x00, 0x03}; // 112 cmdAACtrlSetReset, 3 // this triggers 0x02 SN, Hardware, Version

// cmdAACtrlcantestsetAck test cancom...111
byte Init11[] = {0xAA, 0x18, 0x2A, 0x00, 0x00}; // 

byte GetRtc[] = {0xAA, 0x10, 0x02, 0x00, 0x00};

byte TimeAck[] = {'c', 'a', 'y', 'm', 'd', 'h', 'm', 's', 0, 0, 0, 0};

// ctrl_cmd set start power mode done
byte Init15[] = {0xAA, 0x18, 0x09, 0x01, 0x00, 0x00};
//W (2021-04-11 18:36:27) [PRIV_COMM, 1764]: Tx(cmd_AA len:15) :  FA 03 00 00 AA 40 05 00 18 09 01 00 00 F9 36
//W (2021-04-11 18:36:27) [PRIV_COMM, 1919]: Rx(cmd_0A len:15) :  FA 03 00 00 0A 40 05 00 14 09 01 00 00 11 30
//I (2021-04-11 18:36:27) [PRIV_COMM, 279]: ctrl_cmd set start power mode done -> minpower: 3150080

byte CardAuthAckA5[] = {0xA5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x50, 0, 0, 0, 0}; // D0 = decline, 40 = accept, 50 = unknown ID (disabled)

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
// // evse GD status strings
static const char *evse_status_text[] = {
   "undefined",
   "Available (not engaged)",
   "Preparing (engaged, not started)",
   "Charging (charging ongoing, power output)",
   "Suspended by charger (started but no power available)",
   "Suspended by EV (power available but waiting for the EV response)",
   "Finishing, charging acomplished (RFID stop or EMS control stop)",
   "(Reserved)",
   "(Unavailable)",
   "Fault (charger in fault condition)",
};

static const char *stop_reason_text[] = {
   "undefined",
   "Remote",
   "undefined",
   "EV disconnected",
};

static const char *cmd_0B_text[] = {
    "Reset into application mode",
    "Handshake",
    "Flash erase",
    "Flash write",
    "Flash verify",
    "Reset into boot mode",
};

static const char *evse_slot_name[] = {
    "INCOMING_CABLE",
    "OUTGOING_CABLE",
    "SHUTDOWN_INPUT",
    "GP_INPUT",
    "AUTOSTART_BUTTON",
    "GLOBAL",
    "USER",
    "CHARGE_MANAGER",
    "EXTERNAL",
    "MODBUS_TCP",
    "MODBUS_TCP_ENABLE",
    "OCPP",
};


#ifdef EXPERIMENTAL
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
#endif

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

String AC011K::get_hex_privcomm_line(byte *data) {
    #define LOG_LEN 4048 //TODO work without such a big buffer by writing line by line
    char log[LOG_LEN] = {0};

    if (!(data[4] == 0xAB and data[14] == 3)) { // suppress logging of the whole GD firmware during flashing

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
            if(i>0) { local_log += snprintf(local_log, LOG_LEN - (local_log - log), "\r\n\t\t"); }
            if(i>=BUFFER_CHUNKS) { local_log += snprintf(local_log, LOG_LEN - (local_log - log), "\t"); }
            local_log += snprintf(local_log, LOG_LEN - (local_log - log), "\t%.3d: ", offset);
            offset = offset + BUFFER_CHUNKS;
        }
        local_log += snprintf(local_log, LOG_LEN - (local_log - log), "%.2X ", data[i]);
    }
    local_log += snprintf(local_log, LOG_LEN - (local_log - log), "\r\n");
    logger.write(log, local_log - log);
    
    }
    
    return String(log);
}

void AC011K::Serial2write(byte *data, int size) {
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

void AC011K::sendCommand(byte *data, int datasize, byte sendSequenceNumber, bool verbose /* default = true */) {
    PrivCommTxBuffer[4] = data[0]; // command code
    PrivCommTxBuffer[5] = sendSequenceNumber;
    PrivCommTxBuffer[6] = (datasize-1) & 0xFF;
    PrivCommTxBuffer[7] = (datasize-1) >> 8;
    memcpy(PrivCommTxBuffer+8, data+1, datasize-1);

    uint16_t crc = crc16_modbus(PrivCommTxBuffer, datasize + 7);

    PrivCommTxBuffer[datasize+7] = crc & 0xFF;
    PrivCommTxBuffer[datasize+8] = crc >> 8;

    if(verbose || ac011k_hardware.config.get("verbose_communication")->asBool()
        /* || ( */
        /*     // we are silent by default for the heartbeat and other well known stuff */
        /*     (data[0]!=0x0A) && // AA answer */
        /*     (data[0]!=0xA3) && // Status */
        /*     (data[0]!=0xA4) && // heartbeat */
        /*     (data[0]!=0xA5) && // RFID */
        /*     //(data[0]!=0xA6) && // RemoteTransactionReq (Start/Stop) */
        /*     //(data[0]!=0xA7) && // Start/Stop charging approval */
        /*     (data[0]!=0xA8) && // ClockAlignedDataInterval */
        /*     (data[0]!=0xA9) && // Charging stop */
        /*     (data[0]!=0xAB) && // Update */
        /*     //(data[0]!=0xAD) && // Limit ack */
        /*     (data[0]!=0xAE) && // ChargingParameterRpt */
        /*     (data[0]!=0xAF)    // Charging limit request */
        /*     ) */
        ) 
    {
        get_hex_privcomm_line(PrivCommTxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
        String cmdText = "";
        switch (PrivCommTxBuffer[4]) {
            case 0xA3: cmdText = "- Status data ack"; break;
            case 0xA4: cmdText = "- Heartbeat ack " + String(timeStr(&PrivCommTxBuffer[9])); break;
            case 0xA6: if (PrivCommTxBuffer[72] == 0x40) cmdText = "- Stop charging request"; else cmdText = "- Start charging request"; break;
            case 0xA7: if (PrivCommTxBuffer[40] == 0x10) cmdText = "- Stop charging command"; else cmdText = "- Start charging command"; break;
            case 0xA8: cmdText = "- Power data ack"; break;
            case 0xA9: cmdText = "- Transaction data ack"; break;
            case 0xAD: if (PrivCommTxBuffer[8] == 0) cmdText = "- Limit2: " + String(PrivCommTxBuffer[72]) + " Ampere"; else cmdText = "- Limit3: " + String(PrivCommTxBuffer[77]) + " Ampere"; break;
            case 0xAF: cmdText = "- Limit1: " + String(PrivCommTxBuffer[24]) + " Ampere"; break;
        }
        logger.printfln("Tx cmd_%.2X seq:%.2X len:%d crc:%.4X %s", PrivCommTxBuffer[4], PrivCommTxBuffer[5], datasize+9, crc, cmdText.c_str());
    }

    Serial2write(data, datasize + 9);
}

void AC011K::PrivCommSend(byte cmd, uint16_t datasize, byte *data, bool verbose /* default = true */) {
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

    if(verbose || ac011k_hardware.config.get("verbose_communication")->asBool()) {
        get_hex_privcomm_line(data); // PrivCommHexBuffer now holds the hex representation of the buffer
        logger.printfln("Tx cmd_%.2X seq:%.2X, len:%d, crc:%.4X", cmd, data[5], datasize, crc);
    }

    Serial2write(data, datasize + 10);
}

void AC011K::PrivCommAck(byte cmd, byte *data) {
    // the first 4 bytes never change and should be set already
    data[4] = cmd ^ 0xA0;  // complement command for ack
    data[6] = 1; //len
    //data[5]=seq; //should already be copied from RxBuffer
    data[7] = 0; //len
    data[8] = 0; //payload
    uint16_t crc = crc16_modbus(data, 9);
    data[9] = crc & 0xFF;
    data[10] = crc >> 8;

    /* if(ac011k_hardware.config.get("verbose_communication")->asBool() || cmd!=2) { // be silent for the heartbeat //TODO show it at first and after an hour? */
    /*     get_hex_privcomm_line(data); // PrivCommHexBuffer now holds the hex representation of the buffer */
    /*     logger.printfln("Tx cmd_%.2X seq:%.2X, crc:%.4X", data[4], data[5], crc); */
    /* } */

    Serial2write(data, 11);
}

char timeString[25];  // global since local variable could not be used as return value
const char* AC011K::timeStr(byte *data) {
    sprintf(timeString, "%04d/%02d/%02d %02d:%02d:%02d",
        (int)(data[0])+2000, data[1], data[2],
        data[3], data[4], data[5]
    );
    return timeString;
}

void AC011K::sendTime(byte cmd, byte action, byte len, byte sendSequenceNumber) {
    TimeAck[0] = cmd;
    TimeAck[1] = action;
    fillTimeGdCommand(&TimeAck[2]);
    // TimeAck[8] to TimeAck[11] are always 0
    sendCommand(TimeAck, len, sendSequenceNumber, false);
}

void AC011K::fillTimeGdCommand(byte *datetime) {
    struct timeval tv_now;
    struct tm timeinfo;

    if (clock_synced(&tv_now)) {
        gmtime_r(&tv_now.tv_sec, &timeinfo);
        datetime[0] = (byte)(timeinfo.tm_year - 100);
        datetime[1] = (byte)(timeinfo.tm_mon + 1);
        datetime[2] = (byte)(timeinfo.tm_mday);
        datetime[3] = (byte)(timeinfo.tm_hour);
        datetime[4] = (byte)(timeinfo.tm_min);
        datetime[5] = (byte)(timeinfo.tm_sec);
    } else if((PrivCommRxBuffer[8] == GD_GET_RTC_ANSWER) && (PrivCommRxBuffer[9] == GD_0A_TIME_ANSWER)) { 
        datetime[0] = PrivCommRxBuffer[PayloadStart + 4]; // year  
        datetime[1] = PrivCommRxBuffer[PayloadStart + 5]; // month 
        datetime[2] = PrivCommRxBuffer[PayloadStart + 6]; // day   
        datetime[3] = PrivCommRxBuffer[PayloadStart + 7]; // hour  
        datetime[4] = PrivCommRxBuffer[PayloadStart + 8]; // minute
        datetime[5] = PrivCommRxBuffer[PayloadStart + 9]; // second
        //if(ac011k_hardware.config.get("verbose_communication")->asBool())
            logger.printfln("time from GD RTC %s", timeStr(datetime));
    } else {
        datetime[0] = 22; // year  
        datetime[1] = 1;  // month 
        datetime[2] = 2;  // day   
        datetime[3] = 3;  // hour  
        datetime[4] = 4;  // minute
        datetime[5] = 5;  // second
        logger.printfln("time fill went wrong, using FAKE %s", timeStr(datetime));
    }
}

void AC011K::GetRTC() {
    sendCommand(GetRtc, sizeof(GetRtc), sendSequenceNumber++, false);
}

void AC011K::SetRTC() {
    PrivCommTxBuffer[5] = sendSequenceNumber++;
    PrivCommTxBuffer[PayloadStart + 0] = 0x18;
    PrivCommTxBuffer[PayloadStart + 1] = 0x02;
    PrivCommTxBuffer[PayloadStart + 2] = 0x06;
    PrivCommTxBuffer[PayloadStart + 3] = 0x00;
    fillTimeGdCommand(&PrivCommTxBuffer[PayloadStart + 4]);
    PrivCommSend(0xAA, 10, &PrivCommTxBuffer[0], false);
}

void AC011K::SetRTC(timeval time) {
    settimeofday(&time, nullptr);
    SetRTC();
}

void AC011K::sendChargingLimit1(uint8_t currentLimit, byte sendSequenceNumber) {  // AF 00 date/time
    fillTimeGdCommand(&ChargingLimit1[2]);
    ChargingLimit1[17] = currentLimit;
    sendCommand(ChargingLimit1, sizeof(ChargingLimit1), sendSequenceNumber, false);
}

void AC011K::sendChargingLimit2(uint8_t currentLimit, byte sendSequenceNumber) {  // AD 00
//    ChargingLimit2[2] = 8;  // charging profile ID - 0x41 for 1.0.1435 ?
    fillTimeGdCommand(&ChargingLimit2[55]);
    ChargingLimit2[65] = currentLimit;
    sendCommand(ChargingLimit2, sizeof(ChargingLimit2), sendSequenceNumber, false);
}

void AC011K::sendChargingLimit3(uint8_t currentLimit, byte sendSequenceNumber) {  //  AD 01 91
    fillTimeGdCommand(&ChargingLimit3[56]);
    ChargingLimit3[56] = ChargingLimit3[56] +100;  // adds 100 to the year, because it starts at the year 1900
    ChargingLimit3[70] = currentLimit;
    sendCommand(ChargingLimit3, sizeof(ChargingLimit3), sendSequenceNumber, false);
}


int AC011K::bs_evse_start_charging() {
    uint8_t allowed_charging_current = uint8_t(evse.evse_state.get("allowed_charging_current")->asUint()/1000);
    logger.printfln("EVSE start charging with max %d Ampere", allowed_charging_current);

    switch (evse.evse_hardware_configuration.get("GDFirmwareVersion")->asUint()) {
        case 212:
            sendCommand(StartChargingA6, sizeof(StartChargingA6), sendSequenceNumber++);  // max current is switched to 16A when plugged out to ensure full current range available
            sendChargingLimit3(allowed_charging_current, sendSequenceNumber++);  // reduce to intended value
            break;
        default:
            logger.printfln("Unknown firmware version. Trying commands for latest version.");
            __attribute__ ((fallthrough));
        case 258:
        case 460:
        case 538:
        case 805:
        case 812:
        case 888:
        case 1435:
            sendCommand(StartChargingA6, sizeof(StartChargingA6), sendSequenceNumber++, false);
            break;
    }
    return 0;
}

int AC011K::bs_evse_stop_charging() {
    logger.printfln("EVSE stop charging");
    sendCommand(StopChargingA6, sizeof(StopChargingA6), sendSequenceNumber++, false);
    return 0;
}

int AC011K::bs_evse_persist_config() {
    api.writeConfig("evse/slots", &(evse.evse_slots));
    /* String error = api.callCommand("evse/config_update", Config::ConfUpdateObject{{ */
    /*     {"auto_start_charging", evse_auto_start_charging.get("auto_start_charging")->asBool()}, */
    /*     {"max_current_configured", evse_max_charging_current.get("max_current_configured")->asUint()}, */
    /*     //{"managed", evse_managed.get("managed")->asBool()} */
    /* }}); */
    /* if (error != "") { */
    /*     logger.printfln("Failed to save config: %s", error.c_str()); */
    /*     return 500; */
    /* } else { */
    /* logger.printfln("saved config - auto_start_charging: %s, managed: %s, max_current_configured: %d", */
    /*         evse_config.get("auto_start_charging")->asBool() ?"true":"false", */
    /*         evse_config.get("managed")->asBool() ?"true":"false", */
    /*         evse_config.get("max_current_configured")->asUint()); */
    /*     return 0; */
    /* } */
    return 0;
}

int AC011K::bs_evse_set_charging_autostart(bool autostart) {
    logger.printfln("EVSE set auto start charging to %s", autostart ? "true" :"false");
    evse.evse_auto_start_charging.get("auto_start_charging")->updateBool(autostart);
    bs_evse_persist_config();
    return 0;
}

int AC011K::bs_evse_set_max_charging_current(uint16_t max_current) {
    //bs_evse_persist_config();
    //update_all_data();
    evse.evse_state.get("allowed_charging_current")->updateUint(max_current);
    uint8_t allowed_charging_current = evse.evse_state.get("allowed_charging_current")->asUint()/1000;
    logger.printfln("EVSE set allowed charging limit to %d Ampere", allowed_charging_current);
    switch (evse.evse_hardware_configuration.get("GDFirmwareVersion")->asUint()) {
        case 212:
            sendChargingLimit3(allowed_charging_current, sendSequenceNumber++);
            break;
        default:
            logger.printfln("Unknown firmware version. Trying commands for latest version.");
            __attribute__ ((fallthrough));
        case 258:
        case 460:
        case 538:
        case 805:
        case 812:
        case 888:
        case 1435:
            sendChargingLimit1(allowed_charging_current, sendSequenceNumber++);
    }
    return 0;
}

void AC011K::evse_slot_machine() {
    /* 
     * this function is to do the logic and calculate the state of the EVSE
     *
     */

    static uint16_t last_allowed_charging_current;
    static uint16_t last_iec61851_state;

    // perform clear on disconnect
    if((evse.evse_state.get("iec61851_state")->asUint() == IEC_STATE_A) && (last_iec61851_state != IEC_STATE_A)) {
        for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
            if (evse.evse_slots.get(i)->get("clear_on_disconnect")->asBool()) {
                if ((i == CHARGING_SLOT_AUTOSTART_BUTTON) && (evse.evse_auto_start_charging.get("auto_start_charging")->asBool())){
                    evse.apply_slot_default(i, 32000, evse.evse_slots.get(i)->get("active")->asBool(), true);
                }
                else {
                    evse.apply_slot_default(i, 0, evse.evse_slots.get(i)->get("active")->asBool(), true);
                }
            }
        }
    }

    // find the charging current maximum
    uint16_t allowed_charging_current = 32000;
    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
        if (!evse.evse_slots.get(i)->get("active")->asBool())
            continue;
        allowed_charging_current = min(allowed_charging_current, (uint16_t)evse.evse_slots.get(i)->get("max_current")->asUint());
    }

    // plugged in
    if((last_iec61851_state == IEC_STATE_A) && (evse.evse_state.get("iec61851_state")->asUint() == IEC_STATE_B) && (allowed_charging_current > 0)) {
        logger.printfln("EVSE Just plugged in and allowed charging current > 0 (%dA)", allowed_charging_current/1000);
        bs_evse_start_charging();
    }
    
    // plugged out
    if((last_iec61851_state != IEC_STATE_A) && (evse.evse_state.get("iec61851_state")->asUint() == IEC_STATE_A)) {
        bs_evse_stop_charging();
        if (evse.evse_hardware_configuration.get("GDFirmwareVersion")->asUint() == 212)
            //sendChargingLimit2(16, sendSequenceNumber++);  // hack to ensure full current range is available in next charging session 
            sendChargingLimit3(16, sendSequenceNumber++);  // hack to ensure full current range is available in next charging session
    }

    if(last_allowed_charging_current != allowed_charging_current) {
        
        evse.evse_state.get("allowed_charging_current")->updateUint(allowed_charging_current);
        logger.printfln("EVSE Allowed charging current changed from %dmA to %dmA", last_allowed_charging_current, allowed_charging_current);

        if((last_allowed_charging_current == 0) && // now > 0
                (evse.evse_state.get("iec61851_state")->asUint() == IEC_STATE_B) && // plugged
                (evse.evse_state.get("charger_state")->asUint() == CHARGER_STATE_WAITING_FOR_RELEASE)) {
            logger.printfln("EVSE Start charging, set allowed charging current to %dmA, IEC_STATE %d", allowed_charging_current, evse.evse_state.get("iec61851_state")->asUint());
            bs_evse_start_charging();
        }
        else if(allowed_charging_current == 0) { // current shall be 0, no more charging allowed
            bs_evse_stop_charging();
        }
        else if((last_iec61851_state != IEC_STATE_A) &&                                // was not unplugged before
                ((evse.evse_state.get("iec61851_state")->asUint() == IEC_STATE_B) ||   // is now plugged
                (evse.evse_state.get("iec61851_state")->asUint() == IEC_STATE_C))) {   // or already charging
            logger.printfln("EVSE Allowed charging current change during charging from %dmA to %dmA", last_allowed_charging_current, allowed_charging_current);
            bs_evse_set_max_charging_current(allowed_charging_current);
        }
        else {
            //logger.printfln("EVSE unhandeled state change, allowed charging current: %d, IEC_STATE %d", allowed_charging_current, evse.evse_state.get("iec61851_state")->asUint());
        }
    }

    last_allowed_charging_current = allowed_charging_current;
    last_iec61851_state = evse.evse_state.get("iec61851_state")->asUint();
}

void AC011K::update_evseStatus(uint8_t evseStatus) {
    //if(evse_hardware_configuration.get("initialized")->asBool()) { // only update the EVSE status if we support the hardware
    uint8_t last_iec61851_state = evse.evse_state.get("iec61851_state")->asUint();
    uint8_t last_evseStatus = evse.evse_state.get("GD_state")->asUint();
    evse.evse_state.get("GD_state")->updateUint(evseStatus);

    //if(!ac011k_hardware.config.get("verbose_communication")->asBool() && (evseStatus != last_evseStatus)) {
    if(evseStatus != last_evseStatus) {
        logger.printfln("EVSE GD Status now %d: %s, allowed charging current %dmA", evseStatus, evse_status_text[evseStatus], evse.evse_state.get("allowed_charging_current")->asUint());
    }

    switch (evseStatus) {
        case 1: // Available (not engaged)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_A); // Nicht verbunden
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_NOT_PLUGGED_IN);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 2: // Preparing (engaged, not started)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_B); // Verbunden
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_WAITING_FOR_RELEASE);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 3: // Charging (charging ongoing, power output)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_C); // Lädt
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_CHARGING);
            evse.evse_state.get("contactor_state")->updateUint(3); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_LIVE
            for (int i = 0; i < 3; ++i)
                phases_active[i] = phases_connected[i];
            break;
        case 4: // Suspended by charger (started but no power available)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_B); // Verbunden
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_READY_TO_CHARGE);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 5: // Suspended by EV (power available but waiting for the EV response)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_B); // Verbunden
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_READY_TO_CHARGE);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 6: // Finishing, charging acomplished (RFID stop or EMS control stop)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_B); // Verbunden
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_WAITING_FOR_RELEASE);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 7: // (Reserved)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_EF);
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_ERROR);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 8: // (Unavailable)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_EF);
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_ERROR);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        case 9: // Fault (charger in fault condition)
            evse.evse_state.get("iec61851_state")->updateUint(IEC_STATE_EF);
            evse.evse_state.get("charger_state")->updateUint(CHARGER_STATE_ERROR);
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            break;
        default:
            evse.evse_state.get("contactor_state")->updateUint(1); // TF_EVSE_V2_CONTACTOR_STATE_AC1_LIVE_AC2_NLIVE
            logger.printfln("err: can not determine EVSE status %d", evseStatus);
            break;
    }
    firmware_update_allowed = evse.evse_state.get("charger_state")->asUint() == CHARGER_STATE_NOT_PLUGGED_IN || evse.evse_state.get("charger_state")->asUint() == CHARGER_STATE_ERROR;
    meter.updateMeterPhases(phases_connected, phases_active);
    
    if(last_iec61851_state != evse.evse_state.get("iec61851_state")->asUint()) {
        evse.evse_state.get("last_state_change")->updateUint(millis());
        evse.evse_state.get("time_since_state_change")->updateUint(millis() - evse.evse_state.get("last_state_change")->asUint());
        evse.evse_low_level_state.get("time_since_state_change")->updateUint(evse.evse_state.get("time_since_state_change")->asUint());
    }
}


#ifdef GD_FLASH
/* GD Firmware updater */

bool AC011K::handle_update_chunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length) {

    if(chunk_index == 0) {
        logger.printfln("EVSE RemoteUpdate, reset into boot mode");
        RemoteUpdate[7] = 5; // Reset into boot mode
        sendCommand(RemoteUpdate, sizeof(RemoteUpdate), sendSequenceNumber++, false);

        size_t chunk_offset = 0 + 0x8000;
        size_t length = gd_firmware_len - 0x8000;

        FlashVerify[7] = command; // flash write (3=write, 4=verify)

        while (length > 0) {
            while (!ready_for_next_chunk) {
                loop(); //TODO make this more elegant
            }

            //calculate maxlength
            size_t maxlength = 512;               // 512 byte chunks
            if (length < 512) maxlength = length; // reminder
            FlashVerify[9]  = (maxlength/2 & 0x000000FF); // number of words to process (therefore divided by 2)
            FlashVerify[10] = (maxlength/2 & 0x0000FF00) >> 8;

            //calculate address
            uint32_t gd_address = chunk_index + chunk_offset + 0x8000000; // 0x8000000 is the start address for the GD chip
            FlashVerify[5] = (gd_address & 0x000000FF);
            FlashVerify[6] = (gd_address & 0x0000FF00) >> 8;
            FlashVerify[3] = (gd_address & 0x00FF0000) >> 16;
            FlashVerify[4] = (gd_address & 0xFF000000) >> 24;

            logger.printfln("gd(%.2x%.2x%.2x%.2x) chunk_offset %d, l %d, ml %d", FlashVerify[3],FlashVerify[4],FlashVerify[5],FlashVerify[6], chunk_offset, length, maxlength);

            if (update_aborted)
                return true;

            // copy data
            memcpy(FlashVerify+11, gd_firmware_1_2_460 + chunk_offset, maxlength);  // firmware file for upload button

            MAXLENGTH = maxlength;
            sendCommand(FlashVerify, maxlength+11, sendSequenceNumber++, false); // next chunk (11 bytes header) 
            flash_seq = PrivCommTxBuffer[5];
            last_flash = millis();
            ready_for_next_chunk = false;

            chunk_offset = chunk_offset + maxlength;
            length = length - maxlength;
        } // iterate through big chunks
    } // first chunk
    return true;
}
#endif

void AC011K::factory_reset()
{
    evse.factory_reset();
    logger.printfln("AC011K factory reset is not implemented yet.");
}

void AC011K::update_all_data() {
    evse.update_all_data();
    evse_slot_machine();
}

void AC011K::set_user_current(uint16_t current) {
    evse.set_user_current(current);
}

void AC011K::pre_setup() {
    rtc.pre_setup();
    rtc.setup();
    rtc.register_urls();
    evse.pre_setup();
    evse_state = &evse.evse_state;
    evse_low_level_state = &evse.evse_low_level_state;
    evse_slots = &evse.evse_slots;
}

void AC011K::setup() {
    evse.setup_evse();
    evse.update_all_data();
    evse.setup();
    task_scheduler.scheduleWithFixedDelay([this](){
        update_all_data();
    }, 0, 250);
#ifdef EXPERIMENTAL
    UdpListener.begin(commandPort); // experimental
#endif

    // Get settings from LittleFS or set them to defaults
    if(api.restorePersistentConfig("evse/slots", &(evse.evse_slots))) {
        for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
            logger.printfln("Slot %d active: %s clear_on_disconnect: %s %dmA (%s)", i, evse.evse_slots.get(i)->get("active")->asBool()?"true":"false", evse.evse_slots.get(i)->get("clear_on_disconnect")->asBool()?"true":"false", (uint16_t)evse.evse_slots.get(i)->get("max_current")->asUint(), evse_slot_name[i]);
        }
    } else {
        logger.printfln("EVSE error, could not restore persistent storage slots config, setting defaults.");
        // Slot 0 to 3 are set according to the hardware
        evse.apply_slot_default(CHARGING_SLOT_INCOMING_CABLE, 16000, true, false);
        evse.apply_slot_default(CHARGING_SLOT_OUTGOING_CABLE, 16000, true, false);
        evse.apply_slot_default(CHARGING_SLOT_SHUTDOWN_INPUT, 0, false, false);
        evse.apply_slot_default(CHARGING_SLOT_GP_INPUT, 0, false, false);
        evse.apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 0, false, false);
        if (!evse.evse_slots.get(CHARGING_SLOT_GLOBAL)->get("active")->asBool())
            evse.apply_slot_default(CHARGING_SLOT_GLOBAL, 32000, true, false);
        evse.apply_slot_default(CHARGING_SLOT_USER, 0, evse.evse_slots.get(CHARGING_SLOT_USER)->get("active")->asBool(), true);
        evse.apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, evse.evse_slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("active")->asBool(), true);
        evse.apply_slot_default(CHARGING_SLOT_EXTERNAL, 0, false, false);
        evse.apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 0, false, false);
        evse.apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 0, false, false);
        evse.apply_slot_default(CHARGING_SLOT_OCPP, 0, false, false);
    }
    evse.evse_auto_start_charging.get("auto_start_charging")->updateBool(
        !evse.evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());

    Serial2.setRxBufferSize(1024);
    Serial2.begin(115200, SERIAL_8N1, 26, 27); // PrivComm to EVSE GD32 Chip
    //Serial2.setTimeout(90);
    logger.printfln("Set up PrivComm: 115200, SERIAL_8N1, RX 26, TX 27");

    /* // TODO start: look out for this on a unconfigured box ( no wifi ) - if it still works, delete the code */
    /* setTime(23,59,00,31,12,2018); */
    /* switch (timeStatus()){ */
    /*     case timeNotSet: */
    /*         logger.printfln("the time has never been set, the clock started on Jan 1, 1970"); */
    /*         break; */
    /*     case timeNeedsSync: */
    /*         logger.printfln("the time had been set but a sync attempt did not succeed"); */
    /*         break; */
    /*     case timeSet: */
    /*         logger.printfln("the time is set and is synced"); */
    /*         break; */
    /* } */
    /* logger.printfln("the time is %d", now()); */
    /* logger.printfln("the now() call was not blocking"); */
    /* // TODO end: look out for this on a unconfigured box ( no wifi ) - if it still works, delete the code */


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
    sendCommand(ClockAlignedDataInterval,  sizeof(ClockAlignedDataInterval), sendSequenceNumber++);
    sendCommand(Init6,  sizeof(Init6), sendSequenceNumber++);
    sendCommand(Init7,  sizeof(Init7), sendSequenceNumber++);
    sendCommand(Init8,  sizeof(Init8), sendSequenceNumber++);
    GetRTC();
    /* we had init9 and init10 in here, but connie found that it is not in the original FW and */ 
    /* now we think it was just a copy/paste mistake and a duplicate of init12 */
    /* sendCommand(Init9,  sizeof(Init9), sendSequenceNumber++); */
    /* sendCommand(Init10, sizeof(Init10), sendSequenceNumber++);  // last two bytes correct? */

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
    PrivCommTxBuffer[PayloadStart + 4] =  240;  /* 240 sec heartbeat timeout */
    PrivCommTxBuffer[PayloadStart + 5] = 0x00;  /* heartbeat timeout 16bit   */
    PrivCommSend(0xAA, 6, PrivCommTxBuffer);


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

    // patch transaction number into command templates
    sprintf((char*)StartChargingA7 +1, "%06d", transactionNumber);
    sprintf((char*)StopChargingA7  +1, "%06d", transactionNumber);
    sprintf((char*)StopChargingA6 +33, "%06d", transactionNumber);
    logger.printfln("Initial transaction number %05d", transactionNumber);

    meter.updateMeterState(2, 99);
    logger.printfln("Internal meter enabeled");
}
            

void AC011K::loop()
{
    evse.loop();
    initialized = evse.initialized;
    static struct timeval tv_now;
    static uint8_t evseStatus = 0;
    static uint8_t cmd;
    static uint8_t seq;
    static uint16_t len;
    uint16_t crc = 0;
    static bool cmd_to_process = false;
    static byte PrivCommRxState = PRIVCOMM_MAGIC;
    static int PrivCommRxBufferPointer = 0;
    byte rxByte;
    if (rtc.rtc_updated && ntp.state.get("time")->asUint() == 0) {
        // only use the browser time if NTP is not synced
        timeval time;
        time.tv_sec = rtc.get_time(true) - GetTimeZoneOffset();
        time.tv_usec = 0;
        SetRTC(time);
        logger.printfln("Got time from browser. Set RTC to: %s UTC", timeStr(&PrivCommTxBuffer[PayloadStart + 4]));
    }

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
                            cmd_to_process = true;
                        }
                    }
                    break;
                case PRIVCOMM_CRC:
                    if(PrivCommRxBufferPointer == len + 10) {
                        PrivCommRxState = PRIVCOMM_MAGIC;
                        if(ac011k_hardware.config.get("verbose_communication")->asBool() || ( // we are silent by default for the heartbeat and other well known stuff
                            (cmd!=0x02) && // Serial
                            (cmd!=0x03) && // Status
                            (cmd!=0x04) && // heartbeat
                            (cmd!=0x05) && // RFID
                            (cmd!=0x06) && // RemoteTransactionReq (Start/Stop)
                            (cmd!=0x07) && // Start/Stop charging approval
                            (cmd!=0x08) && // ClockAlignedDataInterval
                            (cmd!=0x09) && // Charging stop
                            (cmd!=0x0A) && // cmdAACtrl answer
                            (cmd!=0x0B) && // Update
                            (cmd!=0x0C) && // cmdACCtrl answer
                            (cmd!=0x0D) && // Limit ack
                            (cmd!=0x0E) && // ChargingParameterRpt
                            (cmd!=0x0F)    // Charging limit request
                            )) {
                            get_hex_privcomm_line(PrivCommRxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
                        }
                        crc = (uint16_t)(PrivCommRxBuffer[len + 9] << 8 | PrivCommRxBuffer[len + 8]);
                        uint16_t checksum = crc16_modbus(PrivCommRxBuffer, len+8);
                        if(crc == checksum) {
                            if(!evse.evse_found) {
                                logger.printfln("EN+ GD EVSE found. Enabling EVSE support.");
                                evse.evse_found = true;
                                evse.evse_hardware_configuration.get("evse_found")->updateBool(evse.evse_found);
                            }
                        } else {
                            logger.printfln("CRC ERROR Rx cmd_%.2X seq:%.2X len:%d crc:%.4X checksum:%.4X", cmd, seq, len, crc, checksum);
                            break;
                        }
                        cmd_to_process = true;
                    }
                    break;
            }  //switch read packet
        } while((Serial2.available() > 0) && !cmd_to_process && PrivCommRxBufferPointer<sizeof(PrivCommRxBuffer)/sizeof(PrivCommRxBuffer[0])); // one command at a time
    }

    char str[20];
    uint8_t allowed_charging_current = uint8_t(evse.evse_state.get("allowed_charging_current")->asUint()/1000);
    //time_t t=now();     // get current time

    if(cmd_to_process) {
        switch( cmd ) {
            case 0x02: // Info: Serial number, Version
//W (1970-01-01 00:08:52) [PRIV_COMM, 1919]: Rx(cmd_02 len:135) :  FA 03 00 00 02 26 7D 00 53 4E 31 30 30 35 32 31 30 31 31 39 33 35 37 30 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 24 D1 00 41 43 30 31 31 4B 2D 41 55 2D 32 35 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 31 2E 31 2E 32 37 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 00 00 00 00 00 5A 00 1E 00 00 00 00 00 00 00 00 00 D9 25
//W (1970-01-01 00:08:52) [PRIV_COMM, 1764]: Tx(cmd_A2 len:11) :  FA 03 00 00 A2 26 01 00 00 99 E0
                if(ac011k_hardware.config.get("verbose_communication")->asBool()) { // we may be silent for the heartbeat //TODO show it at first and after an hour?
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Serial number and version.", cmd, seq, len, crc);
                }
                sprintf(str, "%s", PrivCommRxBuffer+8);
                evse.evse_hardware_configuration.get("SerialNumber")->updateString(str);
                if(strlen(local_uid_str) == 0) {
                    sprintf(local_uid_str, "%s",PrivCommRxBuffer+10);
                    logger.printfln("This seems to be the first boot after a reset. Setting the UID based on the serial number to %s and reboot.", local_uid_str);
                    ac011k_hardware.persist_config();
                    trigger_reboot("Empty UID");
                }
                sprintf(str, "%s",PrivCommRxBuffer+43);
                evse.evse_hardware_configuration.get("Hardware")->updateString(str);
                sprintf(str, "%s",PrivCommRxBuffer+91);
                evse.evse_hardware_configuration.get("FirmwareVersion")->updateString(str);
                if(!evse.evse_hardware_configuration.get("initialized")->asBool()) {
                    evse.initialized = (
                            (evse.evse_hardware_configuration.get("Hardware")->asString().compareTo("AC011K-AU-25") == 0) 
                            || (evse.evse_hardware_configuration.get("Hardware")->asString().compareTo("AC011K-AE-25") == 0)
                        )
                        && (   
                            (   
                                evse.evse_hardware_configuration.get("FirmwareVersion")->asString().startsWith("1.1.", 0)  // known working: 1.1.27, 1.1.212, 1.1.258, 1.1.525, 1.1.538, 1.1.805, 1.1.812, 1.1.888
                                && (evse.evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt() <= 888)  // higest known working version (we assume earlier versions work as well)
                            )
                            || (   
                                evse.evse_hardware_configuration.get("FirmwareVersion")->asString().startsWith("1.0.", 0)  // known working: 1.0.1435
                                && (evse.evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt() <= 1435)  // higest known working version (we assume earlier versions work as well)
                            )
                            || (   
                                evse.evse_hardware_configuration.get("FirmwareVersion")->asString().startsWith("1.2.", 0)  // known working: 1.2.460
                                && (evse.evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt() <= 460)  // higest known working version (we assume earlier versions work as well)
                            )
                        );
                    evse.evse_hardware_configuration.get("initialized")->updateBool(evse.initialized);
                    initialized = evse.initialized;
                    evse.evse_hardware_configuration.get("GDFirmwareVersion")->updateUint(evse.evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt());
                    evse.evse_hardware_configuration.get("evse_version")->updateUint(evse.evse_hardware_configuration.get("FirmwareVersion")->asString().substring(4).toInt());
                    logger.printfln("EVSE serial: %s hw: %s fw: %s", 
                        evse.evse_hardware_configuration.get("SerialNumber")->asEphemeralCStr(),
                        evse.evse_hardware_configuration.get("Hardware")->asEphemeralCStr(),
                        evse.evse_hardware_configuration.get("FirmwareVersion")->asEphemeralCStr());
                    if(evse.initialized) {
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
                update_evseStatus(evseStatus);
                if(ac011k_hardware.config.get("verbose_communication")->asBool()) {
                    //TODO figure out what substatus (PrivCommRxBuffer[8]) is or should be
                    //TODO figure out what substatus (PrivCommRxBuffer[11]) is or should be
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Status %d: %s substatus b0 %.2X b3 %.2X", cmd, seq, len, crc, evseStatus, evse_status_text[evseStatus], PrivCommRxBuffer[8], PrivCommRxBuffer[11]);
                    // there is the time in the 03 cmd - do we need it for something?
                    logger.printfln("       time: %d/%d/%d %d:%d.%d", PrivCommRxBuffer[22],PrivCommRxBuffer[23],PrivCommRxBuffer[24],PrivCommRxBuffer[25],PrivCommRxBuffer[26],PrivCommRxBuffer[27]);
                }
                sendTime(0xA3, 0x10, 8, seq);  // send ack
                break;

            case 0x04: // time request / ESP32-GD32 communication heartbeat
//W (1970-01-01 00:08:52) [PRIV_COMM, 1919]: Rx(cmd_04 len:16) :  FA 03 00 00 04 28 06 00 09 00 00 00 00 00 C0 5E
//W (1970-01-01 00:08:52) [PRIV_COMM, 1764]: Tx(cmd_A4 len:17) :  FA 03 00 00 A4 28 07 00 00 E2 01 01 00 08 34 E5 6B
                evseStatus = PrivCommRxBuffer[8];
                update_evseStatus(evseStatus);
                if(ac011k_hardware.config.get("verbose_communication")->asBool()) { // we may be silent for the heartbeat //TODO show it at first and after an hour?
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Heartbeat request, Status %d: %s, value:%d", cmd, seq, len, crc, evseStatus, evse_status_text[evseStatus], PrivCommRxBuffer[12]);
                }
#ifdef EXPERIMENTAL
// experimental:
                send_http(String(",\"type\":\"en+04\",\"data\":{")
                    +"\"status\":"+String(PrivCommRxBuffer[8])  // status
                    +",\"value\":"+String(PrivCommRxBuffer[12])
                    +"}}"
                );
// end experimental
#endif
                sendTime(0xA4, 0x01, 8, seq);  // send ack
                break;

            case 0x05:
                sprintf(str, "%c%c:%c%c:%c%c:%c%c",
                        PrivCommRxBuffer[PayloadStart + 0],
                        PrivCommRxBuffer[PayloadStart + 1],
                        PrivCommRxBuffer[PayloadStart + 2],
                        PrivCommRxBuffer[PayloadStart + 3],
                        PrivCommRxBuffer[PayloadStart + 4],
                        PrivCommRxBuffer[PayloadStart + 5],
                        PrivCommRxBuffer[PayloadStart + 6],
                        PrivCommRxBuffer[PayloadStart + 7]
                );
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - RFID card detected. ID: %s", cmd, seq, len, crc, str);
                sendCommand(CardAuthAckA5, sizeof(CardAuthAckA5), seq, false); // offline charging cfgAllowOfflineTxForUnknownId Disabled
                api.callCommand("nfc/inject_tag", Config::ConfUpdateObject{{
                    {"tag_type", 0},
                    {"tag_id", String(str)}
                }});
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
                    sendCommand(StartChargingA7, sizeof(StartChargingA7), seq, false);
#ifdef EXPERIMENTAL
// experimental:
                    send_http(String(",\"type\":\"en+07\",\"data\":{")
                      +"\"transaction\":"+String(transactionNumber)
                      +",\"meterStart\":"+String(PrivCommRxBuffer[79]+256*PrivCommRxBuffer[80])  // status ?
                      //+",\"startMode\":"+String(PrivCommRxBuffer[72])
                      +"}}"
                    );
// end experimental
#endif
                } else {
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Stop charging approval", cmd, seq, len, crc);
                    sendCommand(StopChargingA7, sizeof(StopChargingA7), seq, false);
                }
                break;

            case 0x08: /* ClockAlignedDataInterval */
// W (2021-08-07 07:55:19) [PRIV_COMM, 1764]: Tx(cmd_A8 len:21) :  FA 03 00 00 A8 25 0B 00 40 15 08 07 07 37 13 00 00|[2021-08-07 07:55:18] Rx(cmd_A8 len:21) : FA 03 00 00 A8 25 0B 00 40 15 08 07 07 37 13 00 00 00 00 1B BE 00 00 1B BE                                                                                                      |[2021-08-07 07:55:18] cmd_A8 [privCommCmdA8RTDataAck]!
//D (2021-08-07 07:55:19) [OCPP_SRV, 3550]: ocpp_sevice_ntc_evt: 9, chan:0,sts:8                                    |[2021-08-07 07:55:18] charger A8 settime:21-8-7 7:55:19
//D (2021-08-07 07:55:19) [OCPP_SRV, 3031]: startMode(0:app 1:card 2:vin):1, stopreson:Remote timestamp:2021-08-07T0|[2021-08-07 07:55:19] [comm] cmd03 cpNowSts=0, gunNowSts=1,gunPreSts=0,chargerreson=6
//7:55:17Z idTag:50a674e1                                                                                           |[2021-08-07 07:55:19] Tx(cmd_03 len:24) : FA 03 00 00 03 18 0E 00 80 01 01 06 00 00 00 00 [2021-08-07 07:55:19] [
                if (PrivCommRxBuffer[77] < 10) {  // statistics
                    // TODO is it true that PrivCommRxBuffer[77] is the evseStatus?
                    evseStatus = PrivCommRxBuffer[77];
                    update_evseStatus(evseStatus);
                    if(ac011k_hardware.config.get("verbose_communication")->asBool()) { // we may be silent for the heartbeat //TODO show it at first and after an hour?
                        logger.printfln("Rx cmd_%.2X %dWh\t%d\t%dWh\t%d\t%d\t%d\t%dW\t%d\t%.1fV\t%.1fV\t%.1fV\t%.1fA\t%.1fA\t%.1fA\t%d minutes",
                            cmd,
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
                    }
                    // push values to the meter
                    // voltages
                    meter.updateMeterAllValues(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1, float(PrivCommRxBuffer[100]+256*PrivCommRxBuffer[101])/10);
                    meter.updateMeterAllValues(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L2, float(PrivCommRxBuffer[102]+256*PrivCommRxBuffer[103])/10);
                    meter.updateMeterAllValues(METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L3, float(PrivCommRxBuffer[104]+256*PrivCommRxBuffer[105])/10);
                    // current
                    meter.updateMeterAllValues(METER_ALL_VALUES_CURRENT_L1_A, float(PrivCommRxBuffer[106]+256*PrivCommRxBuffer[107])/10);
                    meter.updateMeterAllValues(METER_ALL_VALUES_CURRENT_L2_A, float(PrivCommRxBuffer[108]+256*PrivCommRxBuffer[109])/10);
                    meter.updateMeterAllValues(METER_ALL_VALUES_CURRENT_L3_A, float(PrivCommRxBuffer[110]+256*PrivCommRxBuffer[111])/10);

                    // meter power
                    meter.updateMeterValues(
                              (PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97]),             // charging power W  (power)
                              float(PrivCommRxBuffer[84]+256*PrivCommRxBuffer[85])/1000, // charged energy Wh (energy_rel)
                              float(PrivCommRxBuffer[88]+256*PrivCommRxBuffer[89])/1000  // charged energy Wh (energy_abs)
                              );
                    /*
                    meter.updateMeterAllValues(i, all_values_update.get(i)->asFloat());
                    */

                    /* fill phases status values */
                    /* consider a voltage > 70V as real, below that it is probably a faulty reading (we tried with 10V, which still was too low as a threshold) */
                    if ((PrivCommRxBuffer[100]+256*PrivCommRxBuffer[101]) > 700) { phases_connected[0] = true; } // L1 plug voltage
                    if ((PrivCommRxBuffer[102]+256*PrivCommRxBuffer[103]) > 700) { phases_connected[1] = true; } // L2 plug voltage
                    if ((PrivCommRxBuffer[104]+256*PrivCommRxBuffer[105]) > 700) { phases_connected[2] = true; } // L3 plug voltage
                    meter.updateMeterPhases(phases_connected, phases_active);

                    /* set charging_time */
                    evse.evse_low_level_state.get("charging_time")->updateUint((PrivCommRxBuffer[113]+256*PrivCommRxBuffer[114])*60*1000);   // in ms
                              
#ifdef EXPERIMENTAL
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
#endif
                } else {
                    logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - type:%.2X", cmd, seq, len, crc, PrivCommRxBuffer[77]);
                    if (PrivCommRxBuffer[77] == 0x10) {  // RFID card
                        String rfid = "";
                        for (int i=0; i<8; i++) {rfid += PrivCommRxBuffer[40 + i];}  // Card number in bytes 40..47
                        logger.printfln("RFID Card %s", rfid.c_str());
                    }
                }
                sendTime(0xA8, 0x40, 12, seq); // send ack
                break;

            case 0x09:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Charging stop reason: %d - %s",
                    cmd, seq, len, crc,
                    PrivCommRxBuffer[77], PrivCommRxBuffer[77]<=3 ? stop_reason_text[PrivCommRxBuffer[77]] : stop_reason_text[0]);  // "stopreson": 1 = Remote, 3 = EVDisconnected
                /* warum sind start zeit und stop zeit gleich? */
                logger.printfln("start:%s stop:%s meter:%dWh value1:%d value2:%d value3:%d",
                    timeStr(&PrivCommRxBuffer[80]),
                    timeStr(&PrivCommRxBuffer[86]),
                    PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97],
                    PrivCommRxBuffer[78]+256*PrivCommRxBuffer[79],
                    PrivCommRxBuffer[92]+256*PrivCommRxBuffer[93],
                    PrivCommRxBuffer[94]+256*PrivCommRxBuffer[95]);
#ifdef EXPERIMENTAL
// experimental:
                send_http(String(",\"type\":\"en+09\",\"data\":{")
                    +"\"transaction\":"+String(transactionNumber)
                    +",\"meterStop\":"+String(PrivCommRxBuffer[96]+256*PrivCommRxBuffer[97])  // status
                    +",\"stopReason\":"+String(PrivCommRxBuffer[77])
                    +"}}"
                );
// end experimental
#endif
                sendCommand(TransactionAck, sizeof(TransactionAck), seq, false);
                break;

            case 0x0A:
                switch( PrivCommRxBuffer[9] ) { // 9: answertype
                    case GD_0A_TIME_ANSWER: // time answer
                        switch( PrivCommRxBuffer[8] ) {
                            case GD_GET_RTC_ANSWER:
                                if (!clock_synced(&tv_now) || (ntp.state.get("time")->asUint() == 0)) {
                                    struct timeval tv_rtc;
                                    struct tm timeinfo;
                                    timeinfo.tm_year  = PrivCommRxBuffer[PayloadStart + 4] + 100;
                                    timeinfo.tm_mon   = PrivCommRxBuffer[PayloadStart + 5] - 1;
                                    timeinfo.tm_mday  = PrivCommRxBuffer[PayloadStart + 6];
                                    timeinfo.tm_hour  = PrivCommRxBuffer[PayloadStart + 7];
                                    timeinfo.tm_min   = PrivCommRxBuffer[PayloadStart + 8];
                                    timeinfo.tm_sec   = PrivCommRxBuffer[PayloadStart + 9];
                                    timeinfo.tm_isdst = -1;
                                    tv_rtc.tv_sec = mktime(&timeinfo) - GetTimeZoneOffset();
                                    tv_rtc.tv_usec = 0;

                                    if (tv_rtc.tv_sec > build_timestamp()) {
                                        rtc.set_synced();
                                        if (abs(tv_now.tv_sec - tv_rtc.tv_sec) > 1) {
                                            settimeofday(&tv_rtc, nullptr);
                                            logger.printfln("Got a more precise time from RTC (%s UTC), setting local time in respect to timezone (%s).", timeStr(&PrivCommRxBuffer[PayloadStart + 4]), ntp.config.get("timezone")->asEphemeralCStr());
                                        }
                                    } else {
                                        logger.printfln("Got time from RTC (%s UTC), but time is before build time, ignoring.", timeStr(&PrivCommRxBuffer[PayloadStart + 4]));
                                    }
                                }
                                break;
                            case GD_SET_RTC_ANSWER:
                            case 0x16:
                            case 0x17:
                                //logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Set Time done", cmd, seq, len, crc);
                                break;
                            default:
                                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X -  Time answer, but I don't know what %.2X means.", cmd, seq, len, crc, PrivCommRxBuffer[8]);
                                get_hex_privcomm_line(PrivCommRxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
                                break;
                        }
                        break;
                    case 0x08: // answer to set hb timeout
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Heartbeat Timeout: %ds", cmd, seq, len, crc, PrivCommRxBuffer[12]+256*PrivCommRxBuffer[13]);
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
                    case 0x3E: // 
                        logger.printfln("ClockAlignedDataInterval is now: %ds", PrivCommRxBuffer[12]+256*PrivCommRxBuffer[13]);
                        break;
                    case 0x3F: // 
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - cmdAAInit7Ack", cmd, seq, len, crc);
                        break;
                    default:
                        logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X -  I don't know what %.2X means.", cmd, seq, len, crc, PrivCommRxBuffer[9]);
                        get_hex_privcomm_line(PrivCommRxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
                        break;
                }  //switch cmdAA answer processing
                break;

            case 0x0B:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - update reply: %.2X%.2X%.2X%.2X  %d (%s) - %s, fw update running: %s", cmd, seq, len, crc, FlashVerify[3], FlashVerify[4], FlashVerify[5], FlashVerify[6], PrivCommRxBuffer[10], cmd_0B_text[PrivCommRxBuffer[10]], PrivCommRxBuffer[12]==0 ?"success":"failure", this->firmware_update_running==true ?"true":"false");
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
                        case 4: // verify
                            //if (*((unsigned int) FlashVerify[3]) == 0x030800FE) {
                            if (FlashVerify[3] == 0x03 && FlashVerify[4] == 0x08 && FlashVerify[5] == 0x00 && FlashVerify[6] == 0xFE) {
                                logger.printfln("   finished flashing, getting the GD chip back into app mode");
                                sendCommand(EnterAppMode, sizeof(EnterAppMode), sendSequenceNumber++);
                            }
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
                // as of now we set the value when we send the setting to the GD
                // set the value here? is the value in the ack?
                // evse_state.get("allowed_charging_current")->updateUint(allowed_charging_current);
                break;

            case 0x0E: // ChargingParameterRpt
                if(ac011k_hardware.config.get("verbose_communication")->asBool() 
                    // report on changes
                    || (evse.evse_low_level_state.get("cp_pwm_duty_cycle")->asUint() != PrivCommRxBuffer[17]+256*PrivCommRxBuffer[18]) //duty
                    || (evse.evse_low_level_state.get("adc_values")->get(6)->asUint() != PrivCommRxBuffer[19]+256*PrivCommRxBuffer[20])) { //cpVolt
                    logger.printfln("Charging parameter report - duty:%d cpVolt:%d power factors:%d/%d/%d %d offset0:%d offset1:%d leakcurr:%d AMBTemp:%d lock:%d",
                        PrivCommRxBuffer[17]+256*PrivCommRxBuffer[18], // duty
                        PrivCommRxBuffer[19]+256*PrivCommRxBuffer[20], // cpVolt
                        PrivCommRxBuffer[ 9]+256*PrivCommRxBuffer[10], // power factor 1
                        PrivCommRxBuffer[11]+256*PrivCommRxBuffer[12], // power factor 2
                        PrivCommRxBuffer[13]+256*PrivCommRxBuffer[14], // power factor 3
                        PrivCommRxBuffer[15]+256*PrivCommRxBuffer[16], // power factor total
                        PrivCommRxBuffer[55]+256*PrivCommRxBuffer[56], // offset0
                        PrivCommRxBuffer[57]+256*PrivCommRxBuffer[58], // offset1
                        PrivCommRxBuffer[59]+256*PrivCommRxBuffer[60], // leakcurr
                        PrivCommRxBuffer[61]+256*PrivCommRxBuffer[62], // AMBTemp
                        PrivCommRxBuffer[63]);                         // lock
                }
                evse.evse_low_level_state.get("cp_pwm_duty_cycle")->updateUint(PrivCommRxBuffer[17]+256*PrivCommRxBuffer[18]); //duty
                evse.evse_low_level_state.get("adc_values")->get(6)->updateUint(PrivCommRxBuffer[19]+256*PrivCommRxBuffer[20]); //cpVolt
#ifdef EXPERIMENTAL
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
#endif
                break;

            case 0x0F:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - Charging limit request, answer: %dA", cmd, seq, len, crc, allowed_charging_current);
                sendChargingLimit1(allowed_charging_current, seq);
                // as of now we set the value when we send the setting to the GD
                // set the value here? is the value in the ack?
                // evse_state.get("allowed_charging_current")->updateUint(allowed_charging_current);
                break;

            default:
                logger.printfln("Rx cmd_%.2X seq:%.2X len:%d crc:%.4X - I don't know what to do about it.", cmd, seq, len, crc);
                get_hex_privcomm_line(PrivCommRxBuffer); // PrivCommHexBuffer now holds the hex representation of the buffer
                break;
        }  //switch process cmd
        cmd_to_process = false;
    }

    evse.evse_state.get("time_since_state_change")->updateUint(millis() - evse.evse_state.get("last_state_change")->asUint());
    evse.evse_low_level_state.get("time_since_state_change")->updateUint(evse.evse_state.get("time_since_state_change")->asUint());

    /*
     * this is probably already (and better) taken care of via the ClockAlignedDataInterval setting
     * if charging and x seconds elapsed, trigger 0x0E via
     * sendTime(0xA8, 0x40, 12, seq); // Trigger Message: MeterValues
     *
     */
    
    //resend flash commands if needed
    if(this->firmware_update_running && flash_seq == PrivCommTxBuffer[5] && !ready_for_next_chunk && deadline_elapsed(last_flash + 2000)) {
        last_flash = millis();
        logger.printfln("resend the last chunk fseq: %d, seq: %d rfnc: %s", flash_seq, PrivCommTxBuffer[5], ready_for_next_chunk?"true":"false");
        sendCommand(FlashVerify, MAXLENGTH+11, sendSequenceNumber++); // next chunk (11 bytes header) 
    }


#ifdef EXPERIMENTAL
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

                if (receiveCommandBuffer[1] == '1') sendChargingLimit1(limit, sendSequenceNumber++);  // working for all except for 1.1.212
                else if (receiveCommandBuffer[1] == '2') sendChargingLimit2(limit, sendSequenceNumber++);
                else if (receiveCommandBuffer[1] == '3') sendChargingLimit3(limit, sendSequenceNumber++);  // working for 1.1.212

                if (receiveCommandBuffer[1] == 'P') {
                    ChargingLimit3[74] = 1;
                    sendChargingLimit3(limit, sendSequenceNumber++);  // e.g. LPA for 1 phase, 10 Ampere. Sadly not working here :-( 
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
#endif
}

void AC011K::register_urls()
{
    evse.register_urls();
#ifdef GD_FLASH
    server.on("/evse/reflash", HTTP_PUT, [this](WebServerRequest request){
        if (update_aborted)
            return request.unsafe_ResponseAlreadySent(); // Already sent in upload callback.
        this->firmware_update_running = false;
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            return request.unsafe_ResponseAlreadySent(); // Already sent in upload callback.
        }
        /* request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK"); */
        return request.send(200, "text/plain", "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            this->firmware_update_running = false;
            return false;
        }
        this->firmware_update_running = true;
        logger.printfln("/evse/reflash %d (%d)", index, len);
        return handle_update_chunk(3, request, index, data, len);
    });
#endif

    task_scheduler.scheduleWithFixedDelay([this]() {
        GetRTC();
    }, 1000 * 60 * 10, 1000 * 60 * 10);
}
