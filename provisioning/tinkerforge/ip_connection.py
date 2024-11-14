# -*- coding: utf-8 -*-
# Copyright (C) 2012-2015, 2017, 2019-2020 Matthias Bolte <matthias@tinkerforge.com>
# Copyright (C) 2011-2012 Olaf Lüke <olaf@tinkerforge.com>
#
# Redistribution and use in source and binary forms of this file,
# with or without modification, are permitted. See the Creative
# Commons Zero (CC0 1.0) License for more details.

import struct
import socket
import sys
import time
import os
import math
import hmac
import hashlib
import errno
import threading

try:
    import queue # Python 3
except ImportError:
    import Queue as queue # Python 2

if not 'INTERNAL_DEVICE_DISPLAY_NAMES' in globals():
    try:
        from .device_display_names import get_device_display_name
    except (ValueError, ImportError):
        from device_display_names import get_device_display_name

# internal
def get_uid_from_data(data):
    return struct.unpack('<I', data[0:4])[0]

# internal
def get_length_from_data(data):
    return struct.unpack('<B', data[4:5])[0]

# internal
def get_function_id_from_data(data):
    return struct.unpack('<B', data[5:6])[0]

# internal
def get_sequence_number_from_data(data):
    return (struct.unpack('<B', data[6:7])[0] >> 4) & 0x0F

# internal
def get_error_code_from_data(data):
    return (struct.unpack('<B', data[7:8])[0] >> 6) & 0x03

BASE58 = '123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ'

# internal
def base58encode(value):
    encoded = ''

    while value >= 58:
        div, mod = divmod(value, 58)
        encoded = BASE58[mod] + encoded
        value = div

    return BASE58[value] + encoded

# internal
def base58decode(encoded):
    value = 0
    column_multiplier = 1

    for c in encoded[::-1]:
        try:
            column = BASE58.index(c)
        except ValueError:
            raise Error(Error.INVALID_UID, 'UID "{0}" contains invalid character'.format(encoded), suppress_context=True)

        value += column * column_multiplier
        column_multiplier *= 58

    return value

# internal
def uid64_to_uid32(uid64):
    value1 = uid64 & 0xFFFFFFFF
    value2 = (uid64 >> 32) & 0xFFFFFFFF

    uid32  = (value1 & 0x00000FFF)
    uid32 |= (value1 & 0x0F000000) >> 12
    uid32 |= (value2 & 0x0000003F) << 16
    uid32 |= (value2 & 0x000F0000) << 6
    uid32 |= (value2 & 0x3F000000) << 2

    return uid32

# internal
def create_chunk_data(data, chunk_offset, chunk_length, chunk_padding):
    chunk_data = data[chunk_offset:chunk_offset + chunk_length]

    if len(chunk_data) < chunk_length:
        chunk_data += [chunk_padding] * (chunk_length - len(chunk_data))

    return chunk_data

if sys.hexversion < 0x03000000:
    # internal
    def create_char(value): # return str with len() == 1 and ord() <= 255
        if isinstance(value, str) and len(value) == 1: # Python2 str satisfies ord() <= 255 by default
            return value
        elif isinstance(value, unicode) and len(value) == 1: # pylint: disable=undefined-variable
            code_point = ord(value)

            if code_point <= 255:
                return chr(code_point)
            else:
                raise ValueError('Invalid char value: ' + repr(value))
        elif isinstance(value, bytearray) and len(value) == 1: # Python2 bytearray satisfies item <= 255 by default
            return chr(value[0])
        elif isinstance(value, int) and value >= 0 and value <= 255:
            return chr(value)
        else:
            raise ValueError('Invalid char value: ' + repr(value))
else:
    # internal
    def create_char(value): # return str with len() == 1 and ord() <= 255
        if isinstance(value, str) and len(value) == 1 and ord(value) <= 255:
            return value
        elif isinstance(value, (bytes, bytearray)) and len(value) == 1: # Python3 bytes/bytearray satisfies item <= 255 by default
            return chr(value[0])
        elif isinstance(value, int) and value >= 0 and value <= 255:
            return chr(value)
        else:
            raise ValueError('Invalid char value: ' + repr(value))

if sys.hexversion < 0x03000000:
    # internal
    def create_char_list(value, expected_type='char list'): # return list of str with len() == 1 and ord() <= 255 for all items
        if isinstance(value, list):
            return map(create_char, value)
        elif isinstance(value, str): # Python2 str satisfies ord() <= 255 by default
            return list(value)
        elif isinstance(value, unicode): # pylint: disable=undefined-variable
            chars = []

            for char in value:
                code_point = ord(char)

                if code_point <= 255:
                    chars.append(chr(code_point))
                else:
                    raise ValueError('Invalid {0} value: {1}'.format(expected_type, repr(value)))

            return chars
        elif isinstance(value, bytearray): # Python2 bytearray satisfies item <= 255 by default
            return map(chr, value)
        else:
            raise ValueError('Invalid {0} value: {1}'.format(expected_type, repr(value)))
else:
    # internal
    def create_char_list(value, expected_type='char list'): # return list of str with len() == 1 and ord() <= 255 for all items
        if isinstance(value, list):
            return list(map(create_char, value))
        elif isinstance(value, str):
            chars = list(value)

            for char in chars:
                if ord(char) > 255:
                    raise ValueError('Invalid {0} value: {1}'.format(expected_type, repr(value)))

            return chars
        elif isinstance(value, (bytes, bytearray)): # Python3 bytes/bytearray satisfies item <= 255 by default
            return list(map(chr, value))
        else:
            raise ValueError('Invalid {0} value: {1}'.format(expected_type, repr(value)))

