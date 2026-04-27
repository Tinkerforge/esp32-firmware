/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
let x = {
    "iso15118": {
        "content": {
            "configuration": "Configuration",
            "autocharge": "Autocharge",
            "autocharge_desc": "Enable automatic charging authorization via EV identification",
            "autocharge_help": <><p>When enabled, the charger briefly communicates with the vehicle to read its unique network identifier (MAC address). This identifier can then be used for automatic charging authorization.</p><p>The process takes about 5 to 20 seconds depending on the vehicle, after which the communication session ends.</p></>,
            "read_soc": "Read SoC",
            "read_soc_desc": "Read State of Charge from the EV",
            "read_soc_help": <><p>When enabled, the charger reads the current battery level (State of Charge) from the vehicle once at the start of a charging session. This takes about 10 to 45 seconds depending on the vehicle.</p><p>After the initial reading, the charger tracks the battery level during charging using its built-in energy meter. {options.PRODUCT_NAME.startsWith("WARP") ? <span>This feature is only available on the {options.PRODUCT_NAME} Pro, as it requires the built-in energy meter.</span> : <span>This feature requires a charger with a built-in energy meter.</span>}</p></>,
            "charge_via_iso15118": "Charge via ISO 15118-20",
            "charge_via_iso15118_desc": "Enable charging via ISO 15118-20",
            "charge_via_iso15118_help": <><p>This feature is not yet available. It will be enabled with a future firmware update once ISO 15118-20 certification is complete.</p><p>With ISO 15118-20, the charger will be able to identify the vehicle, continuously read the battery level, and manage charging directly over the ISO 15118-20 protocol.</p></>,
            "min_charge_current": "Minimum Charge Current",
            "min_charge_current_help": <><p>The default minimum charging current that will be offered to vehicles during ISO 15118-20 charging. This value applies to all vehicles unless overridden by a vehicle-specific configuration.</p><p>Some vehicles can charge efficiently at low currents, while others may waste energy or charge very slowly. If a vehicle charges inefficiently at low power, increase this value.</p></>,
            "advanced_settings": "Advanced settings",
            "pib_download": "Download PIB",
            "pib_download_desc": "Download Parameter Information Block",
            "pib_upload": "Upload PIB",
            "pib_upload_desc": "Upload a PIB file to the PLC modem",
            "pib_upload_button": "Flash PIB",
            "pib_uploading": "Flashing PIB..."
        },
        "script": {
            "save_failed": "Failed to save the ISO15118 settings.",
            "reboot_content_changed": "ISO15118 settings",
            "pib_download_failed": "Failed to download PIB",
            "pib_upload_failed": "Failed to upload PIB",
            "pib_upload_success": "PIB successfully flashed. The PLC modem is resetting.",
            "pib_write_failed": "PIB write failed"
        }
    }
}
