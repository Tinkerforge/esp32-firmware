/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { removeUnicodeHacks } from "../../ts/translation";
let x = {
    "modbus_tcp": {
        "navbar": {
            "modbus_tcp": "Modbus/TCP"
        },
        "content": {
            "disabled": "Disabled",
            "read_only": "Read access only (error on writes)",
            "ignore_writes": "Read access only (ignore writes)",
            "full_access": "Read/write access",
            "table": "Register table",
            "warp": null,
            "bender_emulate": "Open Modbus Charge Control Interface (OMCCI); Bender CC613 compatible",
            "keba_emulate": "Keba P30 C-series compatible",
            "modbus_tcp": "Modbus/TCP",
            "enable": "Modbus/TCP server mode",
            "port": "Port",
            "port_muted": "typically 502",
            "debug": "Debug client",
            "table_docu": "Register documentation",
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
                        <td>Current version: 4</td>
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
                        <td>1100</td>
                        <td>"Button pressed" timestamp</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Timestamp of the last button press. 0 if the button was not pressed since the EVSE boot-up. To act on a button press, read this value periodically. Every time this value changes, the button was pressed at least once.</td>
                    </tr>
                    <tr>
                        <td>1102</td>
                        <td>"Button released" timestamp</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Timestamp of the last button release. 0 if the button was not released since the EVSE boot-up. To act on a button release, read this value periodically. Every time this value changes, the button was released at least once.</td>
                    </tr>
                    <tr>
                        <td>2000</td>
                        <td>Meter type</td>
                        <td>uint32</td>
                        <td>meter</td>
                        <td>
                            <ul>
                                <li>0: No meter available</li>
                                <li>1: SDM72{options.PRODUCT_ID_IS_WARP_ANY ? <span> (WARP1 only)</span> : undefined}</li>
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
                        <td>{options.WARP_DOC_BASE_URL.length > 0 ? <span>See <a href={removeUnicodeHacks(`${options.WARP_DOC_BASE_URL}/docs/mqtt_http/api_reference/meter/#meter_all_values_any`)}>API Documentation</a></span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>3100</td>
                        <td>Phases connected</td>
                        <td>uint32</td>
                        <td>phase_switch</td>
                        <td>The number of phases connected to the vehicle</td>
                    </tr>
                    <tr>
                        <td>3102</td>
                        <td>Phase switch state</td>
                        <td>uint32</td>
                        <td>phase_switch</td>
                        <td>
                            The current state of the phase switch control:
                            <ul>
                                <li>0: Ready to switch phases.</li>
                                <li>1: Phase switch disabled in settings.</li>
                                <li>2: Phase switch enabled, but currently not available.</li>
                                <li>3: Currently switching phases. Commands will be ignored.</li>
                            </ul>
                        </td>
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
                    <tr>
                        <td>4012 bis 4013</td>
                        <td>Type of the last NFC tag</td>
                        <td>uint8 (4x)</td>
                        <td>nfc</td>
                        <td>
                            Type of the last seen NFC tag as ASCII coded hex string.
                            <ul>
                                <li>"0000": Mifare Classic</li>
                                <li>"0001": NFC Forum Typ 1</li>
                                <li>"0002": NFC Forum Typ 2</li>
                                <li>"0003": NFC Forum Typ 3</li>
                                <li>"0004": NFC Forum Typ 4</li>
                                <li>"0005": NFC Forum Typ 5</li>
                            </ul>
                        </td>
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
                            Controls the LED in the charger's front button. <strong>Blink pattern and duration must be written with a single
                            Modbus command! To also set the blink color{options.PRODUCT_ID_IS_WARP_ANY ? <span> (WARP3 only)</span> : undefined},
                            registers 1004 to and including 1013 must be written with a single command.</strong> The setting "Status LED control"
                            must be enabled to be able to control the LED.

                            <ul>
                                <li>0xFFFFFFFF: EVSE controls LED</li>
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
                        <td>1008</td>
                        <td>Front LED blink hue</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Hue of the color (in the <a href="https://en.wikipedia.org/wiki/HSL_and_HSV">HSV color representation</a>) that the blink pattern set in register 1004 should be shown in. Only values between 0 and 359 (°) are allowed.{options.PRODUCT_ID_IS_WARP_ANY ? <span> The color can only be set for a WARP3 charger. WARP and WARP2 Charger use a blue LED.</span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>1010</td>
                        <td>Front LED blink saturation</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Saturation of the color (in the <a href="https://en.wikipedia.org/wiki/HSL_and_HSV">HSV color representation</a>) that the blink pattern set in register 1004 should be shown in. Only values between 0 and 255 are allowed.{options.PRODUCT_ID_IS_WARP_ANY ? <span> The color can only be set for a WARP3 charger. WARP and WARP2 Charger use a blue LED.</span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>1012</td>
                        <td>Front LED blink value</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Value of the color (in the <a href="https://en.wikipedia.org/wiki/HSL_and_HSV">HSV color representation</a>) that the blink pattern set in register 1004 should be shown in. Only values between 0 and 255 are allowed.{options.PRODUCT_ID_IS_WARP_ANY ? <span> The color can only be set for a WARP3 charger. WARP and WARP2 Charger use a blue LED.</span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>2000</td>
                        <td>Reset relative energy</td>
                        <td>uint32</td>
                        <td>meter</td>
                        <td>Resets the relative energy value (input register 2006). Password: 0x3E12E5E7</td>
                    </tr>
                    <tr>
                        <td>3100</td>
                        <td>Trigger phase switch</td>
                        <td>uint32</td>
                        <td>phase_switch</td>
                        <td>1 for single-phase charging. 3 for three-phase charging.</td>
                    </tr>
                    <tr>
                        <td>4000 to 4009</td>
                        <td>ID of the NFC tag to inject</td>
                        <td>uint8 (20x)</td>
                        <td>nfc</td>
                        <td>
                            By writing the registers 4000 up to and including 4013 a NFC tag can be injected (as if using the API nfc/inject_tag):
                            <ul>
                                <li>Register 4000 to 4009: The tag's ID as ASCII coded hex string.</li>
                                <li>
                                    Register 4010 and 4011:
                                    <ul>
                                        <li>"0001": The injected tag can only a start charge (as if using the API nfc/inject_tag_start)</li>
                                        <li>"0002": The injected tag can only a stop charge (as if using the API nfc/inject_tag_stop)</li>
                                        <li>all other values: The injected tag can start and stop a charge (as if using the API nfc/inject_tag)</li>
                                    </ul>
                                </li>
                                <li>
                                    Register 4012 and 4013: The tag's type as ASCII coded hex string:
                                    <ul>
                                        <li>"0000": Mifare Classic</li>
                                        <li>"0001": NFC Forum Typ 1</li>
                                        <li>"0002": NFC Forum Typ 2</li>
                                        <li>"0003": NFC Forum Typ 3</li>
                                        <li>"0004": NFC Forum Typ 4</li>
                                        <li>"0005": NFC Forum Typ 5</li>
                                    </ul>
                                </li>
                            </ul>
                            <strong>Writing registers 4012 and 4013 starts the tag injection. Holding Registers 4000 to 4013 will
                            be cleared afterwards!</strong> The data format of holding registers 4000 to 4013 is identical to the
                            one of input registers 4000 to 4013 (that contain the last seen NFC tag). A physically existing tag can
                            later be (re-)injected by presenting it to the charger and copying the values read from input registers
                            4000 to 4013 into holding registers 4000 to 4013.
                        </td>
                    </tr>
                    <tr>
                        <td>4010 to 4011</td>
                        <td>Usage of the NFC tag to inject</td>
                        <td>uint8 (4x)</td>
                        <td>nfc</td>
                        <td>See description of holding registers 4000 to 4009.</td>
                    </tr>
                    <tr>
                        <td>4012 to 4013</td>
                        <td>Type of the NFC tag to inject</td>
                        <td>uint8 (4x)</td>
                        <td>nfc</td>
                        <td>See description of holding registers 4000 to 4009.</td>
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
                        <td>A charge controller is available. This feature should be available on all chargers provided the
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
                        <td>Feature "phase_switch" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Hardware and configuration support switching between single- and three-phase charging.</td>
                    </tr>
                    <tr>
                        <td>5</td>
                        <td>Feature "nfc" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>A NFC Bricklet is connected and active.</td>
                    </tr>
                    <tr>
                        <td>6</td>
                        <td>Feature "evse_sd_input" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>The charge controller has a shutdown input.</td>
                    </tr>
                    <tr>
                        <td>7</td>
                        <td>Feature "evse_gp_input" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>The charge controller has a general purpose input.</td>
                    </tr>
                    <tr>
                        <td>8</td>
                        <td>Feature "evse_gp_output" available</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>The charge controller has a general purpose output.</td>
                    </tr>
                    <tr>
                        <td>1100</td>
                        <td>State of the shutdown input</td>
                        <td>bool</td>
                        <td>evse_sd_input</td>
                        <td>0 - closed, 1 - open</td>
                    </tr>
                    <tr>
                        <td>1101</td>
                        <td>State of the general purpose input</td>
                        <td>bool</td>
                        <td>evse_gp_input</td>
                        <td>0 - closed, 1 - open</td>
                    </tr>
                    <tr>
                        <td>1102</td>
                        <td>Front button state</td>
                        <td>bool</td>
                        <td>evse</td>
                        <td>0 - not pressed, 1 - pressed</td>
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
                    <tr>
                        <td>1100</td>
                        <td>Sets the state of the general purpose output</td>
                        <td>bool</td>
                        <td>evse_gp_output</td>
                        <td>0 - connected to ground, 1 - high impedance</td>
                    </tr>
                </tbody>
            </>
        },
        "script": {
            "save_failed": "Failed to save the Modbus/TCP settings."
        }
    }
}
