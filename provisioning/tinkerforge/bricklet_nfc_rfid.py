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

GetTagID = namedtuple('TagID', ['tag_type', 'tid_length', 'tid'])
GetState = namedtuple('State', ['state', 'idle'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickletNFCRFID(Device):
    r"""
    Reads and writes NFC and RFID tags
    """

    DEVICE_IDENTIFIER = 246
    DEVICE_DISPLAY_NAME = 'NFC/RFID Bricklet'
    DEVICE_URL_PART = 'nfc_rfid' # internal

    CALLBACK_STATE_CHANGED = 8


    FUNCTION_REQUEST_TAG_ID = 1
    FUNCTION_GET_TAG_ID = 2
    FUNCTION_GET_STATE = 3
    FUNCTION_AUTHENTICATE_MIFARE_CLASSIC_PAGE = 4
    FUNCTION_WRITE_PAGE = 5
    FUNCTION_REQUEST_PAGE = 6
    FUNCTION_GET_PAGE = 7
    FUNCTION_GET_IDENTITY = 255

    TAG_TYPE_MIFARE_CLASSIC = 0
    TAG_TYPE_TYPE1 = 1
    TAG_TYPE_TYPE2 = 2
    STATE_INITIALIZATION = 0
    STATE_IDLE = 128
    STATE_ERROR = 192
    STATE_REQUEST_TAG_ID = 2
    STATE_REQUEST_TAG_ID_READY = 130
    STATE_REQUEST_TAG_ID_ERROR = 194
    STATE_AUTHENTICATING_MIFARE_CLASSIC_PAGE = 3
    STATE_AUTHENTICATING_MIFARE_CLASSIC_PAGE_READY = 131
    STATE_AUTHENTICATING_MIFARE_CLASSIC_PAGE_ERROR = 195
    STATE_WRITE_PAGE = 4
    STATE_WRITE_PAGE_READY = 132
    STATE_WRITE_PAGE_ERROR = 196
    STATE_REQUEST_PAGE = 5
    STATE_REQUEST_PAGE_READY = 133
    STATE_REQUEST_PAGE_ERROR = 197
    KEY_A = 0
    KEY_B = 1

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickletNFCRFID.DEVICE_IDENTIFIER, BrickletNFCRFID.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletNFCRFID.FUNCTION_REQUEST_TAG_ID] = BrickletNFCRFID.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFCRFID.FUNCTION_GET_TAG_ID] = BrickletNFCRFID.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFCRFID.FUNCTION_GET_STATE] = BrickletNFCRFID.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFCRFID.FUNCTION_AUTHENTICATE_MIFARE_CLASSIC_PAGE] = BrickletNFCRFID.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFCRFID.FUNCTION_WRITE_PAGE] = BrickletNFCRFID.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFCRFID.FUNCTION_REQUEST_PAGE] = BrickletNFCRFID.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFCRFID.FUNCTION_GET_PAGE] = BrickletNFCRFID.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFCRFID.FUNCTION_GET_IDENTITY] = BrickletNFCRFID.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletNFCRFID.CALLBACK_STATE_CHANGED] = (10, 'B !')

        ipcon.add_device(self)

    def request_tag_id(self, tag_type):
        r"""
        To read or write a tag that is in proximity of the NFC/RFID Bricklet you
        first have to call this function with the expected tag type as parameter.
        It is no problem if you don't know the tag type. You can cycle through
        the available tag types until the tag gives an answer to the request.

        Currently the following tag types are supported:

        * Mifare Classic
        * NFC Forum Type 1
        * NFC Forum Type 2

        After you call :func:`Request Tag ID` the NFC/RFID Bricklet will try to read
        the tag ID from the tag. After this process is done the state will change.
        You can either register the :cb:`State Changed` callback or you can poll
        :func:`Get State` to find out about the state change.

        If the state changes to *RequestTagIDError* it means that either there was
        no tag present or that the tag is of an incompatible type. If the state
        changes to *RequestTagIDReady* it means that a compatible tag was found
        and that the tag ID could be read out. You can now get the tag ID by
        calling :func:`Get Tag ID`.

        If two tags are in the proximity of the NFC/RFID Bricklet, this
        function will cycle through the tags. To select a specific tag you have
        to call :func:`Request Tag ID` until the correct tag id is found.

        In case of any *Error* state the selection is lost and you have to
        start again by calling :func:`Request Tag ID`.
        """
        self.check_validity()

        tag_type = int(tag_type)

        self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_REQUEST_TAG_ID, (tag_type,), 'B', 0, '')

    def get_tag_id(self):
        r"""
        Returns the tag type, tag ID and the length of the tag ID
        (4 or 7 bytes are possible length). This function can only be called if the
        NFC/RFID is currently in one of the *Ready* states. The returned ID
        is the ID that was saved through the last call of :func:`Request Tag ID`.

        To get the tag ID of a tag the approach is as follows:

        1. Call :func:`Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Get State` or
           :cb:`State Changed` callback)
        3. Call :func:`Get Tag ID`
        """
        self.check_validity()

        return GetTagID(*self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_GET_TAG_ID, (), '', 17, 'B B 7B'))

    def get_state(self):
        r"""
        Returns the current state of the NFC/RFID Bricklet.

        On startup the Bricklet will be in the *Initialization* state. The
        initialization will only take about 20ms. After that it changes to *Idle*.

        The functions of this Bricklet can be called in the *Idle* state and all of
        the *Ready* and *Error* states.

        Example: If you call :func:`Request Page`, the state will change to
        *RequestPage* until the reading of the page is finished. Then it will change
        to either *RequestPageReady* if it worked or to *RequestPageError* if it
        didn't. If the request worked you can get the page by calling :func:`Get Page`.

        The same approach is used analogously for the other API functions.
        """
        self.check_validity()

        return GetState(*self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_GET_STATE, (), '', 10, 'B !'))

    def authenticate_mifare_classic_page(self, page, key_number, key):
        r"""
        Mifare Classic tags use authentication. If you want to read from or write to
        a Mifare Classic page you have to authenticate it beforehand.
        Each page can be authenticated with two keys: A (``key_number`` = 0) and B
        (``key_number`` = 1). A new Mifare Classic
        tag that has not yet been written to can be accessed with key A
        and the default key ``[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]``.

        The approach to read or write a Mifare Classic page is as follows:

        1. Call :func:`Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Get State`
           or :cb:`State Changed` callback)
        3. If looking for a specific tag then call :func:`Get Tag ID` and check if the
           expected tag was found, if it was not found go back to step 1
        4. Call :func:`Authenticate Mifare Classic Page` with page and key for the page
        5. Wait for state to change to *AuthenticatingMifareClassicPageReady* (see
           :func:`Get State` or :cb:`State Changed` callback)
        6. Call :func:`Request Page` or :func:`Write Page` to read/write page
        """
        self.check_validity()

        page = int(page)
        key_number = int(key_number)
        key = list(map(int, key))

        self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_AUTHENTICATE_MIFARE_CLASSIC_PAGE, (page, key_number, key), 'H B 6B', 0, '')

    def write_page(self, page, data):
        r"""
        Writes 16 bytes starting from the given page. How many pages are written
        depends on the tag type. The page sizes are as follows:

        * Mifare Classic page size: 16 byte (one page is written)
        * NFC Forum Type 1 page size: 8 byte (two pages are written)
        * NFC Forum Type 2 page size: 4 byte (four pages are written)

        The general approach for writing to a tag is as follows:

        1. Call :func:`Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Get State` or
           :cb:`State Changed` callback)
        3. If looking for a specific tag then call :func:`Get Tag ID` and check if the
           expected tag was found, if it was not found got back to step 1
        4. Call :func:`Write Page` with page number and data
        5. Wait for state to change to *WritePageReady* (see :func:`Get State` or
           :cb:`State Changed` callback)

        If you use a Mifare Classic tag you have to authenticate a page before you
        can write to it. See :func:`Authenticate Mifare Classic Page`.
        """
        self.check_validity()

        page = int(page)
        data = list(map(int, data))

        self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_WRITE_PAGE, (page, data), 'H 16B', 0, '')

    def request_page(self, page):
        r"""
        Reads 16 bytes starting from the given page and stores them into a buffer.
        The buffer can then be read out with :func:`Get Page`.
        How many pages are read depends on the tag type. The page sizes are
        as follows:

        * Mifare Classic page size: 16 byte (one page is read)
        * NFC Forum Type 1 page size: 8 byte (two pages are read)
        * NFC Forum Type 2 page size: 4 byte (four pages are read)

        The general approach for reading a tag is as follows:

        1. Call :func:`Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Get State`
           or :cb:`State Changed` callback)
        3. If looking for a specific tag then call :func:`Get Tag ID` and check if the
           expected tag was found, if it was not found got back to step 1
        4. Call :func:`Request Page` with page number
        5. Wait for state to change to *RequestPageReady* (see :func:`Get State`
           or :cb:`State Changed` callback)
        6. Call :func:`Get Page` to retrieve the page from the buffer

        If you use a Mifare Classic tag you have to authenticate a page before you
        can read it. See :func:`Authenticate Mifare Classic Page`.
        """
        self.check_validity()

        page = int(page)

        self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_REQUEST_PAGE, (page,), 'H', 0, '')

    def get_page(self):
        r"""
        Returns 16 bytes of data from an internal buffer. To fill the buffer
        with specific pages you have to call :func:`Request Page` beforehand.
        """
        self.check_validity()

        return self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_GET_PAGE, (), '', 24, '16B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Bricklet is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position can be 'a', 'b', 'c', 'd', 'e', 'f', 'g' or 'h' (Bricklet Port).
        A Bricklet connected to an :ref:`Isolator Bricklet <isolator_bricklet>` is always at
        position 'z'.

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickletNFCRFID.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

NFCRFID = BrickletNFCRFID # for backward compatibility
