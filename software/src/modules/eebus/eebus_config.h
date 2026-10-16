/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#ifdef EEBUS_CONFIG_H
#error "Overlapping EEBUS config header includes! include eebus_config_end.h at the end of headers including eebus_config.h!"
#endif

#define EEBUS_CONFIG_H

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wundef"
#endif

// If the EVSE module is available we are probably a charger.
#if MODULE_EVSE_COMMON_AVAILABLE()
#define EEBUS_MODE_EVSE() 1
#define EEBUS_MODE_EM() 0
#define EEBUS_DEVICE_TYPE "ChargingStation" // The device type as defined in EEBUS SPINE TS ResourceSpecification. Can be freely defined
#define EEBUS_DEVICE_CATEGORY "3"           // [SRIP-220/15] Device category: 3 = E-mobility related device
#else
#define EEBUS_MODE_EVSE() 0
#define EEBUS_MODE_EM() 1
#define EEBUS_DEVICE_TYPE "EnergyManagementSystem" // The device type as defined in EEBUS SPINE TS ResourceSpecification. Can be freely defined
#define EEBUS_DEVICE_CATEGORY "2"                  // [SRIP-220/15] Device category: 2 = Energy Management System
#endif

#define EEBUS_DEV_TEST_ENABLE() 0 // Enable to set the eebus system into test mode. This will update all enabled usecases with random data to see if the appropriate functions work properly and communication works.

// If we are in testmode enable only the tested usecases, otherwise enable the usecases either for energy manager or evse
#if EEBUS_DEV_TEST_ENABLE()

#define EEBUS_ENABLE_CEVC_USECASE()   0
#define EEBUS_ENABLE_EVCC_USECASE()   0
#define EEBUS_ENABLE_EVCEM_USECASE()  0
#define EEBUS_ENABLE_EVCS_USECASE()   0
#define EEBUS_ENABLE_EVSECC_USECASE() 0
#define EEBUS_ENABLE_LPC_USECASE()    1
#define EEBUS_ENABLE_LPP_USECASE()    0
#define EEBUS_ENABLE_MGCP_USECASE()   1
#define EEBUS_ENABLE_MPC_USECASE()    0
#define EEBUS_ENABLE_OPEV_USECASE()   0

#else

// What usecases are enabled depends on if this is a WARP Charger or an Energy Manager

#if EEBUS_MODE_EVSE()

#define EEBUS_ENABLE_CEVC_USECASE()   0
#define EEBUS_ENABLE_EVCC_USECASE()   1
#define EEBUS_ENABLE_EVCEM_USECASE()  1
#define EEBUS_ENABLE_EVCS_USECASE()   0
#define EEBUS_ENABLE_EVSECC_USECASE() 1
#define EEBUS_ENABLE_LPC_USECASE()    1
#define EEBUS_ENABLE_LPP_USECASE()    0
#define EEBUS_ENABLE_MGCP_USECASE()   0
#define EEBUS_ENABLE_MPC_USECASE()    1
#define EEBUS_ENABLE_OPEV_USECASE()   0

#endif

#if EEBUS_MODE_EM()

#define EEBUS_ENABLE_CEVC_USECASE()   0
#define EEBUS_ENABLE_EVCC_USECASE()   0
#define EEBUS_ENABLE_EVCEM_USECASE()  0
#define EEBUS_ENABLE_EVCS_USECASE()   0
#define EEBUS_ENABLE_EVSECC_USECASE() 0
#define EEBUS_ENABLE_LPC_USECASE()    1
#define EEBUS_ENABLE_LPP_USECASE()    0
#define EEBUS_ENABLE_MGCP_USECASE()   1
#define EEBUS_ENABLE_MPC_USECASE()    0
#define EEBUS_ENABLE_OPEV_USECASE()   0

#endif

#endif

#define EEBUS_PEER_FILE "/eebus/peers"
#define EEBUS_FAILSAFE_CONFIG_PATH "eebus/failsafe"
#define EEBUS_MAX_PEER_REMEMBERED 4 // How many ship peers configured to be remembered
#define EEBUS_MAX_PEER_DISCOVERED 4 // How many ship peers can be discovered via mDNS

#define EEBUS_SUPPORTED_SPINE_VERSION "1.3.0" // The supported SPINE version for EEBus
#define EEBUS_NO_VALUE INT32_MIN

#define EEBUS_SHIP_AUTOCONNECT() 1 // If defined, the EEBus device will automatically connect to discovered and trusted SHIP peers. This is currently in testing
#define EEBUS_SHIP_AUTOCONNECT_INTERVAL 300_s

#define EEBUS_TRACE_SUPER_VERBOSE() 0 // If enabled, will log much more information to the tracelog which may help in debugging SHIP/SPINE/EEBUS issues. Off by default as it fills up the tracelog very quickly

// Configuration related to the LPC usecases

// Disable if subscription functionalities shall not be used
#define EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS true

// The power consumption limit at startup in w. Should be the maximum limit of the Warp Charger. Is also used to tell the Energy Broker the maximum consumption limit of the device
#define EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION 22000

// Enable LPC heartbeat
#define EEBUS_LPC_ENABLE_HEARTBEAT true
#define EEBUS_LPC_HEARTBEAT_INTERVAL 30_s // Must be more than 10 seconds and less or equal to 60 seconds
