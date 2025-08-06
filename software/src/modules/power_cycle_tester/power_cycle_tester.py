#!/usr/bin/env -S python3 -u

HOST = "localhost"
PORT = 4223
UID = "HyH"
UID_RELAY = "WKF"

THRES_LOW_LX = 3000
THRES_HIGH_LX = 4000

import time
from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_ambient_light_v3 import BrickletAmbientLightV3
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay

if __name__ == "__main__":
    ipcon = IPConnection()
    al = BrickletAmbientLightV3(UID, ipcon)
    idr = BrickletIndustrialDualRelay(UID_RELAY, ipcon)

    ipcon.connect(HOST, PORT)

    in_thres_counter = 0

    while True:
        illuminance = al.get_illuminance()
        print("Illuminance: " + str(illuminance/100.0) + " lx")
        time.sleep(0.1)

        if (THRES_LOW_LX * 100) < illuminance < (THRES_HIGH_LX * 100):
            in_thres_counter += 1
        else:
            in_thres_counter = 0

        if in_thres_counter > 20: # 2 seconds
            idr.set_monoflop(0, True, 3000)
            in_thres_counter = -30
