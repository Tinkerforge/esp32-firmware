/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "modbus_tcp": {
        "navbar": {
            "modbus_tcp": "Modbus/TCP"
        },
        "content": {
            "disabled": "Disabled",
            "read_only": "Read access only",
            "full_access": "Read/write access",
            "table": "Register table",
            "tf": "WARP Charger",
            "bender_emulate": "Bender CC613 compatible",
            "keba_emulate": "Keba C-series compatible",
            "modbus_tcp": "Modbus/TCP",
            "enable": "Modbus/TCP mode",
            "port": "Port",
            "port_muted": "typically 502",
            "table_docu": "WARP register documentation",
            "table_content": <>
                <thead class="thead-light">
                    <tr>
                        <th scope="col">Register address</th>
                        <th scope="col">Name</th>
                        <th scope="col">Type</th>
                        <th scope="col">Required feature</th>
                        <th scope="col">Description</th>
                    </tr>
                </thead>
                <thead>
                    <tr>
                        <th colSpan={5}>Input Registers (function codes: read: 4)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>0</td>
                        <td>Modbus table version</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Current version: 2</td>
                    </tr>
                    <tr>
                        <td>2</td>
                        <td>Firmware Version Major</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>For example 2 for firmware 2.4.0+66558ade</td>
                    </tr>
                    <tr>
                        <td>4</td>
                        <td>Firmware Version Minor</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>For example 4 for firmware 2.4.0+66558ade</td>
                    </tr>
                    <tr>
                        <td>6</td>
                        <td>Firmware Version Patch</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>For example 0 for firmware 2.4.0+66558ade</td>
                    </tr>
                    <tr>
                        <td>8</td>
                        <td>Firmware build timestamp</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Unix timestamp of the point in time when the firmware was built. For example 0x66558ADE for firmware
                            2.4.0+66558ade (1716882142 in decimal), i.e. 04/28/2024 07:42:22 UTC.</td>
                    </tr>
                    <tr>
                        <td>10</td>
                        <td>Box ID</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Decoded form of the base58 UID used for the default hostname, SSID, etc. For example 185460 for X8A.</td>
                    </tr>
                    <tr>
                        <td>12</td>
                        <td>Uptime (s)</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Time in seconds since the boot-up of the charger firmware.</td>
                    </tr>
                    <tr>
                        <td>1000</td>
                        <td>IEC-61851 state</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>
                            <ul>
                                <li>0: A</li>
                                <li>1: B</li>
                                <li>2: C</li>
                                <li>3: D</li>
                                <li>4: E/F</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>1002</td>
                        <td>Charge state</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>
                            <ul>
                                <li>0: Disconnected</li>
                                <li>1: Waiting for release</li>
                                <li>2: Ready</li>
                                <li>3: Charging</li>
                                <li>4: Error</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>1004</td>
                        <td>Current user</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>ID of the user who started a charge. 0 if the charge is not assigned to a user. 0xFFFFFFFF if no charge is
                            running.</td>
                    </tr>
                    <tr>
                        <td>1006</td>
                        <td>Start timestamp (min)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>A unix timestamp in <strong>minutes</strong> that indices the start of the current charge. 0 if there was no
                            time sync available.</td>
                    </tr>
                    <tr>
                        <td>1008</td>
                        <td>Charging time (s)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Duration of the current charge in seconds. Available even without NTP sync.</td>
                    </tr>
                    <tr>
                        <td>1010</td>
                        <td>Allowed charging current</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Maximum allowed current that a vehicle is allowed to draw. Minimum of all active charging slots.</td>
                    </tr>
                    <tr>
                        <td>1012 to 1050</td>
                        <td>Charging slots (mA)</td>
                        <td>uint32 (20x)</td>
                        <td>evse</td>
                        <td>Value of all charging slots in milliampere. 0xFFFFFFFF if a slot is not active. 0 if a slot blocks. Else
                            between 6000 (6 A) and 32000 (32 A).</td>
                    </tr>
                    <tr>
                        <td>2000</td>
                        <td>Meter type</td>
                        <td>uint32</td>
                        <td>meter</td>
                        <td>
                            <ul>
                                <li>0: No meter available</li>
                                <li>1: SDM72 (WARP1 only)</li>
                                <li>2: SDM630</li>
                                <li>3: SDM72 V2</li>
                                <li>4: SDM72CTM</li>
                                <li>5: SDM630MCT V2</li>
                                <li>6: DSZ15DZMOD</li>
                                <li>7: DEM4A</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>2002</td>
                        <td>Power (W)</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>The current charging power in watt.</td>
                    </tr>
                    <tr>
                        <td>2004</td>
                        <td>Absolute energy (kWh)</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>The charged energy since manufacturing of the energy meter.</td>
                    </tr>
                    <tr>
                        <td>2006</td>
                        <td>Relative energy (kWh)</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>The charged energy since the last reset (see holding register 2000).</td>
                    </tr>
                    <tr>
                        <td>2008</td>
                        <td>Energy this charge</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>The charged energy since the start of the current charge.</td>
                    </tr>
                    <tr>
                        <td>2100 to 2268</td>
                        <td>More meter values</td>
                        <td>float32 (85x)</td>
                        <td>all_values</td>
                        <td>See <a href="https://docs.warp-charger.com/docs/mqtt_http/api_reference/meter/#meter_all_values_any">API Documentation</a></td>
                    </tr>
                    <tr>
                        <td>3000</td>
                        <td>CP-State</td>
                        <td>uint32</td>
                        <td>cp_disc</td>
                        <td>Not implemented yet!</td>
                    </tr>
                    <tr>
                        <td>4000 to 4009</td>
                        <td>ID of the last NFC tag</td>
                        <td>uint8 (20x)</td>
                        <td>nfc</td>
                        <td>ID of the last seen NFC tag as ASCII coded hex string.</td>
                    </tr>
                    <tr>
                        <td>4010</td>
                        <td>Age of the last NFC tag</td>
                        <td>uint32</td>
                        <td>nfc</td>
                        <td>Time in milliseconds since the last NFC tag was seen. An age less than 1000 ms usually indicates that the
                            tag is currently held to the charger.</td>
                    </tr>
                </tbody>
                <thead>
                    <tr>
                        <th colSpan={5}>Holding Registers (function codes: read: 3, write single: 6, write multiple: 16)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>0</td>
                        <td>Reboot</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Reboots the charger (in other words the ESP Brick) to for example apply configuration changes. Password:
                            0x012EB007</td>
                    </tr>
                    <tr>
                        <td>1000</td>
                        <td>Charge release</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td><strong>Deprecated. Use coil 1000 instead!</strong> 0 to block charging; a value not equal to 0 to allow
                            charging.</td>
                    </tr>
                    <tr>
                        <td>1002</td>
                        <td>Allowed current (mA)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>0 mA or 6000 mA to 32000 mA. Other charging slots can further decrease the allowed current.</td>
                    </tr>
                    <tr>
                        <td>1004</td>
                        <td>Front LED blink pattern</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>
                            Controls the LED in the charger's front button.
                            <ul>
                                <li>-1: EVSE controls LED</li>
                                <li>0: LED off</li>
                                <li>1 to 254: LED dimmed</li>
                                <li>255: LED on</li>
                                <li>1001: known NFC tag seen</li>
                                <li>1002: unknown NFC tag seen</li>
                                <li>1003: NFC tag required</li>
                                <li>2001 to 2010: blink 1 to 10 times</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>1006</td>
                        <td>Front LED blink duration</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>The duration in milliseconds that the blink pattern set in register 1004 shall be shown. At most 65536 ms
                            are supported.</td>
                    </tr>
                    <tr>
                        <td>2000</td>
                        <td>Reset relative energy</td>
                        <td>uint32</td>
                        <td>meter</td>
                        <td>Resets the relative energy value (input register 2006). Password: 0x3E12E5E7</td>
                    </tr>
                    <tr>
                        <td>3000</td>
                        <td>Trigger CP disconnect</td>
                        <td>uint32</td>
                        <td>cp_disc</td>
                        <td>Not implemented yet!</td>
                    </tr>
                </tbody>
                <thead>
                    <tr>
                        <th colSpan={5}>Discrete Input Registers (function codes: read: 2)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>0</td>
                        <td>Feature "evse" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>A charge controller is available. This feature should be available on all WARP chargers provided the
                            hardware is working correctly.</td>
                    </tr>
                    <tr>
                        <td>1</td>
                        <td>Feature "meter" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>An energy meter and hardware to read it via RS485 is available. This feature will bet set if an energy meter
                            is read successfully at least once via Modbus.</td>
                    </tr>
                    <tr>
                        <td>2</td>
                        <td>Feature "phases" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>The detected energy meter supports phase-wise measurements.</td>
                    </tr>
                    <tr>
                        <td>3</td>
                        <td>Feature "all_values" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>The detected energy meter supports more measurements.</td>
                    </tr>
                    <tr>
                        <td>4</td>
                        <td>Feature "cp_disc" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Not implemented yet!</td>
                    </tr>
                    <tr>
                        <td>5</td>
                        <td>Feature "nfc" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>A NFC Bricklet is connected and active.</td>
                    </tr>
                    <tr>
                        <td>2100</td>
                        <td>Phase L1 connected</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2101</td>
                        <td>Phase L2 connected</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2102</td>
                        <td>Phase L3 connected</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2103</td>
                        <td>Phase L1 active</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2104</td>
                        <td>Phase L2 active</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2105</td>
                        <td>Phase L3 active</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                </tbody>
                <thead>
                    <tr>
                        <th colSpan={5}>Coils (function codes: read: 1, write single: 5, write multiple: 15)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>1000</td>
                        <td>Charge release</td>
                        <td>bool</td>
                        <td>evse</td>
                        <td>false or 0 to block charging. true or 1 to allow charging. Identical to holding register 1000.</td>
                    </tr>
                    <tr>
                        <td>1001</td>
                        <td>Manual charge release</td>
                        <td>bool</td>
                        <td>evse</td>
                        <td>false or 0 to block charging. true or 1 to allow charging. Sets the same charge release that is used via the
                            web interface, the API or (depending on the button configuration) the button.</td>
                    </tr>
                </tbody>
            </>
        },
        "script": {
            "save_failed": "Failed to save the Modbus/TCP configuration.",
            "reboot_content_changed": "Modbus/TCP settings"
        }
    }
}