if sys.hexversion < 0x03000000:
    # internal
    def create_string(value): # return str with ord() <= 255 for all chars
        if isinstance(value, str): # Python2 str satisfies ord() <= 255 by default
            return value
        elif isinstance(value, unicode): # pylint: disable=undefined-variable
            chars = []

            for char in value:
                code_point = ord(char)

                if code_point <= 255:
                    chars.append(chr(code_point))
                else:
                    raise ValueError('Invalid string value: {0}'.format(repr(value)))

            return ''.join(chars)
        elif isinstance(value, bytearray): # Python2 bytearray satisfies item <= 255 by default
            chars = []

            for byte in value:
                chars.append(chr(byte))

            return ''.join(chars)
        else:
            return ''.join(create_char_list(value, expected_type='string'))
else:
    # internal
    def create_string(value): # return str with ord() <= 255 for all chars
        if isinstance(value, str):
            for char in value:
                if ord(char) > 255:
                    raise ValueError('Invalid string value: {0}'.format(repr(value)))

            return value
        elif isinstance(value, (bytes, bytearray)): # Python3 bytes/bytearray satisfies item <= 255 by default
            chars = []

            for byte in value:
                chars.append(chr(byte))

            return ''.join(chars)
        else:
            return ''.join(create_char_list(value, expected_type='string'))

