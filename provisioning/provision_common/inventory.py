# -*- coding: utf-8 -*-
# Copyright (C) 2021 Matthias Bolte <matthias@tinkerforge.com>
#
# Redistribution and use in source and binary forms of this file,
# with or without modification, are permitted. See the Creative
# Commons Zero (CC0 1.0) License for more details.

import time
from collections import namedtuple

InventoryEntry = namedtuple('InventoryEntry', 'uid connected_uid position hardware_version firmware_version device_identifier')

class Inventory:
    CHANGE_ADDED = 1
    CHANGE_REMOVED = 2
    CHANGE_UPDATED = 3

    def __init__(self, ipcon, change_callback=None):
        self._ipcon = ipcon
        self._change_callback = change_callback
        self._entires = {} # by uid

        ipcon.register_callback(ipcon.CALLBACK_CONNECTED, self._cb_connected)
        ipcon.register_callback(ipcon.CALLBACK_ENUMERATE, self._cb_enumerate)

    def _cb_connected(self, _reason):
        self.update()

    def _cb_enumerate(self, uid, connected_uid, position, hardware_version,
                      firmware_version, device_identifier, enumeration_type):
        position = position.upper()
        hardware_version = tuple(hardware_version)
        firmware_version = tuple(firmware_version)

        if enumeration_type == self._ipcon.ENUMERATION_TYPE_DISCONNECTED:
            entry = self._entires.pop(uid, None)

            if entry != None:
                self._report_change(self.CHANGE_REMOVED, entry)
        else:
            old_entry = self._entires.get(uid)
            new_entry = InventoryEntry(uid, connected_uid, position, hardware_version, firmware_version, device_identifier)

            if old_entry != new_entry:
                self._entires[uid] = new_entry

                if old_entry == None:
                    change = self.CHANGE_ADDED
                else:
                    change = self.CHANGE_UPDATED

                self._report_change(change, new_entry)

    def _report_change(self, change, entry):
        if self._change_callback != None:
            try:
                self._change_callback(change, entry)
            except:
                pass # FIXME

    def clear(self):
        self._entires = {}

    def update(self, timeout=None):
        try:
            self._ipcon.enumerate()
        except:
            pass # FIXME

        if timeout != None:
            time.sleep(timeout)

    def get_all(self, uid=None, connected_uid=None, position=None, hardware_version=None, firmware_version=None, device_identifier=None):
        matched = []

        for entry in list(self._entires.values()):
            if uid != None and entry.uid != uid:
                continue

            if connected_uid != None and entry.connected_uid != connected_uid:
                continue

            if position != None and entry.position != position.upper():
                continue

            if hardware_version != None and entry.hardware_version != tuple(hardware_version):
                continue

            if firmware_version != None and entry.firmware_version != tuple(firmware_version):
                continue

            if device_identifier != None and entry.device_identifier != device_identifier:
                continue

            matched.append(entry)

        return matched

    def get_one(self, **kwargs):
        matched = self.get_all(**kwargs)

        if len(matched) == 0:
            return None

        return matched[0]
