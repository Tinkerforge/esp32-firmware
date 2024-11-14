# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-02-27.      #
#                                                           #
# Python Bindings Version 2.1.31                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

GetChibiErrorLog = namedtuple('ChibiErrorLog', ['underrun', 'crc_error', 'no_ack', 'overflow'])
GetRS485Configuration = namedtuple('RS485Configuration', ['speed', 'parity', 'stopbits'])
GetWifiConfiguration = namedtuple('WifiConfiguration', ['ssid', 'connection', 'ip', 'subnet_mask', 'gateway', 'port'])
GetWifiEncryption = namedtuple('WifiEncryption', ['encryption', 'key', 'key_index', 'eap_options', 'ca_certificate_length', 'client_certificate_length', 'private_key_length'])
GetWifiStatus = namedtuple('WifiStatus', ['mac_address', 'bssid', 'channel', 'rssi', 'ip', 'subnet_mask', 'gateway', 'rx_count', 'tx_count', 'state'])
GetWifiCertificate = namedtuple('WifiCertificate', ['data', 'data_length'])
GetWifiBufferInfo = namedtuple('WifiBufferInfo', ['overflow', 'low_watermark', 'used'])
GetStackCurrentCallbackThreshold = namedtuple('StackCurrentCallbackThreshold', ['option', 'min', 'max'])
GetStackVoltageCallbackThreshold = namedtuple('StackVoltageCallbackThreshold', ['option', 'min', 'max'])
GetUSBVoltageCallbackThreshold = namedtuple('USBVoltageCallbackThreshold', ['option', 'min', 'max'])
GetEthernetConfiguration = namedtuple('EthernetConfiguration', ['connection', 'ip', 'subnet_mask', 'gateway', 'port'])
GetEthernetStatus = namedtuple('EthernetStatus', ['mac_address', 'ip', 'subnet_mask', 'gateway', 'rx_count', 'tx_count', 'hostname'])
GetEthernetWebsocketConfiguration = namedtuple('EthernetWebsocketConfiguration', ['sockets', 'port'])
ReadWifi2SerialPort = namedtuple('ReadWifi2SerialPort', ['data', 'result'])
GetWifi2Configuration = namedtuple('Wifi2Configuration', ['port', 'websocket_port', 'website_port', 'phy_mode', 'sleep_mode', 'website'])
GetWifi2Status = namedtuple('Wifi2Status', ['client_enabled', 'client_status', 'client_ip', 'client_subnet_mask', 'client_gateway', 'client_mac_address', 'client_rx_count', 'client_tx_count', 'client_rssi', 'ap_enabled', 'ap_ip', 'ap_subnet_mask', 'ap_gateway', 'ap_mac_address', 'ap_rx_count', 'ap_tx_count', 'ap_connected_count'])
GetWifi2ClientConfiguration = namedtuple('Wifi2ClientConfiguration', ['enable', 'ssid', 'ip', 'subnet_mask', 'gateway', 'mac_address', 'bssid'])
GetWifi2APConfiguration = namedtuple('Wifi2APConfiguration', ['enable', 'ssid', 'ip', 'subnet_mask', 'gateway', 'encryption', 'hidden', 'channel', 'mac_address'])
GetWifi2MeshConfiguration = namedtuple('Wifi2MeshConfiguration', ['enable', 'root_ip', 'root_subnet_mask', 'root_gateway', 'router_bssid', 'group_id', 'group_ssid_prefix', 'gateway_ip', 'gateway_port'])
GetWifi2MeshCommonStatus = namedtuple('Wifi2MeshCommonStatus', ['status', 'root_node', 'root_candidate', 'connected_nodes', 'rx_count', 'tx_count'])
GetWifi2MeshClientStatus = namedtuple('Wifi2MeshClientStatus', ['hostname', 'ip', 'subnet_mask', 'gateway', 'mac_address'])
GetWifi2MeshAPStatus = namedtuple('Wifi2MeshAPStatus', ['ssid', 'ip', 'subnet_mask', 'gateway', 'mac_address'])
SetBrickletXMCFlashConfig = namedtuple('SetBrickletXMCFlashConfig', ['return_value', 'return_data'])
GetSPITFPBaudrateConfig = namedtuple('SPITFPBaudrateConfig', ['enable_dynamic_baudrate', 'minimum_dynamic_baudrate'])
GetSPITFPErrorCount = namedtuple('SPITFPErrorCount', ['error_count_ack_checksum', 'error_count_message_checksum', 'error_count_frame', 'error_count_overflow'])
GetProtocol1BrickletName = namedtuple('Protocol1BrickletName', ['protocol_version', 'firmware_version', 'name'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickMaster(Device):
    r"""
    Basis to build stacks and has 4 Bricklet ports
    """

    DEVICE_IDENTIFIER = 13
    DEVICE_DISPLAY_NAME = 'Master Brick'
    DEVICE_URL_PART = 'master' # internal

    CALLBACK_STACK_CURRENT = 59
    CALLBACK_STACK_VOLTAGE = 60
    CALLBACK_USB_VOLTAGE = 61
    CALLBACK_STACK_CURRENT_REACHED = 62
    CALLBACK_STACK_VOLTAGE_REACHED = 63
    CALLBACK_USB_VOLTAGE_REACHED = 64


    FUNCTION_GET_STACK_VOLTAGE = 1
    FUNCTION_GET_STACK_CURRENT = 2
    FUNCTION_SET_EXTENSION_TYPE = 3
    FUNCTION_GET_EXTENSION_TYPE = 4
    FUNCTION_IS_CHIBI_PRESENT = 5
    FUNCTION_SET_CHIBI_ADDRESS = 6
    FUNCTION_GET_CHIBI_ADDRESS = 7
    FUNCTION_SET_CHIBI_MASTER_ADDRESS = 8
    FUNCTION_GET_CHIBI_MASTER_ADDRESS = 9
    FUNCTION_SET_CHIBI_SLAVE_ADDRESS = 10
    FUNCTION_GET_CHIBI_SLAVE_ADDRESS = 11
    FUNCTION_GET_CHIBI_SIGNAL_STRENGTH = 12
    FUNCTION_GET_CHIBI_ERROR_LOG = 13
    FUNCTION_SET_CHIBI_FREQUENCY = 14
    FUNCTION_GET_CHIBI_FREQUENCY = 15
    FUNCTION_SET_CHIBI_CHANNEL = 16
    FUNCTION_GET_CHIBI_CHANNEL = 17
    FUNCTION_IS_RS485_PRESENT = 18
    FUNCTION_SET_RS485_ADDRESS = 19
    FUNCTION_GET_RS485_ADDRESS = 20
    FUNCTION_SET_RS485_SLAVE_ADDRESS = 21
    FUNCTION_GET_RS485_SLAVE_ADDRESS = 22
    FUNCTION_GET_RS485_ERROR_LOG = 23
    FUNCTION_SET_RS485_CONFIGURATION = 24
    FUNCTION_GET_RS485_CONFIGURATION = 25
    FUNCTION_IS_WIFI_PRESENT = 26
    FUNCTION_SET_WIFI_CONFIGURATION = 27
    FUNCTION_GET_WIFI_CONFIGURATION = 28
    FUNCTION_SET_WIFI_ENCRYPTION = 29
    FUNCTION_GET_WIFI_ENCRYPTION = 30
    FUNCTION_GET_WIFI_STATUS = 31
    FUNCTION_REFRESH_WIFI_STATUS = 32
    FUNCTION_SET_WIFI_CERTIFICATE = 33
    FUNCTION_GET_WIFI_CERTIFICATE = 34
    FUNCTION_SET_WIFI_POWER_MODE = 35
    FUNCTION_GET_WIFI_POWER_MODE = 36
    FUNCTION_GET_WIFI_BUFFER_INFO = 37
    FUNCTION_SET_WIFI_REGULATORY_DOMAIN = 38
    FUNCTION_GET_WIFI_REGULATORY_DOMAIN = 39
    FUNCTION_GET_USB_VOLTAGE = 40
    FUNCTION_SET_LONG_WIFI_KEY = 41
    FUNCTION_GET_LONG_WIFI_KEY = 42
    FUNCTION_SET_WIFI_HOSTNAME = 43
    FUNCTION_GET_WIFI_HOSTNAME = 44
    FUNCTION_SET_STACK_CURRENT_CALLBACK_PERIOD = 45
    FUNCTION_GET_STACK_CURRENT_CALLBACK_PERIOD = 46
    FUNCTION_SET_STACK_VOLTAGE_CALLBACK_PERIOD = 47
    FUNCTION_GET_STACK_VOLTAGE_CALLBACK_PERIOD = 48
    FUNCTION_SET_USB_VOLTAGE_CALLBACK_PERIOD = 49
    FUNCTION_GET_USB_VOLTAGE_CALLBACK_PERIOD = 50
    FUNCTION_SET_STACK_CURRENT_CALLBACK_THRESHOLD = 51
    FUNCTION_GET_STACK_CURRENT_CALLBACK_THRESHOLD = 52
    FUNCTION_SET_STACK_VOLTAGE_CALLBACK_THRESHOLD = 53
    FUNCTION_GET_STACK_VOLTAGE_CALLBACK_THRESHOLD = 54
    FUNCTION_SET_USB_VOLTAGE_CALLBACK_THRESHOLD = 55
    FUNCTION_GET_USB_VOLTAGE_CALLBACK_THRESHOLD = 56
    FUNCTION_SET_DEBOUNCE_PERIOD = 57
    FUNCTION_GET_DEBOUNCE_PERIOD = 58
    FUNCTION_IS_ETHERNET_PRESENT = 65
    FUNCTION_SET_ETHERNET_CONFIGURATION = 66
    FUNCTION_GET_ETHERNET_CONFIGURATION = 67
    FUNCTION_GET_ETHERNET_STATUS = 68
    FUNCTION_SET_ETHERNET_HOSTNAME = 69
    FUNCTION_SET_ETHERNET_MAC_ADDRESS = 70
    FUNCTION_SET_ETHERNET_WEBSOCKET_CONFIGURATION = 71
    FUNCTION_GET_ETHERNET_WEBSOCKET_CONFIGURATION = 72
    FUNCTION_SET_ETHERNET_AUTHENTICATION_SECRET = 73
    FUNCTION_GET_ETHERNET_AUTHENTICATION_SECRET = 74
    FUNCTION_SET_WIFI_AUTHENTICATION_SECRET = 75
    FUNCTION_GET_WIFI_AUTHENTICATION_SECRET = 76
    FUNCTION_GET_CONNECTION_TYPE = 77
    FUNCTION_IS_WIFI2_PRESENT = 78
    FUNCTION_START_WIFI2_BOOTLOADER = 79
    FUNCTION_WRITE_WIFI2_SERIAL_PORT = 80
    FUNCTION_READ_WIFI2_SERIAL_PORT = 81
    FUNCTION_SET_WIFI2_AUTHENTICATION_SECRET = 82
    FUNCTION_GET_WIFI2_AUTHENTICATION_SECRET = 83
    FUNCTION_SET_WIFI2_CONFIGURATION = 84
    FUNCTION_GET_WIFI2_CONFIGURATION = 85
    FUNCTION_GET_WIFI2_STATUS = 86
    FUNCTION_SET_WIFI2_CLIENT_CONFIGURATION = 87
    FUNCTION_GET_WIFI2_CLIENT_CONFIGURATION = 88
    FUNCTION_SET_WIFI2_CLIENT_HOSTNAME = 89
    FUNCTION_GET_WIFI2_CLIENT_HOSTNAME = 90
    FUNCTION_SET_WIFI2_CLIENT_PASSWORD = 91
    FUNCTION_GET_WIFI2_CLIENT_PASSWORD = 92
    FUNCTION_SET_WIFI2_AP_CONFIGURATION = 93
    FUNCTION_GET_WIFI2_AP_CONFIGURATION = 94
    FUNCTION_SET_WIFI2_AP_PASSWORD = 95
    FUNCTION_GET_WIFI2_AP_PASSWORD = 96
    FUNCTION_SAVE_WIFI2_CONFIGURATION = 97
    FUNCTION_GET_WIFI2_FIRMWARE_VERSION = 98
    FUNCTION_ENABLE_WIFI2_STATUS_LED = 99
    FUNCTION_DISABLE_WIFI2_STATUS_LED = 100
    FUNCTION_IS_WIFI2_STATUS_LED_ENABLED = 101
    FUNCTION_SET_WIFI2_MESH_CONFIGURATION = 102
    FUNCTION_GET_WIFI2_MESH_CONFIGURATION = 103
    FUNCTION_SET_WIFI2_MESH_ROUTER_SSID = 104
    FUNCTION_GET_WIFI2_MESH_ROUTER_SSID = 105
    FUNCTION_SET_WIFI2_MESH_ROUTER_PASSWORD = 106
    FUNCTION_GET_WIFI2_MESH_ROUTER_PASSWORD = 107
    FUNCTION_GET_WIFI2_MESH_COMMON_STATUS = 108
    FUNCTION_GET_WIFI2_MESH_CLIENT_STATUS = 109
    FUNCTION_GET_WIFI2_MESH_AP_STATUS = 110
    FUNCTION_SET_BRICKLET_XMC_FLASH_CONFIG = 111
    FUNCTION_SET_BRICKLET_XMC_FLASH_DATA = 112
    FUNCTION_SET_BRICKLETS_ENABLED = 113
    FUNCTION_GET_BRICKLETS_ENABLED = 114
    FUNCTION_SET_SPITFP_BAUDRATE_CONFIG = 231
    FUNCTION_GET_SPITFP_BAUDRATE_CONFIG = 232
    FUNCTION_GET_SEND_TIMEOUT_COUNT = 233
    FUNCTION_SET_SPITFP_BAUDRATE = 234
    FUNCTION_GET_SPITFP_BAUDRATE = 235
    FUNCTION_GET_SPITFP_ERROR_COUNT = 237
    FUNCTION_ENABLE_STATUS_LED = 238
    FUNCTION_DISABLE_STATUS_LED = 239
    FUNCTION_IS_STATUS_LED_ENABLED = 240
    FUNCTION_GET_PROTOCOL1_BRICKLET_NAME = 241
    FUNCTION_GET_CHIP_TEMPERATURE = 242
    FUNCTION_RESET = 243
    FUNCTION_WRITE_BRICKLET_PLUGIN = 246
    FUNCTION_READ_BRICKLET_PLUGIN = 247
    FUNCTION_GET_IDENTITY = 255

    EXTENSION_TYPE_CHIBI = 1
    EXTENSION_TYPE_RS485 = 2
    EXTENSION_TYPE_WIFI = 3
    EXTENSION_TYPE_ETHERNET = 4
    EXTENSION_TYPE_WIFI2 = 5
    CHIBI_FREQUENCY_OQPSK_868_MHZ = 0
    CHIBI_FREQUENCY_OQPSK_915_MHZ = 1
    CHIBI_FREQUENCY_OQPSK_780_MHZ = 2
    CHIBI_FREQUENCY_BPSK40_915_MHZ = 3
    RS485_PARITY_NONE = 'n'
    RS485_PARITY_EVEN = 'e'
    RS485_PARITY_ODD = 'o'
    WIFI_CONNECTION_DHCP = 0
    WIFI_CONNECTION_STATIC_IP = 1
    WIFI_CONNECTION_ACCESS_POINT_DHCP = 2
    WIFI_CONNECTION_ACCESS_POINT_STATIC_IP = 3
    WIFI_CONNECTION_AD_HOC_DHCP = 4
    WIFI_CONNECTION_AD_HOC_STATIC_IP = 5
    WIFI_ENCRYPTION_WPA_WPA2 = 0
    WIFI_ENCRYPTION_WPA_ENTERPRISE = 1
    WIFI_ENCRYPTION_WEP = 2
    WIFI_ENCRYPTION_NO_ENCRYPTION = 3
    WIFI_EAP_OPTION_OUTER_AUTH_EAP_FAST = 0
    WIFI_EAP_OPTION_OUTER_AUTH_EAP_TLS = 1
    WIFI_EAP_OPTION_OUTER_AUTH_EAP_TTLS = 2
    WIFI_EAP_OPTION_OUTER_AUTH_EAP_PEAP = 3
    WIFI_EAP_OPTION_INNER_AUTH_EAP_MSCHAP = 0
    WIFI_EAP_OPTION_INNER_AUTH_EAP_GTC = 4
    WIFI_EAP_OPTION_CERT_TYPE_CA_CERT = 0
    WIFI_EAP_OPTION_CERT_TYPE_CLIENT_CERT = 8
    WIFI_EAP_OPTION_CERT_TYPE_PRIVATE_KEY = 16
    WIFI_STATE_DISASSOCIATED = 0
    WIFI_STATE_ASSOCIATED = 1
    WIFI_STATE_ASSOCIATING = 2
    WIFI_STATE_ERROR = 3
    WIFI_STATE_NOT_INITIALIZED_YET = 255
    WIFI_POWER_MODE_FULL_SPEED = 0
    WIFI_POWER_MODE_LOW_POWER = 1
    WIFI_DOMAIN_CHANNEL_1TO11 = 0
    WIFI_DOMAIN_CHANNEL_1TO13 = 1
    WIFI_DOMAIN_CHANNEL_1TO14 = 2
    THRESHOLD_OPTION_OFF = 'x'
    THRESHOLD_OPTION_OUTSIDE = 'o'
    THRESHOLD_OPTION_INSIDE = 'i'
    THRESHOLD_OPTION_SMALLER = '<'
    THRESHOLD_OPTION_GREATER = '>'
    ETHERNET_CONNECTION_DHCP = 0
    ETHERNET_CONNECTION_STATIC_IP = 1
    CONNECTION_TYPE_NONE = 0
    CONNECTION_TYPE_USB = 1
    CONNECTION_TYPE_SPI_STACK = 2
    CONNECTION_TYPE_CHIBI = 3
    CONNECTION_TYPE_RS485 = 4
    CONNECTION_TYPE_WIFI = 5
    CONNECTION_TYPE_ETHERNET = 6
    CONNECTION_TYPE_WIFI2 = 7
    WIFI2_PHY_MODE_B = 0
    WIFI2_PHY_MODE_G = 1
    WIFI2_PHY_MODE_N = 2
    WIFI2_CLIENT_STATUS_IDLE = 0
    WIFI2_CLIENT_STATUS_CONNECTING = 1
    WIFI2_CLIENT_STATUS_WRONG_PASSWORD = 2
    WIFI2_CLIENT_STATUS_NO_AP_FOUND = 3
    WIFI2_CLIENT_STATUS_CONNECT_FAILED = 4
    WIFI2_CLIENT_STATUS_GOT_IP = 5
    WIFI2_CLIENT_STATUS_UNKNOWN = 255
    WIFI2_AP_ENCRYPTION_OPEN = 0
    WIFI2_AP_ENCRYPTION_WEP = 1
    WIFI2_AP_ENCRYPTION_WPA_PSK = 2
    WIFI2_AP_ENCRYPTION_WPA2_PSK = 3
    WIFI2_AP_ENCRYPTION_WPA_WPA2_PSK = 4
    WIFI2_MESH_STATUS_DISABLED = 0
    WIFI2_MESH_STATUS_WIFI_CONNECTING = 1
    WIFI2_MESH_STATUS_GOT_IP = 2
    WIFI2_MESH_STATUS_MESH_LOCAL = 3
    WIFI2_MESH_STATUS_MESH_ONLINE = 4
    WIFI2_MESH_STATUS_AP_AVAILABLE = 5
    WIFI2_MESH_STATUS_AP_SETUP = 6
    WIFI2_MESH_STATUS_LEAF_AVAILABLE = 7
    COMMUNICATION_METHOD_NONE = 0
    COMMUNICATION_METHOD_USB = 1
    COMMUNICATION_METHOD_SPI_STACK = 2
    COMMUNICATION_METHOD_CHIBI = 3
    COMMUNICATION_METHOD_RS485 = 4
    COMMUNICATION_METHOD_WIFI = 5
    COMMUNICATION_METHOD_ETHERNET = 6
    COMMUNICATION_METHOD_WIFI_V2 = 7

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickMaster.DEVICE_IDENTIFIER, BrickMaster.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 10)

        self.response_expected[BrickMaster.FUNCTION_GET_STACK_VOLTAGE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_STACK_CURRENT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_EXTENSION_TYPE] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_EXTENSION_TYPE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_IS_CHIBI_PRESENT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_CHIBI_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_CHIBI_MASTER_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_MASTER_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_CHIBI_SLAVE_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_SLAVE_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_SIGNAL_STRENGTH] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_ERROR_LOG] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_CHIBI_FREQUENCY] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_FREQUENCY] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_CHIBI_CHANNEL] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIBI_CHANNEL] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_IS_RS485_PRESENT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_RS485_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_RS485_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_RS485_SLAVE_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_RS485_SLAVE_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_RS485_ERROR_LOG] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_RS485_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_RS485_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_IS_WIFI_PRESENT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_ENCRYPTION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_ENCRYPTION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_STATUS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_REFRESH_WIFI_STATUS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_CERTIFICATE] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_CERTIFICATE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_POWER_MODE] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_POWER_MODE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_BUFFER_INFO] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_REGULATORY_DOMAIN] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_REGULATORY_DOMAIN] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_USB_VOLTAGE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_LONG_WIFI_KEY] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_LONG_WIFI_KEY] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_HOSTNAME] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_HOSTNAME] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_STACK_CURRENT_CALLBACK_PERIOD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_STACK_CURRENT_CALLBACK_PERIOD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_STACK_VOLTAGE_CALLBACK_PERIOD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_STACK_VOLTAGE_CALLBACK_PERIOD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_USB_VOLTAGE_CALLBACK_PERIOD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_USB_VOLTAGE_CALLBACK_PERIOD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_STACK_CURRENT_CALLBACK_THRESHOLD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_STACK_CURRENT_CALLBACK_THRESHOLD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_STACK_VOLTAGE_CALLBACK_THRESHOLD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_STACK_VOLTAGE_CALLBACK_THRESHOLD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_USB_VOLTAGE_CALLBACK_THRESHOLD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_USB_VOLTAGE_CALLBACK_THRESHOLD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_DEBOUNCE_PERIOD] = BrickMaster.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_DEBOUNCE_PERIOD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_IS_ETHERNET_PRESENT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_ETHERNET_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_ETHERNET_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_ETHERNET_STATUS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_ETHERNET_HOSTNAME] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_SET_ETHERNET_MAC_ADDRESS] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_SET_ETHERNET_WEBSOCKET_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_ETHERNET_WEBSOCKET_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_ETHERNET_AUTHENTICATION_SECRET] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_ETHERNET_AUTHENTICATION_SECRET] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI_AUTHENTICATION_SECRET] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI_AUTHENTICATION_SECRET] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_CONNECTION_TYPE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_IS_WIFI2_PRESENT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_START_WIFI2_BOOTLOADER] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_WRITE_WIFI2_SERIAL_PORT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_READ_WIFI2_SERIAL_PORT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_AUTHENTICATION_SECRET] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_AUTHENTICATION_SECRET] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_STATUS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_CLIENT_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_CLIENT_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_CLIENT_HOSTNAME] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_CLIENT_HOSTNAME] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_CLIENT_PASSWORD] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_CLIENT_PASSWORD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_AP_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_AP_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_AP_PASSWORD] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_AP_PASSWORD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SAVE_WIFI2_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_FIRMWARE_VERSION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_ENABLE_WIFI2_STATUS_LED] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_DISABLE_WIFI2_STATUS_LED] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_IS_WIFI2_STATUS_LED_ENABLED] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_MESH_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_MESH_CONFIGURATION] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_MESH_ROUTER_SSID] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_MESH_ROUTER_SSID] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_WIFI2_MESH_ROUTER_PASSWORD] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_MESH_ROUTER_PASSWORD] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_MESH_COMMON_STATUS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_MESH_CLIENT_STATUS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_WIFI2_MESH_AP_STATUS] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_BRICKLET_XMC_FLASH_CONFIG] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_BRICKLET_XMC_FLASH_DATA] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_BRICKLETS_ENABLED] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_BRICKLETS_ENABLED] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_SEND_TIMEOUT_COUNT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_SET_SPITFP_BAUDRATE] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_GET_SPITFP_BAUDRATE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_SPITFP_ERROR_COUNT] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_ENABLE_STATUS_LED] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_DISABLE_STATUS_LED] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_IS_STATUS_LED_ENABLED] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_CHIP_TEMPERATURE] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_RESET] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_WRITE_BRICKLET_PLUGIN] = BrickMaster.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickMaster.FUNCTION_READ_BRICKLET_PLUGIN] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickMaster.FUNCTION_GET_IDENTITY] = BrickMaster.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickMaster.CALLBACK_STACK_CURRENT] = (10, 'H')
        self.callback_formats[BrickMaster.CALLBACK_STACK_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickMaster.CALLBACK_USB_VOLTAGE] = (10, 'H')
        self.callback_formats[BrickMaster.CALLBACK_STACK_CURRENT_REACHED] = (10, 'H')
        self.callback_formats[BrickMaster.CALLBACK_STACK_VOLTAGE_REACHED] = (10, 'H')
        self.callback_formats[BrickMaster.CALLBACK_USB_VOLTAGE_REACHED] = (10, 'H')

        ipcon.add_device(self)

    def get_stack_voltage(self):
        r"""
        Returns the stack voltage. The stack voltage is the
        voltage that is supplied via the stack, i.e. it is given by a
        Step-Down or Step-Up Power Supply.

        .. note::
         It is not possible to measure voltages supplied per PoE or USB with this function.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_STACK_VOLTAGE, (), '', 10, 'H')

    def get_stack_current(self):
        r"""
        Returns the stack current. The stack current is the
        current that is drawn via the stack, i.e. it is given by a
        Step-Down or Step-Up Power Supply.

        .. note::
         It is not possible to measure the current drawn via PoE or USB with this function.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_STACK_CURRENT, (), '', 10, 'H')

    def set_extension_type(self, extension, exttype):
        r"""
        Writes the extension type to the EEPROM of a specified extension.
        The extension is either 0 or 1 (0 is the lower one, 1 is the upper one,
        if only one extension is present use 0).

        Possible extension types:

        .. csv-table::
         :header: "Type", "Description"
         :widths: 10, 100

         "1",    "Chibi"
         "2",    "RS485"
         "3",    "WIFI"
         "4",    "Ethernet"
         "5",    "WIFI 2.0"

        The extension type is already set when bought and it can be set with the
        Brick Viewer, it is unlikely that you need this function.
        """
        self.check_validity()

        extension = int(extension)
        exttype = int(exttype)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_EXTENSION_TYPE, (extension, exttype), 'B I', 0, '')

    def get_extension_type(self, extension):
        r"""
        Returns the type for a given extension as set by :func:`Set Extension Type`.
        """
        self.check_validity()

        extension = int(extension)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_EXTENSION_TYPE, (extension,), 'B', 12, 'I')

    def is_chibi_present(self):
        r"""
        Returns *true* if the Master Brick is at position 0 in the stack and a Chibi
        Extension is available.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_CHIBI_PRESENT, (), '', 9, '!')

    def set_chibi_address(self, address):
        r"""
        Sets the address belonging to the Chibi Extension.

        It is possible to set the address with the Brick Viewer and it will be
        saved in the EEPROM of the Chibi Extension, it does not
        have to be set on every startup.
        """
        self.check_validity()

        address = int(address)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_CHIBI_ADDRESS, (address,), 'B', 0, '')

    def get_chibi_address(self):
        r"""
        Returns the address as set by :func:`Set Chibi Address`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_ADDRESS, (), '', 9, 'B')

    def set_chibi_master_address(self, address):
        r"""
        Sets the address of the Chibi Master. This address is used if the
        Chibi Extension is used as slave (i.e. it does not have a USB connection).

        It is possible to set the address with the Brick Viewer and it will be
        saved in the EEPROM of the Chibi Extension, it does not
        have to be set on every startup.
        """
        self.check_validity()

        address = int(address)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_CHIBI_MASTER_ADDRESS, (address,), 'B', 0, '')

    def get_chibi_master_address(self):
        r"""
        Returns the address as set by :func:`Set Chibi Master Address`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_MASTER_ADDRESS, (), '', 9, 'B')

    def set_chibi_slave_address(self, num, address):
        r"""
        Sets up to 254 slave addresses. 0 has a
        special meaning, it is used as list terminator and not allowed as normal slave
        address. The address numeration (via ``num`` parameter) has to be used
        ascending from 0. For example: If you use the Chibi Extension in Master mode
        (i.e. the stack has an USB connection) and you want to talk to three other
        Chibi stacks with the slave addresses 17, 23, and 42, you should call with
        ``(0, 17)``, ``(1, 23)``, ``(2, 42)`` and ``(3, 0)``. The last call with
        ``(3, 0)`` is a list terminator and indicates that the Chibi slave address
        list contains 3 addresses in this case.

        It is possible to set the addresses with the Brick Viewer, that will take care
        of correct address numeration and list termination.

        The slave addresses will be saved in the EEPROM of the Chibi Extension, they
        don't have to be set on every startup.
        """
        self.check_validity()

        num = int(num)
        address = int(address)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_CHIBI_SLAVE_ADDRESS, (num, address), 'B B', 0, '')

    def get_chibi_slave_address(self, num):
        r"""
        Returns the slave address for a given ``num`` as set by
        :func:`Set Chibi Slave Address`.
        """
        self.check_validity()

        num = int(num)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_SLAVE_ADDRESS, (num,), 'B', 9, 'B')

    def get_chibi_signal_strength(self):
        r"""
        Returns the signal strength in dBm. The signal strength updates every time a
        packet is received.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_SIGNAL_STRENGTH, (), '', 9, 'B')

    def get_chibi_error_log(self):
        r"""
        Returns underrun, CRC error, no ACK and overflow error counts of the Chibi
        communication. If these errors start rising, it is likely that either the
        distance between two Chibi stacks is becoming too big or there are
        interferences.
        """
        self.check_validity()

        return GetChibiErrorLog(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_ERROR_LOG, (), '', 16, 'H H H H'))

    def set_chibi_frequency(self, frequency):
        r"""
        Sets the Chibi frequency range for the Chibi Extension. Possible values are:

        .. csv-table::
         :header: "Type", "Description"
         :widths: 10, 100

         "0",    "OQPSK 868MHz (Europe)"
         "1",    "OQPSK 915MHz (US)"
         "2",    "OQPSK 780MHz (China)"
         "3",    "BPSK40 915MHz"

        It is possible to set the frequency with the Brick Viewer and it will be
        saved in the EEPROM of the Chibi Extension, it does not
        have to be set on every startup.
        """
        self.check_validity()

        frequency = int(frequency)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_CHIBI_FREQUENCY, (frequency,), 'B', 0, '')

    def get_chibi_frequency(self):
        r"""
        Returns the frequency value as set by :func:`Set Chibi Frequency`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_FREQUENCY, (), '', 9, 'B')

    def set_chibi_channel(self, channel):
        r"""
        Sets the channel used by the Chibi Extension. Possible channels are
        different for different frequencies:

        .. csv-table::
         :header: "Frequency", "Possible Channels"
         :widths: 40, 60

         "OQPSK 868MHz (Europe)", "0"
         "OQPSK 915MHz (US)",     "1, 2, 3, 4, 5, 6, 7, 8, 9, 10"
         "OQPSK 780MHz (China)",  "0, 1, 2, 3"
         "BPSK40 915MHz",         "1, 2, 3, 4, 5, 6, 7, 8, 9, 10"

        It is possible to set the channel with the Brick Viewer and it will be
        saved in the EEPROM of the Chibi Extension, it does not
        have to be set on every startup.
        """
        self.check_validity()

        channel = int(channel)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_CHIBI_CHANNEL, (channel,), 'B', 0, '')

    def get_chibi_channel(self):
        r"""
        Returns the channel as set by :func:`Set Chibi Channel`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIBI_CHANNEL, (), '', 9, 'B')

    def is_rs485_present(self):
        r"""
        Returns *true* if the Master Brick is at position 0 in the stack and a RS485
        Extension is available.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_RS485_PRESENT, (), '', 9, '!')

    def set_rs485_address(self, address):
        r"""
        Sets the address (0-255) belonging to the RS485 Extension.

        Set to 0 if the RS485 Extension should be the RS485 Master (i.e.
        connected to a PC via USB).

        It is possible to set the address with the Brick Viewer and it will be
        saved in the EEPROM of the RS485 Extension, it does not
        have to be set on every startup.
        """
        self.check_validity()

        address = int(address)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_RS485_ADDRESS, (address,), 'B', 0, '')

    def get_rs485_address(self):
        r"""
        Returns the address as set by :func:`Set RS485 Address`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_RS485_ADDRESS, (), '', 9, 'B')

    def set_rs485_slave_address(self, num, address):
        r"""
        Sets up to 255 slave addresses. Valid addresses are in range 1-255. 0 has a
        special meaning, it is used as list terminator and not allowed as normal slave
        address. The address numeration (via ``num`` parameter) has to be used
        ascending from 0. For example: If you use the RS485 Extension in Master mode
        (i.e. the stack has an USB connection) and you want to talk to three other
        RS485 stacks with the addresses 17, 23, and 42, you should call with
        ``(0, 17)``, ``(1, 23)``, ``(2, 42)`` and ``(3, 0)``. The last call with
        ``(3, 0)`` is a list terminator and indicates that the RS485 slave address list
        contains 3 addresses in this case.

        It is possible to set the addresses with the Brick Viewer, that will take care
        of correct address numeration and list termination.

        The slave addresses will be saved in the EEPROM of the Chibi Extension, they
        don't have to be set on every startup.
        """
        self.check_validity()

        num = int(num)
        address = int(address)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_RS485_SLAVE_ADDRESS, (num, address), 'B B', 0, '')

    def get_rs485_slave_address(self, num):
        r"""
        Returns the slave address for a given ``num`` as set by
        :func:`Set RS485 Slave Address`.
        """
        self.check_validity()

        num = int(num)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_RS485_SLAVE_ADDRESS, (num,), 'B', 9, 'B')

    def get_rs485_error_log(self):
        r"""
        Returns CRC error counts of the RS485 communication.
        If this counter starts rising, it is likely that the distance
        between the RS485 nodes is too big or there is some kind of
        interference.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_RS485_ERROR_LOG, (), '', 10, 'H')

    def set_rs485_configuration(self, speed, parity, stopbits):
        r"""
        Sets the configuration of the RS485 Extension. The
        Master Brick will try to match the given baud rate as exactly as possible.
        The maximum recommended baud rate is 2000000 (2MBd).
        Possible values for parity are 'n' (none), 'e' (even) and 'o' (odd).

        If your RS485 is unstable (lost messages etc.), the first thing you should
        try is to decrease the speed. On very large bus (e.g. 1km), you probably
        should use a value in the range of 100000 (100kBd).

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.
        """
        self.check_validity()

        speed = int(speed)
        parity = create_char(parity)
        stopbits = int(stopbits)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_RS485_CONFIGURATION, (speed, parity, stopbits), 'I c B', 0, '')

    def get_rs485_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set RS485 Configuration`.
        """
        self.check_validity()

        return GetRS485Configuration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_RS485_CONFIGURATION, (), '', 14, 'I c B'))

    def is_wifi_present(self):
        r"""
        Returns *true* if the Master Brick is at position 0 in the stack and a WIFI
        Extension is available.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_WIFI_PRESENT, (), '', 9, '!')

    def set_wifi_configuration(self, ssid, connection, ip, subnet_mask, gateway, port):
        r"""
        Sets the configuration of the WIFI Extension. The ``ssid`` can have a max length
        of 32 characters. Possible values for ``connection`` are:

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 90

         "0", "DHCP"
         "1", "Static IP"
         "2", "Access Point: DHCP"
         "3", "Access Point: Static IP"
         "4", "Ad Hoc: DHCP"
         "5", "Ad Hoc: Static IP"

        If you set ``connection`` to one of the static IP options then you have to
        supply ``ip``, ``subnet_mask`` and ``gateway`` as an array of size 4 (first
        element of the array is the least significant byte of the address). If
        ``connection`` is set to one of the DHCP options then ``ip``, ``subnet_mask``
        and ``gateway`` are ignored, you can set them to 0.

        The last parameter is the port that your program will connect to.

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        It is recommended to use the Brick Viewer to set the WIFI configuration.
        """
        self.check_validity()

        ssid = create_string(ssid)
        connection = int(connection)
        ip = list(map(int, ip))
        subnet_mask = list(map(int, subnet_mask))
        gateway = list(map(int, gateway))
        port = int(port)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_CONFIGURATION, (ssid, connection, ip, subnet_mask, gateway, port), '32s B 4B 4B 4B H', 0, '')

    def get_wifi_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Wifi Configuration`.
        """
        self.check_validity()

        return GetWifiConfiguration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_CONFIGURATION, (), '', 55, '32s B 4B 4B 4B H'))

    def set_wifi_encryption(self, encryption, key, key_index, eap_options, ca_certificate_length, client_certificate_length, private_key_length):
        r"""
        Sets the encryption of the WIFI Extension. The first parameter is the
        type of the encryption. Possible values are:

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 90

         "0", "WPA/WPA2"
         "1", "WPA Enterprise (EAP-FAST, EAP-TLS, EAP-TTLS, PEAP)"
         "2", "WEP"
         "3", "No Encryption"

        The ``key`` has a max length of 50 characters and is used if ``encryption``
        is set to 0 or 2 (WPA/WPA2 or WEP). Otherwise the value is ignored.

        For WPA/WPA2 the key has to be at least 8 characters long. If you want to set
        a key with more than 50 characters, see :func:`Set Long Wifi Key`.

        For WEP the key has to be either 10 or 26 hexadecimal digits long. It is
        possible to set the WEP ``key_index`` (1-4). If you don't know your
        ``key_index``, it is likely 1.

        If you choose WPA Enterprise as encryption, you have to set ``eap_options`` and
        the length of the certificates (for other encryption types these parameters
        are ignored). The certificates
        themselves can be set with :func:`Set Wifi Certificate`. ``eap_options`` consist
        of the outer authentication (bits 1-2), inner authentication (bit 3) and
        certificate type (bits 4-5):

        .. csv-table::
         :header: "Option", "Bits", "Description"
         :widths: 20, 10, 70

         "outer authentication", "1-2", "0=EAP-FAST, 1=EAP-TLS, 2=EAP-TTLS, 3=EAP-PEAP"
         "inner authentication", "3", "0=EAP-MSCHAP, 1=EAP-GTC"
         "certificate type", "4-5", "0=CA Certificate, 1=Client Certificate, 2=Private Key"

        Example for EAP-TTLS + EAP-GTC + Private Key: ``option = 2 | (1 << 2) | (2 << 3)``.

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        It is recommended to use the Brick Viewer to set the Wi-Fi encryption.
        """
        self.check_validity()

        encryption = int(encryption)
        key = create_string(key)
        key_index = int(key_index)
        eap_options = int(eap_options)
        ca_certificate_length = int(ca_certificate_length)
        client_certificate_length = int(client_certificate_length)
        private_key_length = int(private_key_length)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_ENCRYPTION, (encryption, key, key_index, eap_options, ca_certificate_length, client_certificate_length, private_key_length), 'B 50s B B H H H', 0, '')

    def get_wifi_encryption(self):
        r"""
        Returns the encryption as set by :func:`Set Wifi Encryption`.

        .. note::
         Since Master Brick Firmware version 2.4.4 the key is not returned anymore.
        """
        self.check_validity()

        return GetWifiEncryption(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_ENCRYPTION, (), '', 67, 'B 50s B B H H H'))

    def get_wifi_status(self):
        r"""
        Returns the status of the WIFI Extension. The ``state`` is updated automatically,
        all of the other parameters are updated on startup and every time
        :func:`Refresh Wifi Status` is called.

        Possible states are:

        .. csv-table::
         :header: "State", "Description"
         :widths: 10, 90

         "0", "Disassociated"
         "1", "Associated"
         "2", "Associating"
         "3", "Error"
         "255", "Not initialized yet"
        """
        self.check_validity()

        return GetWifiStatus(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_STATUS, (), '', 44, '6B 6B B h 4B 4B 4B I I B'))

    def refresh_wifi_status(self):
        r"""
        Refreshes the Wi-Fi status (see :func:`Get Wifi Status`). To read the status
        of the Wi-Fi module, the Master Brick has to change from data mode to
        command mode and back. This transaction and the readout itself is
        unfortunately time consuming. This means, that it might take some ms
        until the stack with attached WIFI Extension reacts again after this
        function is called.
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickMaster.FUNCTION_REFRESH_WIFI_STATUS, (), '', 0, '')

    def set_wifi_certificate(self, index, data, data_length):
        r"""
        This function is used to set the certificate as well as password and username
        for WPA Enterprise. To set the username use index 0xFFFF,
        to set the password use index 0xFFFE. The max length of username and
        password is 32.

        The certificate is written in chunks of size 32 and the index is used as
        the index of the chunk. ``data_length`` should nearly always be 32. Only
        the last chunk can have a length that is not equal to 32.

        The starting index of the CA Certificate is 0, of the Client Certificate
        10000 and for the Private Key 20000. Maximum sizes are 1312, 1312 and
        4320 byte respectively.

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after uploading the certificate.

        It is recommended to use the Brick Viewer to set the certificate, username
        and password.
        """
        self.check_validity()

        index = int(index)
        data = list(map(int, data))
        data_length = int(data_length)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_CERTIFICATE, (index, data, data_length), 'H 32B B', 0, '')

    def get_wifi_certificate(self, index):
        r"""
        Returns the certificate for a given index as set by :func:`Set Wifi Certificate`.
        """
        self.check_validity()

        index = int(index)

        return GetWifiCertificate(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_CERTIFICATE, (index,), 'H', 41, '32B B'))

    def set_wifi_power_mode(self, mode):
        r"""
        Sets the power mode of the WIFI Extension. Possible modes are:

        .. csv-table::
         :header: "Mode", "Description"
         :widths: 10, 90

         "0", "Full Speed (high power consumption, high throughput)"
         "1", "Low Power (low power consumption, low throughput)"
        """
        self.check_validity()

        mode = int(mode)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_POWER_MODE, (mode,), 'B', 0, '')

    def get_wifi_power_mode(self):
        r"""
        Returns the power mode as set by :func:`Set Wifi Power Mode`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_POWER_MODE, (), '', 9, 'B')

    def get_wifi_buffer_info(self):
        r"""
        Returns informations about the Wi-Fi receive buffer. The Wi-Fi
        receive buffer has a max size of 1500 byte and if data is transfered
        too fast, it might overflow.

        The return values are the number of overflows, the low watermark
        (i.e. the smallest number of bytes that were free in the buffer) and
        the bytes that are currently used.

        You should always try to keep the buffer empty, otherwise you will
        have a permanent latency. A good rule of thumb is, that you can transfer
        1000 messages per second without problems.

        Try to not send more then 50 messages at a time without any kind of
        break between them.
        """
        self.check_validity()

        return GetWifiBufferInfo(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_BUFFER_INFO, (), '', 16, 'I H H'))

    def set_wifi_regulatory_domain(self, domain):
        r"""
        Sets the regulatory domain of the WIFI Extension. Possible domains are:

        .. csv-table::
         :header: "Domain", "Description"
         :widths: 10, 90

         "0", "FCC: Channel 1-11 (N/S America, Australia, New Zealand)"
         "1", "ETSI: Channel 1-13 (Europe, Middle East, Africa)"
         "2", "TELEC: Channel 1-14 (Japan)"
        """
        self.check_validity()

        domain = int(domain)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_REGULATORY_DOMAIN, (domain,), 'B', 0, '')

    def get_wifi_regulatory_domain(self):
        r"""
        Returns the regulatory domain as set by :func:`Set Wifi Regulatory Domain`.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_REGULATORY_DOMAIN, (), '', 9, 'B')

    def get_usb_voltage(self):
        r"""
        Returns the USB voltage. Does not work with hardware version 2.1 or newer.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_USB_VOLTAGE, (), '', 10, 'H')

    def set_long_wifi_key(self, key):
        r"""
        Sets a long Wi-Fi key (up to 63 chars, at least 8 chars) for WPA encryption.
        This key will be used
        if the key in :func:`Set Wifi Encryption` is set to "-". In the old protocol,
        a payload of size 63 was not possible, so the maximum key length was 50 chars.

        With the new protocol this is possible, since we didn't want to break API,
        this function was added additionally.

        .. versionadded:: 2.0.2$nbsp;(Firmware)
        """
        self.check_validity()

        key = create_string(key)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_LONG_WIFI_KEY, (key,), '64s', 0, '')

    def get_long_wifi_key(self):
        r"""
        Returns the encryption key as set by :func:`Set Long Wifi Key`.

        .. note::
         Since Master Brick firmware version 2.4.4 the key is not returned anymore.

        .. versionadded:: 2.0.2$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_LONG_WIFI_KEY, (), '', 72, '64s')

    def set_wifi_hostname(self, hostname):
        r"""
        Sets the hostname of the WIFI Extension. The hostname will be displayed
        by access points as the hostname in the DHCP clients table.

        Setting an empty String will restore the default hostname.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        hostname = create_string(hostname)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_HOSTNAME, (hostname,), '16s', 0, '')

    def get_wifi_hostname(self):
        r"""
        Returns the hostname as set by :func:`Set Wifi Hostname`.

        An empty String means, that the default hostname is used.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_HOSTNAME, (), '', 24, '16s')

    def set_stack_current_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Stack Current` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Stack Current` callback is only triggered if the current has changed
        since the last triggering.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_STACK_CURRENT_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_stack_current_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Stack Current Callback Period`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_STACK_CURRENT_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_stack_voltage_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`Stack Voltage` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`Stack Voltage` callback is only triggered if the voltage has changed
        since the last triggering.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_STACK_VOLTAGE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_stack_voltage_callback_period(self):
        r"""
        Returns the period as set by :func:`Set Stack Voltage Callback Period`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_STACK_VOLTAGE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_usb_voltage_callback_period(self, period):
        r"""
        Sets the period with which the :cb:`USB Voltage` callback is triggered
        periodically. A value of 0 turns the callback off.

        The :cb:`USB Voltage` callback is only triggered if the voltage has changed
        since the last triggering.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        period = int(period)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_USB_VOLTAGE_CALLBACK_PERIOD, (period,), 'I', 0, '')

    def get_usb_voltage_callback_period(self):
        r"""
        Returns the period as set by :func:`Set USB Voltage Callback Period`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_USB_VOLTAGE_CALLBACK_PERIOD, (), '', 12, 'I')

    def set_stack_current_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Stack Current Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the current is *outside* the min and max values"
         "'i'",    "Callback is triggered when the current is *inside* the min and max values"
         "'<'",    "Callback is triggered when the current is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the current is greater than the min value (max is ignored)"

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_STACK_CURRENT_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_stack_current_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Stack Current Callback Threshold`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return GetStackCurrentCallbackThreshold(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_STACK_CURRENT_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_stack_voltage_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`Stack Voltage Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the voltage is *outside* the min and max values"
         "'i'",    "Callback is triggered when the voltage is *inside* the min and max values"
         "'<'",    "Callback is triggered when the voltage is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the voltage is greater than the min value (max is ignored)"

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_STACK_VOLTAGE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_stack_voltage_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set Stack Voltage Callback Threshold`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return GetStackVoltageCallbackThreshold(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_STACK_VOLTAGE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_usb_voltage_callback_threshold(self, option, min, max):
        r"""
        Sets the thresholds for the :cb:`USB Voltage Reached` callback.

        The following options are possible:

        .. csv-table::
         :header: "Option", "Description"
         :widths: 10, 100

         "'x'",    "Callback is turned off"
         "'o'",    "Callback is triggered when the voltage is *outside* the min and max values"
         "'i'",    "Callback is triggered when the voltage is *inside* the min and max values"
         "'<'",    "Callback is triggered when the voltage is smaller than the min value (max is ignored)"
         "'>'",    "Callback is triggered when the voltage is greater than the min value (max is ignored)"

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        option = create_char(option)
        min = int(min)
        max = int(max)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_USB_VOLTAGE_CALLBACK_THRESHOLD, (option, min, max), 'c H H', 0, '')

    def get_usb_voltage_callback_threshold(self):
        r"""
        Returns the threshold as set by :func:`Set USB Voltage Callback Threshold`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return GetUSBVoltageCallbackThreshold(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_USB_VOLTAGE_CALLBACK_THRESHOLD, (), '', 13, 'c H H'))

    def set_debounce_period(self, debounce):
        r"""
        Sets the period with which the threshold callbacks

        * :cb:`Stack Current Reached`,
        * :cb:`Stack Voltage Reached`,
        * :cb:`USB Voltage Reached`

        are triggered, if the thresholds

        * :func:`Set Stack Current Callback Threshold`,
        * :func:`Set Stack Voltage Callback Threshold`,
        * :func:`Set USB Voltage Callback Threshold`

        keep being reached.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        debounce = int(debounce)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_DEBOUNCE_PERIOD, (debounce,), 'I', 0, '')

    def get_debounce_period(self):
        r"""
        Returns the debounce period as set by :func:`Set Debounce Period`.

        .. versionadded:: 2.0.5$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_DEBOUNCE_PERIOD, (), '', 12, 'I')

    def is_ethernet_present(self):
        r"""
        Returns *true* if the Master Brick is at position 0 in the stack and an Ethernet
        Extension is available.

        .. versionadded:: 2.1.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_ETHERNET_PRESENT, (), '', 9, '!')

    def set_ethernet_configuration(self, connection, ip, subnet_mask, gateway, port):
        r"""
        Sets the configuration of the Ethernet Extension. Possible values for
        ``connection`` are:

        .. csv-table::
         :header: "Value", "Description"
         :widths: 10, 90

         "0", "DHCP"
         "1", "Static IP"

        If you set ``connection`` to static IP options then you have to supply ``ip``,
        ``subnet_mask`` and ``gateway`` as an array of size 4 (first element of the
        array is the least significant byte of the address). If ``connection`` is set
        to the DHCP options then ``ip``, ``subnet_mask`` and ``gateway`` are ignored,
        you can set them to 0.

        The last parameter is the port that your program will connect to.

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        It is recommended to use the Brick Viewer to set the Ethernet configuration.

        .. versionadded:: 2.1.0$nbsp;(Firmware)
        """
        self.check_validity()

        connection = int(connection)
        ip = list(map(int, ip))
        subnet_mask = list(map(int, subnet_mask))
        gateway = list(map(int, gateway))
        port = int(port)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_ETHERNET_CONFIGURATION, (connection, ip, subnet_mask, gateway, port), 'B 4B 4B 4B H', 0, '')

    def get_ethernet_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Ethernet Configuration`.

        .. versionadded:: 2.1.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetEthernetConfiguration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_ETHERNET_CONFIGURATION, (), '', 23, 'B 4B 4B 4B H'))

    def get_ethernet_status(self):
        r"""
        Returns the status of the Ethernet Extension.

        ``mac_address``, ``ip``, ``subnet_mask`` and ``gateway`` are given as an array.
        The first element of the array is the least significant byte of the address.

        ``rx_count`` and ``tx_count`` are the number of bytes that have been
        received/send since last restart.

        ``hostname`` is the currently used hostname.

        .. versionadded:: 2.1.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetEthernetStatus(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_ETHERNET_STATUS, (), '', 66, '6B 4B 4B 4B I I 32s'))

    def set_ethernet_hostname(self, hostname):
        r"""
        Sets the hostname of the Ethernet Extension. The hostname will be displayed
        by access points as the hostname in the DHCP clients table.

        Setting an empty String will restore the default hostname.

        The current hostname can be discovered with :func:`Get Ethernet Status`.

        .. versionadded:: 2.1.0$nbsp;(Firmware)
        """
        self.check_validity()

        hostname = create_string(hostname)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_ETHERNET_HOSTNAME, (hostname,), '32s', 0, '')

    def set_ethernet_mac_address(self, mac_address):
        r"""
        Sets the MAC address of the Ethernet Extension. The Ethernet Extension should
        come configured with a valid MAC address, that is also written on a
        sticker of the extension itself.

        The MAC address can be read out again with :func:`Get Ethernet Status`.

        .. versionadded:: 2.1.0$nbsp;(Firmware)
        """
        self.check_validity()

        mac_address = list(map(int, mac_address))

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_ETHERNET_MAC_ADDRESS, (mac_address,), '6B', 0, '')

    def set_ethernet_websocket_configuration(self, sockets, port):
        r"""
        Sets the Ethernet WebSocket configuration. The first parameter sets the number of socket
        connections that are reserved for WebSockets. The range is 0-7. The connections
        are shared with the plain sockets. Example: If you set the connections to 3,
        there will be 3 WebSocket and 4 plain socket connections available.

        The second parameter is the port for the WebSocket connections. The port can
        not be the same as the port for the plain socket connections.

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        It is recommended to use the Brick Viewer to set the Ethernet configuration.

        .. versionadded:: 2.2.0$nbsp;(Firmware)
        """
        self.check_validity()

        sockets = int(sockets)
        port = int(port)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_ETHERNET_WEBSOCKET_CONFIGURATION, (sockets, port), 'B H', 0, '')

    def get_ethernet_websocket_configuration(self):
        r"""
        Returns the configuration as set by :func:`Set Ethernet Configuration`.

        .. versionadded:: 2.2.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetEthernetWebsocketConfiguration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_ETHERNET_WEBSOCKET_CONFIGURATION, (), '', 11, 'B H'))

    def set_ethernet_authentication_secret(self, secret):
        r"""
        Sets the Ethernet authentication secret. The secret can be a string of up to 64
        characters. An empty string disables the authentication.

        See the :ref:`authentication tutorial <tutorial_authentication>` for more
        information.

        The secret is stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        It is recommended to use the Brick Viewer to set the Ethernet authentication secret.

        The default value is an empty string (authentication disabled).

        .. versionadded:: 2.2.0$nbsp;(Firmware)
        """
        self.check_validity()

        secret = create_string(secret)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_ETHERNET_AUTHENTICATION_SECRET, (secret,), '64s', 0, '')

    def get_ethernet_authentication_secret(self):
        r"""
        Returns the authentication secret as set by
        :func:`Set Ethernet Authentication Secret`.

        .. versionadded:: 2.2.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_ETHERNET_AUTHENTICATION_SECRET, (), '', 72, '64s')

    def set_wifi_authentication_secret(self, secret):
        r"""
        Sets the WIFI authentication secret. The secret can be a string of up to 64
        characters. An empty string disables the authentication.

        See the :ref:`authentication tutorial <tutorial_authentication>` for more
        information.

        The secret is stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        It is recommended to use the Brick Viewer to set the WIFI authentication secret.

        The default value is an empty string (authentication disabled).

        .. versionadded:: 2.2.0$nbsp;(Firmware)
        """
        self.check_validity()

        secret = create_string(secret)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI_AUTHENTICATION_SECRET, (secret,), '64s', 0, '')

    def get_wifi_authentication_secret(self):
        r"""
        Returns the authentication secret as set by
        :func:`Set Wifi Authentication Secret`.

        .. versionadded:: 2.2.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI_AUTHENTICATION_SECRET, (), '', 72, '64s')

    def get_connection_type(self):
        r"""
        Returns the type of the connection over which this function was called.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CONNECTION_TYPE, (), '', 9, 'B')

    def is_wifi2_present(self):
        r"""
        Returns *true* if the Master Brick is at position 0 in the stack and a WIFI
        Extension 2.0 is available.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_WIFI2_PRESENT, (), '', 9, '!')

    def start_wifi2_bootloader(self):
        r"""
        Starts the bootloader of the WIFI Extension 2.0. Returns 0 on success.
        Afterwards the :func:`Write Wifi2 Serial Port` and :func:`Read Wifi2 Serial Port`
        functions can be used to communicate with the bootloader to flash a new
        firmware.

        The bootloader should only be started over a USB connection. It cannot be
        started over a WIFI2 connection, see the :func:`Get Connection Type` function.

        It is recommended to use the Brick Viewer to update the firmware of the WIFI
        Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_START_WIFI2_BOOTLOADER, (), '', 9, 'b')

    def write_wifi2_serial_port(self, data, length):
        r"""
        Writes up to 60 bytes (number of bytes to be written specified by ``length``)
        to the serial port of the bootloader of the WIFI Extension 2.0. Returns 0 on
        success.

        Before this function can be used the bootloader has to be started using the
        :func:`Start Wifi2 Bootloader` function.

        It is recommended to use the Brick Viewer to update the firmware of the WIFI
        Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        data = list(map(int, data))
        length = int(length)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_WRITE_WIFI2_SERIAL_PORT, (data, length), '60B B', 9, 'b')

    def read_wifi2_serial_port(self, length):
        r"""
        Reads up to 60 bytes (number of bytes to be read specified by ``length``)
        from the serial port of the bootloader of the WIFI Extension 2.0.
        Returns the number of actually read bytes.

        Before this function can be used the bootloader has to be started using the
        :func:`Start Wifi2 Bootloader` function.

        It is recommended to use the Brick Viewer to update the firmware of the WIFI
        Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        length = int(length)

        return ReadWifi2SerialPort(*self.ipcon.send_request(self, BrickMaster.FUNCTION_READ_WIFI2_SERIAL_PORT, (length,), 'B', 69, '60B B'))

    def set_wifi2_authentication_secret(self, secret):
        r"""
        Sets the WIFI authentication secret. The secret can be a string of up to 64
        characters. An empty string disables the authentication. The default value is
        an empty string (authentication disabled).

        See the :ref:`authentication tutorial <tutorial_authentication>` for more
        information.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        secret = create_string(secret)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_AUTHENTICATION_SECRET, (secret,), '64s', 0, '')

    def get_wifi2_authentication_secret(self):
        r"""
        Returns the WIFI authentication secret as set by
        :func:`Set Wifi2 Authentication Secret`.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_AUTHENTICATION_SECRET, (), '', 72, '64s')

    def set_wifi2_configuration(self, port, websocket_port, website_port, phy_mode, sleep_mode, website):
        r"""
        Sets the general configuration of the WIFI Extension 2.0.

        The ``port`` parameter sets the port number that your programm will connect
        to.

        The ``websocket_port`` parameter sets the WebSocket port number that your
        JavaScript programm will connect to.

        The ``website_port`` parameter sets the port number for the website of the
        WIFI Extension 2.0.

        The ``phy_mode`` parameter sets the specific wireless network mode to be used.
        Possible values are B, G and N.

        The ``sleep_mode`` parameter is currently unused.

        The ``website`` parameter is used to enable or disable the web interface of
        the WIFI Extension 2.0, which is available from firmware version 2.0.1. Note
        that, for firmware version 2.0.3 and older, to disable the the web interface
        the ``website_port`` parameter must be set to 1 and greater than 1 to enable
        the web interface. For firmware version 2.0.4 and later, setting this parameter
        to 1 will enable the web interface and setting it to 0 will disable the web
        interface.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        port = int(port)
        websocket_port = int(websocket_port)
        website_port = int(website_port)
        phy_mode = int(phy_mode)
        sleep_mode = int(sleep_mode)
        website = int(website)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_CONFIGURATION, (port, websocket_port, website_port, phy_mode, sleep_mode, website), 'H H H B B B', 0, '')

    def get_wifi2_configuration(self):
        r"""
        Returns the general configuration as set by :func:`Set Wifi2 Configuration`.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2Configuration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_CONFIGURATION, (), '', 17, 'H H H B B B'))

    def get_wifi2_status(self):
        r"""
        Returns the client and access point status of the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2Status(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_STATUS, (), '', 65, '! B 4B 4B 4B 6B I I b ! 4B 4B 4B 6B I I B'))

    def set_wifi2_client_configuration(self, enable, ssid, ip, subnet_mask, gateway, mac_address, bssid):
        r"""
        Sets the client specific configuration of the WIFI Extension 2.0.

        The ``enable`` parameter enables or disables the client part of the
        WIFI Extension 2.0.

        The ``ssid`` parameter sets the SSID (up to 32 characters) of the access point
        to connect to.

        If the ``ip`` parameter is set to all zero then ``subnet_mask`` and ``gateway``
        parameters are also set to all zero and DHCP is used for IP address configuration.
        Otherwise those three parameters can be used to configure a static IP address.
        The default configuration is DHCP.

        If the ``mac_address`` parameter is set to all zero then the factory MAC
        address is used. Otherwise this parameter can be used to set a custom MAC
        address.

        If the ``bssid`` parameter is set to all zero then WIFI Extension 2.0 will
        connect to any access point that matches the configured SSID. Otherwise this
        parameter can be used to make the WIFI Extension 2.0 only connect to an
        access point if SSID and BSSID match.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        enable = bool(enable)
        ssid = create_string(ssid)
        ip = list(map(int, ip))
        subnet_mask = list(map(int, subnet_mask))
        gateway = list(map(int, gateway))
        mac_address = list(map(int, mac_address))
        bssid = list(map(int, bssid))

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_CLIENT_CONFIGURATION, (enable, ssid, ip, subnet_mask, gateway, mac_address, bssid), '! 32s 4B 4B 4B 6B 6B', 0, '')

    def get_wifi2_client_configuration(self):
        r"""
        Returns the client configuration as set by :func:`Set Wifi2 Client Configuration`.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2ClientConfiguration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_CLIENT_CONFIGURATION, (), '', 65, '! 32s 4B 4B 4B 6B 6B'))

    def set_wifi2_client_hostname(self, hostname):
        r"""
        Sets the client hostname (up to 32 characters) of the WIFI Extension 2.0. The
        hostname will be displayed by access points as the hostname in the DHCP clients
        table.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        hostname = create_string(hostname)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_CLIENT_HOSTNAME, (hostname,), '32s', 0, '')

    def get_wifi2_client_hostname(self):
        r"""
        Returns the client hostname as set by :func:`Set Wifi2 Client Hostname`.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_CLIENT_HOSTNAME, (), '', 40, '32s')

    def set_wifi2_client_password(self, password):
        r"""
        Sets the client password (up to 63 chars) for WPA/WPA2 encryption.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        password = create_string(password)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_CLIENT_PASSWORD, (password,), '64s', 0, '')

    def get_wifi2_client_password(self):
        r"""
        Returns the client password as set by :func:`Set Wifi2 Client Password`.

        .. note::
         Since WIFI Extension 2.0 firmware version 2.1.3 the password is not
         returned anymore.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_CLIENT_PASSWORD, (), '', 72, '64s')

    def set_wifi2_ap_configuration(self, enable, ssid, ip, subnet_mask, gateway, encryption, hidden, channel, mac_address):
        r"""
        Sets the access point specific configuration of the WIFI Extension 2.0.

        The ``enable`` parameter enables or disables the access point part of the
        WIFI Extension 2.0.

        The ``ssid`` parameter sets the SSID (up to 32 characters) of the access point.

        If the ``ip`` parameter is set to all zero then ``subnet_mask`` and ``gateway``
        parameters are also set to all zero and DHCP is used for IP address configuration.
        Otherwise those three parameters can be used to configure a static IP address.
        The default configuration is DHCP.

        The ``encryption`` parameter sets the encryption mode to be used. Possible
        values are Open (no encryption), WEP or WPA/WPA2 PSK.
        Use the :func:`Set Wifi2 AP Password` function to set the encryption
        password.

        The ``hidden`` parameter makes the access point hide or show its SSID.

        The ``channel`` parameter sets the channel (1 to 13) of the access point.

        If the ``mac_address`` parameter is set to all zero then the factory MAC
        address is used. Otherwise this parameter can be used to set a custom MAC
        address.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        enable = bool(enable)
        ssid = create_string(ssid)
        ip = list(map(int, ip))
        subnet_mask = list(map(int, subnet_mask))
        gateway = list(map(int, gateway))
        encryption = int(encryption)
        hidden = bool(hidden)
        channel = int(channel)
        mac_address = list(map(int, mac_address))

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_AP_CONFIGURATION, (enable, ssid, ip, subnet_mask, gateway, encryption, hidden, channel, mac_address), '! 32s 4B 4B 4B B ! B 6B', 0, '')

    def get_wifi2_ap_configuration(self):
        r"""
        Returns the access point configuration as set by :func:`Set Wifi2 AP Configuration`.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2APConfiguration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_AP_CONFIGURATION, (), '', 62, '! 32s 4B 4B 4B B ! B 6B'))

    def set_wifi2_ap_password(self, password):
        r"""
        Sets the access point password (at least 8 and up to 63 chars) for the configured encryption
        mode, see :func:`Set Wifi2 AP Configuration`.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        password = create_string(password)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_AP_PASSWORD, (password,), '64s', 0, '')

    def get_wifi2_ap_password(self):
        r"""
        Returns the access point password as set by :func:`Set Wifi2 AP Password`.

        .. note::
         Since WIFI Extension 2.0 firmware version 2.1.3 the password is not
         returned anymore.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_AP_PASSWORD, (), '', 72, '64s')

    def save_wifi2_configuration(self):
        r"""
        All configuration functions for the WIFI Extension 2.0 do not change the
        values permanently. After configuration this function has to be called to
        permanently store the values.

        The values are stored in the EEPROM and only applied on startup. That means
        you have to restart the Master Brick after configuration.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_SAVE_WIFI2_CONFIGURATION, (), '', 9, 'B')

    def get_wifi2_firmware_version(self):
        r"""
        Returns the current version of the WIFI Extension 2.0 firmware.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_FIRMWARE_VERSION, (), '', 11, '3B')

    def enable_wifi2_status_led(self):
        r"""
        Turns the green status LED of the WIFI Extension 2.0 on.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickMaster.FUNCTION_ENABLE_WIFI2_STATUS_LED, (), '', 0, '')

    def disable_wifi2_status_led(self):
        r"""
        Turns the green status LED of the WIFI Extension 2.0 off.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickMaster.FUNCTION_DISABLE_WIFI2_STATUS_LED, (), '', 0, '')

    def is_wifi2_status_led_enabled(self):
        r"""
        Returns *true* if the green status LED of the WIFI Extension 2.0 is turned on.

        .. versionadded:: 2.4.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_WIFI2_STATUS_LED_ENABLED, (), '', 9, '!')

    def set_wifi2_mesh_configuration(self, enable, root_ip, root_subnet_mask, root_gateway, router_bssid, group_id, group_ssid_prefix, gateway_ip, gateway_port):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Sets the mesh specific configuration of the WIFI Extension 2.0.

        The ``enable`` parameter enables or disables the mesh part of the
        WIFI Extension 2.0. The mesh part cannot be
        enabled together with the client and access-point part.

        If the ``root_ip`` parameter is set to all zero then ``root_subnet_mask``
        and ``root_gateway`` parameters are also set to all zero and DHCP is used for
        IP address configuration. Otherwise those three parameters can be used to
        configure a static IP address. The default configuration is DHCP.

        If the ``router_bssid`` parameter is set to all zero then the information is
        taken from Wi-Fi scan when connecting the SSID as set by
        :func:`Set Wifi2 Mesh Router SSID`. This only works if the the SSID is not hidden.
        In case the router has hidden SSID this parameter must be specified, otherwise
        the node will not be able to reach the mesh router.

        The ``group_id`` and the ``group_ssid_prefix`` parameters identifies a
        particular mesh network and nodes configured with same ``group_id`` and the
        ``group_ssid_prefix`` are considered to be in the same mesh network.

        The ``gateway_ip`` and the ``gateway_port`` parameters specifies the location
        of the brickd that supports mesh feature.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        enable = bool(enable)
        root_ip = list(map(int, root_ip))
        root_subnet_mask = list(map(int, root_subnet_mask))
        root_gateway = list(map(int, root_gateway))
        router_bssid = list(map(int, router_bssid))
        group_id = list(map(int, group_id))
        group_ssid_prefix = create_string(group_ssid_prefix)
        gateway_ip = list(map(int, gateway_ip))
        gateway_port = int(gateway_port)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_MESH_CONFIGURATION, (enable, root_ip, root_subnet_mask, root_gateway, router_bssid, group_id, group_ssid_prefix, gateway_ip, gateway_port), '! 4B 4B 4B 6B 6B 16s 4B H', 0, '')

    def get_wifi2_mesh_configuration(self):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Returns the mesh configuration as set by :func:`Set Wifi2 Mesh Configuration`.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2MeshConfiguration(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_MESH_CONFIGURATION, (), '', 55, '! 4B 4B 4B 6B 6B 16s 4B H'))

    def set_wifi2_mesh_router_ssid(self, ssid):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Sets the mesh router SSID of the WIFI Extension 2.0.
        It is used to specify the mesh router to connect to.

        Note that even though in the argument of this function a 32 characters long SSID
        is allowed, in practice valid SSID should have a maximum of 31 characters. This
        is due to a bug in the mesh library that we use in the firmware of the extension.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        ssid = create_string(ssid)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_MESH_ROUTER_SSID, (ssid,), '32s', 0, '')

    def get_wifi2_mesh_router_ssid(self):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Returns the mesh router SSID as set by :func:`Set Wifi2 Mesh Router SSID`.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_MESH_ROUTER_SSID, (), '', 40, '32s')

    def set_wifi2_mesh_router_password(self, password):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Sets the mesh router password (up to 64 characters) for WPA/WPA2 encryption.
        The password will be used to connect to the mesh router.

        To apply configuration changes to the WIFI Extension 2.0 the
        :func:`Save Wifi2 Configuration` function has to be called and the Master Brick
        has to be restarted afterwards.

        It is recommended to use the Brick Viewer to configure the WIFI Extension 2.0.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        password = create_string(password)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_WIFI2_MESH_ROUTER_PASSWORD, (password,), '64s', 0, '')

    def get_wifi2_mesh_router_password(self):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Returns the mesh router password as set by :func:`Set Wifi2 Mesh Router Password`.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_MESH_ROUTER_PASSWORD, (), '', 72, '64s')

    def get_wifi2_mesh_common_status(self):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Returns the common mesh status of the WIFI Extension 2.0.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2MeshCommonStatus(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_MESH_COMMON_STATUS, (), '', 21, 'B ! ! H I I'))

    def get_wifi2_mesh_client_status(self):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Returns the mesh client status of the WIFI Extension 2.0.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2MeshClientStatus(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_MESH_CLIENT_STATUS, (), '', 58, '32s 4B 4B 4B 6B'))

    def get_wifi2_mesh_ap_status(self):
        r"""
        Requires WIFI Extension 2.0 firmware 2.1.0.

        Returns the mesh AP status of the WIFI Extension 2.0.

        .. versionadded:: 2.4.2$nbsp;(Firmware)
        """
        self.check_validity()

        return GetWifi2MeshAPStatus(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_WIFI2_MESH_AP_STATUS, (), '', 58, '32s 4B 4B 4B 6B'))

    def set_bricklet_xmc_flash_config(self, config, parameter1, parameter2, data):
        r"""
        This function is for internal use to flash the initial
        bootstrapper and bootloader to the Bricklets.

        If you need to flash a boostrapper/bootloader (for exmaple
        because you made your own Bricklet from scratch) please
        take a look at our open source flash and test tool at
        `https://github.com/Tinkerforge/flash-test <https://github.com/Tinkerforge/flash-test>`__

        Don't use this function directly.

        .. versionadded:: 2.5.0$nbsp;(Firmware)
        """
        self.check_validity()

        config = int(config)
        parameter1 = int(parameter1)
        parameter2 = int(parameter2)
        data = list(map(int, data))

        return SetBrickletXMCFlashConfig(*self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_BRICKLET_XMC_FLASH_CONFIG, (config, parameter1, parameter2, data), 'I I I 52B', 72, 'I 60B'))

    def set_bricklet_xmc_flash_data(self, data):
        r"""
        This function is for internal use to flash the initial
        bootstrapper and bootloader to the Bricklets.

        If you need to flash a boostrapper/bootloader (for exmaple
        because you made your own Bricklet from scratch) please
        take a look at our open source flash and test tool at
        `https://github.com/Tinkerforge/flash-test <https://github.com/Tinkerforge/flash-test>`__

        Don't use this function directly.

        .. versionadded:: 2.5.0$nbsp;(Firmware)
        """
        self.check_validity()

        data = list(map(int, data))

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_BRICKLET_XMC_FLASH_DATA, (data,), '64B', 12, 'I')

    def set_bricklets_enabled(self, bricklets_enabled):
        r"""
        This function is only available in Master Brick hardware version >= 3.0.

        Enables/disables all four Bricklets if set to true/false.

        If you disable the Bricklets the power supply to the Bricklets will be disconnected.
        The Bricklets will lose all configurations if disabled.

        .. versionadded:: 2.5.0$nbsp;(Firmware)
        """
        self.check_validity()

        bricklets_enabled = bool(bricklets_enabled)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_BRICKLETS_ENABLED, (bricklets_enabled,), '!', 0, '')

    def get_bricklets_enabled(self):
        r"""
        Returns *true* if the Bricklets are enabled, *false* otherwise.

        .. versionadded:: 2.5.0$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_BRICKLETS_ENABLED, (), '', 9, '!')

    def set_spitfp_baudrate_config(self, enable_dynamic_baudrate, minimum_dynamic_baudrate):
        r"""
        The SPITF protocol can be used with a dynamic baudrate. If the dynamic baudrate is
        enabled, the Brick will try to adapt the baudrate for the communication
        between Bricks and Bricklets according to the amount of data that is transferred.

        The baudrate will be increased exponentially if lots of data is sent/received and
        decreased linearly if little data is sent/received.

        This lowers the baudrate in applications where little data is transferred (e.g.
        a weather station) and increases the robustness. If there is lots of data to transfer
        (e.g. Thermal Imaging Bricklet) it automatically increases the baudrate as needed.

        In cases where some data has to transferred as fast as possible every few seconds
        (e.g. RS485 Bricklet with a high baudrate but small payload) you may want to turn
        the dynamic baudrate off to get the highest possible performance.

        The maximum value of the baudrate can be set per port with the function
        :func:`Set SPITFP Baudrate`. If the dynamic baudrate is disabled, the baudrate
        as set by :func:`Set SPITFP Baudrate` will be used statically.

        .. versionadded:: 2.4.6$nbsp;(Firmware)
        """
        self.check_validity()

        enable_dynamic_baudrate = bool(enable_dynamic_baudrate)
        minimum_dynamic_baudrate = int(minimum_dynamic_baudrate)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_SPITFP_BAUDRATE_CONFIG, (enable_dynamic_baudrate, minimum_dynamic_baudrate), '! I', 0, '')

    def get_spitfp_baudrate_config(self):
        r"""
        Returns the baudrate config, see :func:`Set SPITFP Baudrate Config`.

        .. versionadded:: 2.4.6$nbsp;(Firmware)
        """
        self.check_validity()

        return GetSPITFPBaudrateConfig(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_SPITFP_BAUDRATE_CONFIG, (), '', 13, '! I'))

    def get_send_timeout_count(self, communication_method):
        r"""
        Returns the timeout count for the different communication methods.

        The methods 0-2 are available for all Bricks, 3-7 only for Master Bricks.

        This function is mostly used for debugging during development, in normal operation
        the counters should nearly always stay at 0.

        .. versionadded:: 2.4.3$nbsp;(Firmware)
        """
        self.check_validity()

        communication_method = int(communication_method)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_SEND_TIMEOUT_COUNT, (communication_method,), 'B', 12, 'I')

    def set_spitfp_baudrate(self, bricklet_port, baudrate):
        r"""
        Sets the baudrate for a specific Bricklet port.

        If you want to increase the throughput of Bricklets you can increase
        the baudrate. If you get a high error count because of high
        interference (see :func:`Get SPITFP Error Count`) you can decrease the
        baudrate.

        If the dynamic baudrate feature is enabled, the baudrate set by this
        function corresponds to the maximum baudrate (see :func:`Set SPITFP Baudrate Config`).

        Regulatory testing is done with the default baudrate. If CE compatibility
        or similar is necessary in your applications we recommend to not change
        the baudrate.

        .. versionadded:: 2.4.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)
        baudrate = int(baudrate)

        self.ipcon.send_request(self, BrickMaster.FUNCTION_SET_SPITFP_BAUDRATE, (bricklet_port, baudrate), 'c I', 0, '')

    def get_spitfp_baudrate(self, bricklet_port):
        r"""
        Returns the baudrate for a given Bricklet port, see :func:`Set SPITFP Baudrate`.

        .. versionadded:: 2.4.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_SPITFP_BAUDRATE, (bricklet_port,), 'c', 12, 'I')

    def get_spitfp_error_count(self, bricklet_port):
        r"""
        Returns the error count for the communication between Brick and Bricklet.

        The errors are divided into

        * ACK checksum errors,
        * message checksum errors,
        * framing errors and
        * overflow errors.

        The errors counts are for errors that occur on the Brick side. All
        Bricklets have a similar function that returns the errors on the Bricklet side.

        .. versionadded:: 2.4.3$nbsp;(Firmware)
        """
        self.check_validity()

        bricklet_port = create_char(bricklet_port)

        return GetSPITFPErrorCount(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_SPITFP_ERROR_COUNT, (bricklet_port,), 'c', 24, 'I I I I'))

    def enable_status_led(self):
        r"""
        Enables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.2$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickMaster.FUNCTION_ENABLE_STATUS_LED, (), '', 0, '')

    def disable_status_led(self):
        r"""
        Disables the status LED.

        The status LED is the blue LED next to the USB connector. If enabled is is
        on and it flickers if data is transfered. If disabled it is always off.

        The default state is enabled.

        .. versionadded:: 2.3.2$nbsp;(Firmware)
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickMaster.FUNCTION_DISABLE_STATUS_LED, (), '', 0, '')

    def is_status_led_enabled(self):
        r"""
        Returns *true* if the status LED is enabled, *false* otherwise.

        .. versionadded:: 2.3.2$nbsp;(Firmware)
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_IS_STATUS_LED_ENABLED, (), '', 9, '!')

    def get_protocol1_bricklet_name(self, port):
        r"""
        Returns the firmware and protocol version and the name of the Bricklet for a
        given port.

        This functions sole purpose is to allow automatic flashing of v1.x.y Bricklet
        plugins.
        """
        self.check_validity()

        port = create_char(port)

        return GetProtocol1BrickletName(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_PROTOCOL1_BRICKLET_NAME, (port,), 'c', 52, 'B 3B 40s'))

    def get_chip_temperature(self):
        r"""
        Returns the temperature as measured inside the microcontroller. The
        value returned is not the ambient temperature!

        The temperature is only proportional to the real temperature and it has an
        accuracy of ±15%. Practically it is only useful as an indicator for
        temperature changes.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_CHIP_TEMPERATURE, (), '', 10, 'h')

    def reset(self):
        r"""
        Calling this function will reset the Brick. Calling this function
        on a Brick inside of a stack will reset the whole stack.

        After a reset you have to create new device objects,
        calling functions on the existing ones will result in
        undefined behavior!
        """
        self.check_validity()

        self.ipcon.send_request(self, BrickMaster.FUNCTION_RESET, (), '', 0, '')

    def write_bricklet_plugin(self, port, offset, chunk):
        r"""
        Writes 32 bytes of firmware to the bricklet attached at the given port.
        The bytes are written to the position offset * 32.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        port = create_char(port)
        offset = int(offset)
        chunk = list(map(int, chunk))

        self.ipcon.send_request(self, BrickMaster.FUNCTION_WRITE_BRICKLET_PLUGIN, (port, offset, chunk), 'c B 32B', 0, '')

    def read_bricklet_plugin(self, port, offset):
        r"""
        Reads 32 bytes of firmware from the bricklet attached at the given port.
        The bytes are read starting at the position offset * 32.

        This function is used by Brick Viewer during flashing. It should not be
        necessary to call it in a normal user program.
        """
        self.check_validity()

        port = create_char(port)
        offset = int(offset)

        return self.ipcon.send_request(self, BrickMaster.FUNCTION_READ_BRICKLET_PLUGIN, (port, offset), 'c B', 40, '32B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickMaster.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

Master = BrickMaster # for backward compatibility