# internal
def pack_payload(data, form):
    if sys.hexversion < 0x03000000:
        packed = ''
    else:
        packed = b''

    for f, d in zip(form.split(' '), data):
        if '!' in f:
            if len(f) > 1:
                if int(f.replace('!', '')) != len(d):
                    raise ValueError('Incorrect bool list length')

                p = [0] * int(math.ceil(len(d) / 8.0))

                for i, b in enumerate(d):
                    if b:
                        p[i // 8] |= 1 << (i % 8)

                packed += struct.pack('<{0}B'.format(len(p)), *p)
            else:
                packed += struct.pack('<?', d)
        elif 'c' in f:
            if sys.hexversion < 0x03000000:
                if len(f) > 1:
                    packed += struct.pack('<' + f, *d)
                else:
                    packed += struct.pack('<' + f, d)
            else:
                if len(f) > 1:
                    packed += struct.pack('<' + f, *list(map(lambda char: bytes([ord(char)]), d)))
                else:
                    packed += struct.pack('<' + f, bytes([ord(d)]))
        elif 's' in f:
            if sys.hexversion < 0x03000000:
                packed += struct.pack('<' + f, d)
            else:
                packed += struct.pack('<' + f, bytes(map(ord, d)))
        elif len(f) > 1:
            packed += struct.pack('<' + f, *d)
        else:
            packed += struct.pack('<' + f, d)

    return packed

# Mark start and end of the unpack_payload funtion, so that the
# saleae bindings can extract it
# UNPACK_PAYLOAD_CUT_HERE
# internal
def unpack_payload(data, form):
    ret = []

    for f in form.split(' '):
        o = f

        if '!' in f:
            if len(f) > 1:
                f = '{0}B'.format(int(math.ceil(int(f.replace('!', '')) / 8.0)))
            else:
                f = 'B'

        f = '<' + f
        length = struct.calcsize(f)
        x = struct.unpack(f, data[:length])

        if '!' in o:
            y = []

            if len(o) > 1:
                for i in range(int(o.replace('!', ''))):
                    y.append(x[i // 8] & (1 << (i % 8)) != 0)
            else:
                y.append(x[0] != 0)

            x = tuple(y)

        if 'c' in f:
            if sys.hexversion < 0x03000000:
                if len(x) > 1:
                    ret.append(x)
                else:
                    ret.append(x[0])
            else:
                if len(x) > 1:
                    ret.append(tuple(map(lambda item: chr(ord(item)), x)))
                else:
                    ret.append(chr(ord(x[0])))
        elif 's' in f:
            if sys.hexversion < 0x03000000:
                s = x[0]
            else:
                s = ''.join(map(chr, x[0]))

            i = s.find('\x00')

            if i >= 0:
                s = s[:i]

            ret.append(s)
        elif len(x) > 1:
            ret.append(x)
        else:
            ret.append(x[0])

        data = data[length:]

    if len(ret) == 1:
        return ret[0]
    else:
        return ret

# UNPACK_PAYLOAD_CUT_HERE

class Error(Exception):
    TIMEOUT = -1
    NOT_ADDED = -6 # obsolete since v2.0
    ALREADY_CONNECTED = -7
    NOT_CONNECTED = -8
    INVALID_PARAMETER = -9
    NOT_SUPPORTED = -10
    UNKNOWN_ERROR_CODE = -11
    STREAM_OUT_OF_SYNC = -12
    INVALID_UID = -13
    NON_ASCII_CHAR_IN_SECRET = -14
    WRONG_DEVICE_TYPE = -15
    DEVICE_REPLACED = -16
    WRONG_RESPONSE_LENGTH = -17

    def __init__(self, value, description, suppress_context=False):
        Exception.__init__(self, '{0} ({1})'.format(description, value))

        self.value = value
        self.description = description

        if sys.hexversion >= 0x03000000 and suppress_context:
            # this is a Python 2 syntax compatible form of the "raise ... from None"
            # syntax in Python 3. especially the timeout error shows in Python 3
            # the queue.Empty exception as its context. this is confusing and doesn't
            # help much. the "raise ... from None" syntax in Python 3 stops the
            # default traceback printer from outputting the context of the exception
            # while keeping the queue.Empty exception in the __context__ field for
            # debugging purposes.
            self.__cause__ = None
            self.__suppress_context__ = True

class Device(object):
    DEVICE_IDENTIFIER_CHECK_PENDING = 0
    DEVICE_IDENTIFIER_CHECK_MATCH = 1
    DEVICE_IDENTIFIER_CHECK_MISMATCH = 2

    RESPONSE_EXPECTED_INVALID_FUNCTION_ID = 0
    RESPONSE_EXPECTED_ALWAYS_TRUE = 1 # getter
    RESPONSE_EXPECTED_TRUE = 2 # setter
    RESPONSE_EXPECTED_FALSE = 3 # setter, default

    # internal
    def __init__(self, uid, ipcon, device_identifier, device_display_name):
        uid_ = base58decode(uid)

        if uid_ > (1 << 64) - 1:
            raise Error(Error.INVALID_UID, 'UID "{0}" is too big'.format(uid))

        if uid_ > (1 << 32) - 1:
            uid_ = uid64_to_uid32(uid_)

        if uid_ == 0:
            raise Error(Error.INVALID_UID, 'UID "{0}" is empty or maps to zero'.format(uid))

        self.replaced = False
        self.uid = uid_
        self.uid_string = uid
        self.ipcon = ipcon
        self.device_identifier = device_identifier
        self.device_display_name = device_display_name
        self.device_identifier_lock = threading.Lock()
        self.device_identifier_check = Device.DEVICE_IDENTIFIER_CHECK_PENDING # protected by device_identifier_lock
        self.wrong_device_display_name = '?' # protected by device_identifier_lock
        self.api_version = (0, 0, 0)
        self.registered_callbacks = {}
        self.callback_formats = {}
        self.high_level_callbacks = {}
        self.expected_response_function_id = None # protected by request_lock
        self.expected_response_sequence_number = None # protected by request_lock
        self.response_queue = queue.Queue()
        self.request_lock = threading.Lock()
        self.stream_lock = threading.Lock()

        self.response_expected = [Device.RESPONSE_EXPECTED_INVALID_FUNCTION_ID] * 256
        self.response_expected[IPConnection.FUNCTION_ADC_CALIBRATE] = Device.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[IPConnection.FUNCTION_GET_ADC_CALIBRATION] = Device.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[IPConnection.FUNCTION_READ_BRICKLET_UID] = Device.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[IPConnection.FUNCTION_WRITE_BRICKLET_UID] = Device.RESPONSE_EXPECTED_ALWAYS_TRUE

    def get_api_version(self):
        """
        Returns the API version (major, minor, revision) of the bindings for
        this device.
        """

        return self.api_version

    def get_response_expected(self, function_id):
        """
        Returns the response expected flag for the function specified by the
        *function_id* parameter. It is *true* if the function is expected to
        send a response, *false* otherwise.

        For getter functions this is enabled by default and cannot be disabled,
        because those functions will always send a response. For callback
        configuration functions it is enabled by default too, but can be
        disabled via the set_response_expected function. For setter functions
        it is disabled by default and can be enabled.

        Enabling the response expected flag for a setter function allows to
        detect timeouts and other error conditions calls of this setter as
        well. The device will then send a response for this purpose. If this
        flag is disabled for a setter function then no response is sent and
        errors are silently ignored, because they cannot be detected.
        """

        if function_id < 0 or function_id >= len(self.response_expected):
            raise ValueError('Function ID {0} out of range'.format(function_id))

        flag = self.response_expected[function_id]

        if flag == Device.RESPONSE_EXPECTED_INVALID_FUNCTION_ID:
            raise ValueError('Invalid function ID {0}'.format(function_id))

        return flag in [Device.RESPONSE_EXPECTED_ALWAYS_TRUE, Device.RESPONSE_EXPECTED_TRUE]

    def set_response_expected(self, function_id, response_expected):
        """
        Changes the response expected flag of the function specified by the
        *function_id* parameter. This flag can only be changed for setter
        (default value: *false*) and callback configuration functions
        (default value: *true*). For getter functions it is always enabled.

        Enabling the response expected flag for a setter function allows to
        detect timeouts and other error conditions calls of this setter as
        well. The device will then send a response for this purpose. If this
        flag is disabled for a setter function then no response is sent and
        errors are silently ignored, because they cannot be detected.
        """

        if function_id < 0 or function_id >= len(self.response_expected):
            raise ValueError('Function ID {0} out of range'.format(function_id))

        flag = self.response_expected[function_id]

        if flag == Device.RESPONSE_EXPECTED_INVALID_FUNCTION_ID:
            raise ValueError('Invalid function ID {0}'.format(function_id))

        if flag == Device.RESPONSE_EXPECTED_ALWAYS_TRUE:
            raise ValueError('Response Expected flag cannot be changed for function ID {0}'.format(function_id))

        if bool(response_expected):
            self.response_expected[function_id] = Device.RESPONSE_EXPECTED_TRUE
        else:
            self.response_expected[function_id] = Device.RESPONSE_EXPECTED_FALSE

    def set_response_expected_all(self, response_expected):
        """
        Changes the response expected flag for all setter and callback
        configuration functions of this device at once.
        """

        if bool(response_expected):
            flag = Device.RESPONSE_EXPECTED_TRUE
        else:
            flag = Device.RESPONSE_EXPECTED_FALSE

        for i in range(len(self.response_expected)):
            if self.response_expected[i] in [Device.RESPONSE_EXPECTED_TRUE, Device.RESPONSE_EXPECTED_FALSE]:
                self.response_expected[i] = flag

    # internal
    def check_validity(self):
        if self.replaced:
            raise Error(Error.DEVICE_REPLACED, 'Device has been replaced')

        if self.device_identifier < 0:
            return

        if self.device_identifier_check == Device.DEVICE_IDENTIFIER_CHECK_MATCH:
            return

        with self.device_identifier_lock:
            if self.device_identifier_check == Device.DEVICE_IDENTIFIER_CHECK_PENDING:
                device_identifier = self.ipcon.send_request(self, 255, (), '', 33, '8s 8s c 3B 3B H')[5] # <device>.get_identity

                if device_identifier == self.device_identifier:
                    self.device_identifier_check = Device.DEVICE_IDENTIFIER_CHECK_MATCH
                else:
                    self.device_identifier_check = Device.DEVICE_IDENTIFIER_CHECK_MISMATCH
                    self.wrong_device_display_name = get_device_display_name(device_identifier)

            if self.device_identifier_check == Device.DEVICE_IDENTIFIER_CHECK_MISMATCH:
                raise Error(Error.WRONG_DEVICE_TYPE,
                            'UID {0} belongs to a {1} instead of the expected {2}'
                            .format(self.uid_string, self.wrong_device_display_name, self.device_display_name))

class BrickDaemon(Device):
    FUNCTION_GET_AUTHENTICATION_NONCE = 1
    FUNCTION_AUTHENTICATE = 2

    def __init__(self, uid, ipcon):
        Device.__init__(self, uid, ipcon, 0, 'Brick Daemon')

        self.api_version = (2, 0, 0)

        self.response_expected[BrickDaemon.FUNCTION_GET_AUTHENTICATION_NONCE] = BrickDaemon.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickDaemon.FUNCTION_AUTHENTICATE] = BrickDaemon.RESPONSE_EXPECTED_TRUE

        ipcon.add_device(self)

    def get_authentication_nonce(self):
        return self.ipcon.send_request(self, BrickDaemon.FUNCTION_GET_AUTHENTICATION_NONCE, (), '', 12, '4B')

    def authenticate(self, client_nonce, digest):
        self.ipcon.send_request(self, BrickDaemon.FUNCTION_AUTHENTICATE, (client_nonce, digest), '4B 20B', 0, '')

class IPConnection(object):
    FUNCTION_ENUMERATE = 254
    FUNCTION_ADC_CALIBRATE = 251
    FUNCTION_GET_ADC_CALIBRATION = 250
    FUNCTION_READ_BRICKLET_UID = 249
    FUNCTION_WRITE_BRICKLET_UID = 248
    FUNCTION_DISCONNECT_PROBE = 128

    CALLBACK_ENUMERATE = 253
    CALLBACK_CONNECTED = 0
    CALLBACK_DISCONNECTED = 1

    BROADCAST_UID = 0

    # enumeration_type parameter to the enumerate callback
    ENUMERATION_TYPE_AVAILABLE = 0
    ENUMERATION_TYPE_CONNECTED = 1
    ENUMERATION_TYPE_DISCONNECTED = 2

    # connect_reason parameter to the connected callback
    CONNECT_REASON_REQUEST = 0
    CONNECT_REASON_AUTO_RECONNECT = 1

    # disconnect_reason parameter to the disconnected callback
    DISCONNECT_REASON_REQUEST = 0
    DISCONNECT_REASON_ERROR = 1
    DISCONNECT_REASON_SHUTDOWN = 2

    # returned by get_connection_state
    CONNECTION_STATE_DISCONNECTED = 0
    CONNECTION_STATE_CONNECTED = 1
    CONNECTION_STATE_PENDING = 2 # auto-reconnect in process

    QUEUE_EXIT = 0
    QUEUE_META = 1
    QUEUE_PACKET = 2

    DISCONNECT_PROBE_INTERVAL = 5

    class CallbackContext(object):
        def __init__(self):
            self.queue = None
            self.thread = None
            self.packet_dispatch_allowed = False
            self.lock = None

    def __init__(self):
        """
        Creates an IP Connection object that can be used to enumerate the available
        devices. It is also required for the constructor of Bricks and Bricklets.
        """

        self.host = None
        self.port = None
        self.timeout = 2.5
        self.auto_reconnect = True
        self.auto_reconnect_allowed = False
        self.auto_reconnect_pending = False
        self.auto_reconnect_internal = False
        self.connect_failure_callback = None
        self.sequence_number_lock = threading.Lock()
        self.next_sequence_number = 0 # protected by sequence_number_lock
        self.authentication_lock = threading.Lock() # protects authentication handshake
        self.next_authentication_nonce = 0 # protected by authentication_lock
        self.devices = {}
        self.replace_lock = threading.Lock() # used to synchronize replacements in the devices dict
        self.registered_callbacks = {}
        self.socket = None # protected by socket_lock
        self.socket_id = 0 # protected by socket_lock
        self.socket_lock = threading.Lock()
        self.socket_send_lock = threading.Lock()
        self.receive_flag = False
        self.receive_thread = None
        self.callback = None
        self.disconnect_probe_flag = False
        self.disconnect_probe_queue = None
        self.disconnect_probe_thread = None
        self.waiter = threading.Semaphore()
        self.brickd = BrickDaemon('2', self)

    def connect(self, host, port):
        """
        Creates a TCP/IP connection to the given *host* and *port*. The host
        and port can point to a Brick Daemon or to a WIFI/Ethernet Extension.

        Devices can only be controlled when the connection was established
        successfully.

        Blocks until the connection is established and throws an exception if
        there is no Brick Daemon or WIFI/Ethernet Extension listening at the
        given host and port.
        """

        with self.socket_lock:
            if self.socket is not None:
                raise Error(Error.ALREADY_CONNECTED,
                            'Already connected to {0}:{1}'.format(self.host, self.port))

            self.host = host
            self.port = port

            self.connect_unlocked(False)

    def disconnect(self):
        """
        Disconnects the TCP/IP connection from the Brick Daemon or the
        WIFI/Ethernet Extension.
        """

        with self.socket_lock:
            self.auto_reconnect_allowed = False

            if self.auto_reconnect_pending:
                # abort potentially pending auto reconnect
                self.auto_reconnect_pending = False
            else:
                if self.socket is None:
                    raise Error(Error.NOT_CONNECTED, 'Not connected')

                self.disconnect_unlocked()

            # end callback thread
            callback = self.callback
            self.callback = None

        # do this outside of socket_lock to allow calling (dis-)connect from
        # the callbacks while blocking on the join call here
        callback.queue.put((IPConnection.QUEUE_META,
                            (IPConnection.CALLBACK_DISCONNECTED,
                             IPConnection.DISCONNECT_REASON_REQUEST, None)))
        callback.queue.put((IPConnection.QUEUE_EXIT, None))

        if threading.current_thread() is not callback.thread:
            callback.thread.join()

    def authenticate(self, secret):
        """
        Performs an authentication handshake with the connected Brick Daemon or
        WIFI/Ethernet Extension. If the handshake succeeds the connection switches
        from non-authenticated to authenticated state and communication can
        continue as normal. If the handshake fails then the connection gets closed.
        Authentication can fail if the wrong secret was used or if authentication
        is not enabled at all on the Brick Daemon or the WIFI/Ethernet Extension.

        For more information about authentication see
        https://www.tinkerforge.com/en/doc/Tutorials/Tutorial_Authentication/Tutorial.html
        """

        try:
            secret_bytes = secret.encode('ascii')
        except UnicodeEncodeError:
            raise Error(Error.NON_ASCII_CHAR_IN_SECRET, 'Authentication secret contains non-ASCII characters')

        with self.authentication_lock:
            if self.next_authentication_nonce == 0:
                try:
                    self.next_authentication_nonce = struct.unpack('<I', os.urandom(4))[0]
                except NotImplementedError:
                    subseconds, seconds = math.modf(time.time())
                    seconds = int(seconds)
                    subseconds = int(subseconds * 1000000)
                    self.next_authentication_nonce = ((seconds << 26 | seconds >> 6) & 0xFFFFFFFF) + subseconds + os.getpid()

            server_nonce = self.brickd.get_authentication_nonce()
            client_nonce = struct.unpack('<4B', struct.pack('<I', self.next_authentication_nonce))
            self.next_authentication_nonce = (self.next_authentication_nonce + 1) % (1 << 32)

            h = hmac.new(secret_bytes, digestmod=hashlib.sha1)

            h.update(struct.pack('<4B', *server_nonce))
            h.update(struct.pack('<4B', *client_nonce))

            digest = struct.unpack('<20B', h.digest())
            h = None

            self.brickd.authenticate(client_nonce, digest)

    def get_connection_state(self):
        """
        Can return the following states:

        - CONNECTION_STATE_DISCONNECTED: No connection is established.
        - CONNECTION_STATE_CONNECTED: A connection to the Brick Daemon or
          the WIFI/Ethernet Extension is established.
        - CONNECTION_STATE_PENDING: IP Connection is currently trying to
          connect.
        """

        if self.socket is not None:
            return IPConnection.CONNECTION_STATE_CONNECTED
        elif self.auto_reconnect_pending:
            return IPConnection.CONNECTION_STATE_PENDING
        else:
            return IPConnection.CONNECTION_STATE_DISCONNECTED

    def set_auto_reconnect(self, auto_reconnect):
        """
        Enables or disables auto-reconnect. If auto-reconnect is enabled,
        the IP Connection will try to reconnect to the previously given
        host and port, if the connection is lost.

        Default value is *True*.
        """

        self.auto_reconnect = bool(auto_reconnect)

        if not self.auto_reconnect:
            # abort potentially pending auto reconnect
            self.auto_reconnect_allowed = False

    def get_auto_reconnect(self):
        """
        Returns *true* if auto-reconnect is enabled, *false* otherwise.
        """

        return self.auto_reconnect

    def set_timeout(self, timeout):
        """
        Sets the timeout in seconds for getters and for setters for which the
        response expected flag is activated.

        Default timeout is 2.5.
        """

        timeout = float(timeout)

        if timeout < 0:
            raise ValueError('Timeout cannot be negative')

        self.timeout = timeout

    def get_timeout(self):
        """
        Returns the timeout as set by set_timeout.
        """

        return self.timeout

    def enumerate(self):
        """
        Broadcasts an enumerate request. All devices will respond with an
        enumerate callback.
        """

        request, _, _ = self.create_packet_header(None, 8, IPConnection.FUNCTION_ENUMERATE)

        self.send(request)

    def wait(self):
        """
        Stops the current thread until unwait is called.

        This is useful if you rely solely on callbacks for events, if you want
        to wait for a specific callback or if the IP Connection was created in
        a thread.

        Wait and unwait act in the same way as "acquire" and "release" of a
        semaphore.
        """
        self.waiter.acquire()

    def unwait(self):
        """
        Unwaits the thread previously stopped by wait.

        Wait and unwait act in the same way as "acquire" and "release" of
        a semaphore.
        """
        self.waiter.release()

    def register_callback(self, callback_id, function):
        """
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

    # internal
    def connect_unlocked(self, is_auto_reconnect):
        # NOTE: assumes that socket is None and socket_lock is locked

        # create callback thread and queue
        if self.callback is None:
            try:
                self.callback = IPConnection.CallbackContext()
                self.callback.queue = queue.Queue()
                self.callback.packet_dispatch_allowed = False
                self.callback.lock = threading.Lock()
                self.callback.thread = threading.Thread(name='Callback-Processor',
                                                        target=self.callback_loop,
                                                        args=(self.callback,))
                self.callback.thread.daemon = True
                self.callback.thread.start()
            except:
                self.callback = None
                raise

        # create and connect socket
        tmp = None

        try:
            tmp = socket.create_connection((self.host, self.port), timeout=5)
            tmp.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

            if sys.platform == 'win32':
                # for some unknown reason the socket recv() call does not
                # immediate return on Windows if the socket gets shut down on
                # disconnect. the socket recv() call will still block for
                # several seconds before it returns. this in turn blocks the
                # disconnect. to workaround this use a 100ms timeout for
                # blocking socket operations.
                tmp.settimeout(0.1)
            else:
                tmp.settimeout(None)
        except Exception as e:
            def cleanup1():
                if tmp != None:
                    try:
                        tmp.close()
                    except:
                        pass

                if self.auto_reconnect_internal:
                    if is_auto_reconnect:
                        return

                    if self.connect_failure_callback is not None:
                        self.connect_failure_callback(e)

                    self.auto_reconnect_allowed = True

                    # FIXME: don't misuse disconnected-callback here to trigger an auto-reconnect
                    #        because not actual connection has been established yet
                    self.callback.queue.put((IPConnection.QUEUE_META,
                                             (IPConnection.CALLBACK_DISCONNECTED,
                                              IPConnection.DISCONNECT_REASON_ERROR, None)))
                else:
                    # end callback thread
                    if not is_auto_reconnect:
                        self.callback.queue.put((IPConnection.QUEUE_EXIT, None))

                        if threading.current_thread() is not self.callback.thread:
                            self.callback.thread.join()

                        self.callback = None

            cleanup1()
            raise

        self.socket = tmp
        self.socket_id += 1

        # create disconnect probe thread
        try:
            self.disconnect_probe_flag = True
            self.disconnect_probe_queue = queue.Queue()
            self.disconnect_probe_thread = threading.Thread(name='Disconnect-Prober',
                                                            target=self.disconnect_probe_loop,
                                                            args=(self.disconnect_probe_queue,))
            self.disconnect_probe_thread.daemon = True
            self.disconnect_probe_thread.start()
        except:
            def cleanup2():
                self.disconnect_probe_thread = None

                # close socket
                self.socket.close()
                self.socket = None

                # end callback thread
                if not is_auto_reconnect:
                    self.callback.queue.put((IPConnection.QUEUE_EXIT, None))

                    if threading.current_thread() is not self.callback.thread:
                        self.callback.thread.join()

                    self.callback = None

            cleanup2()
            raise

        # create receive thread
        self.callback.packet_dispatch_allowed = True

        try:
            self.receive_flag = True
            self.receive_thread = threading.Thread(name='Brickd-Receiver',
                                                   target=self.receive_loop,
                                                   args=(self.socket_id,))
            self.receive_thread.daemon = True
            self.receive_thread.start()
        except:
            def cleanup3():
                self.receive_thread = None

                # close socket
                self.disconnect_unlocked()

                # end callback thread
                if not is_auto_reconnect:
                    self.callback.queue.put((IPConnection.QUEUE_EXIT, None))

                    if threading.current_thread() is not self.callback.thread:
                        self.callback.thread.join()

                    self.callback = None

            cleanup3()
            raise

        self.auto_reconnect_allowed = False
        self.auto_reconnect_pending = False

        if is_auto_reconnect:
            connect_reason = IPConnection.CONNECT_REASON_AUTO_RECONNECT
        else:
            connect_reason = IPConnection.CONNECT_REASON_REQUEST

        self.callback.queue.put((IPConnection.QUEUE_META,
                                 (IPConnection.CALLBACK_CONNECTED,
                                  connect_reason, None)))

    # internal
    def disconnect_unlocked(self):
        # NOTE: assumes that socket is not None and socket_lock is locked

        # end disconnect probe thread
        self.disconnect_probe_queue.put(True)
        self.disconnect_probe_thread.join() # FIXME: use a timeout?
        self.disconnect_probe_thread = None

        # stop dispatching packet callbacks before ending the receive
        # thread to avoid timeout exceptions due to callback functions
        # trying to call getters
        if threading.current_thread() is not self.callback.thread:
            # FIXME: cannot hold callback lock here because this can
            #        deadlock due to an ordering problem with the socket lock
            #with self.callback.lock:
            if True:
                self.callback.packet_dispatch_allowed = False
        else:
            self.callback.packet_dispatch_allowed = False

        # end receive thread
        self.receive_flag = False

        try:
            self.socket.shutdown(socket.SHUT_RDWR)
        except socket.error:
            pass

        if self.receive_thread is not None:
            self.receive_thread.join() # FIXME: use a timeout?
            self.receive_thread = None

        # close socket
        self.socket.close()
        self.socket = None

    # internal
    def set_auto_reconnect_internal(self, auto_reconnect, connect_failure_callback):
        self.auto_reconnect_internal = auto_reconnect
        self.connect_failure_callback = connect_failure_callback

    # internal
    def add_device(self, device):
        with self.replace_lock:
            replaced_device = self.devices.get(device.uid)

            if replaced_device != None:
                replaced_device.replaced = True

            self.devices[device.uid] = device # FIXME: maybe use a weakref here

    # internal
    def receive_loop(self, socket_id):
        if sys.hexversion < 0x03000000:
            pending_data = ''
        else:
            pending_data = bytes()

        while self.receive_flag:
            try:
                data = self.socket.recv(8192)
            except socket.timeout:
                continue
            except socket.error:
                if self.receive_flag:
                    e = sys.exc_info()[1]
                    if e.errno == errno.EINTR:
                        continue

                    self.handle_disconnect_by_peer(IPConnection.DISCONNECT_REASON_ERROR, socket_id, False)
                break

            if len(data) == 0:
                if self.receive_flag:
                    self.handle_disconnect_by_peer(IPConnection.DISCONNECT_REASON_SHUTDOWN, socket_id, False)
                break

            pending_data += data

            while self.receive_flag:
                if len(pending_data) < 8:
                    # Wait for complete header
                    break

                length = get_length_from_data(pending_data)

                if len(pending_data) < length:
                    # Wait for complete packet
                    break

                packet = pending_data[0:length]
                pending_data = pending_data[length:]

                self.handle_response(packet)

    # internal
    def dispatch_meta(self, function_id, parameter, socket_id):
        if function_id == IPConnection.CALLBACK_CONNECTED:
            cb = self.registered_callbacks.get(IPConnection.CALLBACK_CONNECTED)

            if cb != None:
                cb(parameter)
        elif function_id == IPConnection.CALLBACK_DISCONNECTED:
            if parameter != IPConnection.DISCONNECT_REASON_REQUEST:
                # need to do this here, the receive_loop is not allowed to
                # hold the socket_lock because this could cause a deadlock
                # with a concurrent call to the (dis-)connect function
                with self.socket_lock:
                    # don't close the socket if it got disconnected or
                    # reconnected in the meantime
                    if self.socket is not None and self.socket_id == socket_id:
                        # end disconnect probe thread
                        self.disconnect_probe_queue.put(True)
                        self.disconnect_probe_thread.join() # FIXME: use a timeout?
                        self.disconnect_probe_thread = None

                        # close socket
                        self.socket.close()
                        self.socket = None

            # FIXME: wait a moment here, otherwise the next connect
            # attempt will succeed, even if there is no open server
            # socket. the first receive will then fail directly
            time.sleep(0.1)

            cb = self.registered_callbacks.get(IPConnection.CALLBACK_DISCONNECTED)

            if cb != None:
                cb(parameter)

            if parameter != IPConnection.DISCONNECT_REASON_REQUEST and \
               self.auto_reconnect and self.auto_reconnect_allowed:
                self.auto_reconnect_pending = True
                retry = True

                # block here until reconnect. this is okay, there is no
                # callback to deliver when there is no connection
                while retry:
                    retry = False

                    with self.socket_lock:
                        if self.auto_reconnect_allowed and self.socket is None:
                            try:
                                self.connect_unlocked(True)
                            except:
                                retry = True
                        else:
                            self.auto_reconnect_pending = False

                    if retry:
                        time.sleep(0.1)

    # internal
    def dispatch_packet(self, packet):
        uid = get_uid_from_data(packet)
        length = get_length_from_data(packet)
        function_id = get_function_id_from_data(packet)
        payload = packet[8:]

        if function_id == IPConnection.CALLBACK_ENUMERATE:
            cb = self.registered_callbacks.get(IPConnection.CALLBACK_ENUMERATE)

            if cb == None:
                return

            if len(packet) != 34:
                return # silently ignoring callback with wrong length

            uid, connected_uid, position, hardware_version, \
                firmware_version, device_identifier, enumeration_type = \
                unpack_payload(payload, '8s 8s c 3B 3B H B')

            cb(uid, connected_uid, position, hardware_version,
               firmware_version, device_identifier, enumeration_type)

            return

        device = self.devices.get(uid)

        if device == None:
            return

        try:
            device.check_validity()
        except Error:
            return # silently ignoring callback for invalid device

        if -function_id in device.high_level_callbacks:
            hlcb = device.high_level_callbacks[-function_id] # [roles, options, data]
            length, form = device.callback_formats[function_id] # FIXME: currently assuming that low-level callback has more than one element

            if len(packet) != length:
                return # silently ignoring callback with wrong length

            llvalues = unpack_payload(payload, form)
            has_data = False
            data = None

            if hlcb[1]['fixed_length'] != None:
                length = hlcb[1]['fixed_length']
            else:
                length = llvalues[hlcb[0].index('stream_length')]

            if not hlcb[1]['single_chunk']:
                chunk_offset = llvalues[hlcb[0].index('stream_chunk_offset')]
            else:
                chunk_offset = 0

            chunk_data = llvalues[hlcb[0].index('stream_chunk_data')]

            if hlcb[2] == None: # no stream in-progress
                if chunk_offset == 0: # stream starts
                    hlcb[2] = chunk_data

                    if len(hlcb[2]) >= length: # stream complete
                        has_data = True
                        data = hlcb[2][:length]
                        hlcb[2] = None
                else: # ignore tail of current stream, wait for next stream start
                    pass
            else: # stream in-progress
                if chunk_offset != len(hlcb[2]): # stream out-of-sync
                    has_data = True
                    data = None
                    hlcb[2] = None
                else: # stream in-sync
                    hlcb[2] += chunk_data

                    if len(hlcb[2]) >= length: # stream complete
                        has_data = True
                        data = hlcb[2][:length]
                        hlcb[2] = None

            cb = device.registered_callbacks.get(-function_id)

            if has_data and cb != None:
                result = []

                for role, llvalue in zip(hlcb[0], llvalues):
                    if role == 'stream_chunk_data':
                        result.append(data)
                    elif role == None:
                        result.append(llvalue)

                cb(*tuple(result))

        cb = device.registered_callbacks.get(function_id)

        if cb != None:
            length, form = device.callback_formats.get(function_id, (None, None))

            if length == None:
                return # silently ignore registered but unknown callback

            if len(packet) != length:
                return # silently ignoring callback with wrong length

            if len(form) == 0:
                cb()
            elif ' ' not in form:
                cb(unpack_payload(payload, form))
            else:
                cb(*unpack_payload(payload, form))

    # internal
    def callback_loop(self, callback):
        while True:
            kind, data = callback.queue.get()

            # FIXME: cannot hold callback lock here because this can
            #        deadlock due to an ordering problem with the socket lock
            #with callback.lock:
            if True:
                if kind == IPConnection.QUEUE_EXIT:
                    break
                elif kind == IPConnection.QUEUE_META:
                    self.dispatch_meta(*data)
                elif kind == IPConnection.QUEUE_PACKET:
                    # don't dispatch callbacks when the receive thread isn't running
                    if callback.packet_dispatch_allowed:
                        self.dispatch_packet(data)

    # internal
    # NOTE: the disconnect probe thread is not allowed to hold the socket_lock at any
    #       time because it is created and joined while the socket_lock is locked
    def disconnect_probe_loop(self, disconnect_probe_queue):
        request, _, _ = self.create_packet_header(None, 8, IPConnection.FUNCTION_DISCONNECT_PROBE)

        while True:
            try:
                disconnect_probe_queue.get(True, IPConnection.DISCONNECT_PROBE_INTERVAL)
                break
            except queue.Empty:
                pass

            if self.disconnect_probe_flag:
                try:
                    with self.socket_send_lock:
                        while True:
                            try:
                                self.socket.send(request)
                                break
                            except socket.timeout:
                                continue
                except socket.error:
                    self.handle_disconnect_by_peer(IPConnection.DISCONNECT_REASON_ERROR,
                                                   self.socket_id, False)
                    break
            else:
                self.disconnect_probe_flag = True

    # internal
    def send(self, packet):
        with self.socket_lock:
            if self.socket is None:
                raise Error(Error.NOT_CONNECTED, 'Not connected')

            try:
                with self.socket_send_lock:
                    while True:
                        try:
                            self.socket.send(packet)
                            break
                        except socket.timeout:
                            continue
            except socket.error:
                self.handle_disconnect_by_peer(IPConnection.DISCONNECT_REASON_ERROR, None, True)
                raise Error(Error.NOT_CONNECTED, 'Not connected', suppress_context=True)

            self.disconnect_probe_flag = False

    # internal
    def send_request(self, device, function_id, data, form, length_ret, form_ret):
        payload = pack_payload(data, form)
        header, response_expected, sequence_number = self.create_packet_header(device, 8 + len(payload), function_id)
        request = header + payload

        if response_expected:
            with device.request_lock:
                device.expected_response_function_id = function_id
                device.expected_response_sequence_number = sequence_number

                try:
                    self.send(request)

                    while True:
                        response = device.response_queue.get(True, self.timeout)

                        if function_id == get_function_id_from_data(response) and \
                           sequence_number == get_sequence_number_from_data(response):
                            # ignore old responses that arrived after the timeout expired, but before setting
                            # expected_response_function_id and expected_response_sequence_number back to None
                            break
                except queue.Empty:
                    msg = 'Did not receive response for function {0} in time'.format(function_id)
                    raise Error(Error.TIMEOUT, msg, suppress_context=True)
                finally:
                    device.expected_response_function_id = None
                    device.expected_response_sequence_number = None

            error_code = get_error_code_from_data(response)

            if error_code == 0:
                if length_ret == 0:
                    length_ret = 8 # setter with response-expected enabled

                if len(response) != length_ret:
                    msg = 'Expected response of {0} byte for function ID {1}, got {2} byte instead' \
                          .format(length_ret, function_id, len(response))
                    raise Error(Error.WRONG_RESPONSE_LENGTH, msg)
            elif error_code == 1:
                msg = 'Got invalid parameter for function {0}'.format(function_id)
                raise Error(Error.INVALID_PARAMETER, msg)
            elif error_code == 2:
                msg = 'Function {0} is not supported'.format(function_id)
                raise Error(Error.NOT_SUPPORTED, msg)
            else:
                msg = 'Function {0} returned an unknown error'.format(function_id)
                raise Error(Error.UNKNOWN_ERROR_CODE, msg)

            if len(form_ret) > 0:
                return unpack_payload(response[8:], form_ret)
        else:
            self.send(request)

    # internal
    def get_next_sequence_number(self):
        with self.sequence_number_lock:
            sequence_number = self.next_sequence_number + 1
            self.next_sequence_number = sequence_number % 15
            return sequence_number

    # internal
    def handle_response(self, packet):
        self.disconnect_probe_flag = False

        function_id = get_function_id_from_data(packet)
        sequence_number = get_sequence_number_from_data(packet)

        if sequence_number == 0 and function_id == IPConnection.CALLBACK_ENUMERATE:
            if IPConnection.CALLBACK_ENUMERATE in self.registered_callbacks:
                self.callback.queue.put((IPConnection.QUEUE_PACKET, packet))

            return

        uid = get_uid_from_data(packet)
        device = self.devices.get(uid)

        if device == None:
            return # Response from an unknown device, ignoring it

        if sequence_number == 0:
            if function_id in device.registered_callbacks or \
               -function_id in device.high_level_callbacks:
                self.callback.queue.put((IPConnection.QUEUE_PACKET, packet))

            return

        if device.expected_response_function_id == function_id and \
           device.expected_response_sequence_number == sequence_number:
            device.response_queue.put(packet)
            return

        # Response seems to be OK, but can't be handled

    # internal
    def handle_disconnect_by_peer(self, disconnect_reason, socket_id, disconnect_immediately):
        # NOTE: assumes that socket_lock is locked if disconnect_immediately is true

        self.auto_reconnect_allowed = True

        if disconnect_immediately:
            self.disconnect_unlocked()

        self.callback.queue.put((IPConnection.QUEUE_META,
                                 (IPConnection.CALLBACK_DISCONNECTED,
                                  disconnect_reason, socket_id)))

    # internal
    def create_packet_header(self, device, length, function_id):
        uid = IPConnection.BROADCAST_UID
        sequence_number = self.get_next_sequence_number()
        r_bit = 0

        if device is not None:
            uid = device.uid

            if device.get_response_expected(function_id):
                r_bit = 1

        sequence_number_and_options = (sequence_number << 4) | (r_bit << 3)

        return (struct.pack('<IBBBB', uid, length, function_id,
                            sequence_number_and_options, 0),
                bool(r_bit),
                sequence_number)

    # internal
    def get_adc_calibration(self, device):
        return self.send_request(device,
                                 IPConnection.FUNCTION_GET_ADC_CALIBRATION,
                                 (),
                                 '',
                                 12, 'h h')

    # internal
    def adc_calibrate(self, device, port):
        self.send_request(device,
                          IPConnection.FUNCTION_ADC_CALIBRATE,
                          (port,),
                          'c',
                          0, '')

    # internal
    def write_bricklet_uid(self, device, port, uid):
        uid_int = base58decode(uid)

        self.send_request(device,
                          IPConnection.FUNCTION_WRITE_BRICKLET_UID,
                          (port, uid_int),
                          'c I',
                          0, '')

    # internal
    def read_bricklet_uid(self, device, port):
        uid_int = self.send_request(device,
                                    IPConnection.FUNCTION_READ_BRICKLET_UID,
                                    (port,),
                                    'c',
                                    12, 'I')

        return base58encode(uid_int)
