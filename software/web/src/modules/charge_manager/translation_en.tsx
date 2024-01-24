/** @jsxImportSource preact */
import { h } from "preact";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": null,
            "not_configured": "Deactivated",
            "manager": "Activated",
            "error": "Error",
            "managed_boxes": "Managed chargers",
            "available_current": "Available current"
        },
        "navbar": {
            "charge_manager": null
        },
        "content": {
            "charge_manager": null,
            "enable_charge_manager": "Charge management mode",
            "enable_charge_manager_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "enable_watchdog": "Watchdog enabled",
            "enable_watchdog_muted": "only enable if using the API (not required for normal charge manager use!)",
            "enable_watchdog_desc": "Sets the available current to the default value if it is not updated every 30 seconds",
            "verbose": "Current distribution log enabled",
            "verbose_desc": "Creates log entries whenever current is redistributed",
            "default_available_current": "Default available current",
            "default_available_current_muted": "will be used after charge manager reboot",
            "default_available_current_invalid": "The default available current can at most be the maximum total current.",
            "maximum_available_current": "Maximum total current",
            "maximum_available_current_muted": "Maximum allowed current of all chargers in total. This is usually the rated current of the common supply line.",
            "minimum_current_auto": "Automatic minimum current",
            "minimum_current_auto_desc": "Automatically set minimum charge current based on selected vehicle model",
            "minimum_current_vehicle_type": "Vehicle model",
            "minimum_current_vehicle_type_other": "Default",
            "minimum_current_vehicle_type_zoe": "Renault ZOE R135, ZOE R110 or Twingo Z.E.",
            "minimum_current": "Minimum charge current",
            "minimum_current_muted": "lowest charge current supported by your vehicle",
            "minimum_current_1p": "Minimum single-phase charge current",
            "minimum_current_1p_muted": "lowest charge current supported by your vehicle when charging in single-phase mode",
            "minimum_current_3p": "Minimum three-phase charge current",
            "minimum_current_3p_muted": "lowest charge current supported by your vehicle when charging in three-phase mode",

            "requested_current_margin": "Phase current margin",
            "requested_current_margin_muted": "",
            "requested_current_threshold": "Start-up phase length",
            "requested_current_threshold_muted": "Chargers with an energy meter that measures phase currents will be limited to the maximum phase current plus the configured margin after the start-up phase is elapsed. This enables more efficient current distribution to multiple chargers.",

            "configuration_mode": "Expert settings",
            "configuration_mode_muted": "",

            "table_charger_name": "Display name",
            "table_charger_host": "Host",

            "add_charger_title": "Add charger",
            "add_charger_name": "Display name",
            "add_charger_host": "IP address or hostname",
            "add_charger_found": "Discovered chargers",
            "add_charger_count": /*SFN*/(x: number, max: number) => x + " of " + max + " chargers configured"/*NF*/,

            "edit_charger_title": "Edit charger",
            "edit_charger_name": "Display name",
            "edit_charger_host": "IP address or hostname",

            "multi_broadcast_modal_title": "Reserved, multi- or broadcast ip addresses found",
            "multi_broadcast_modal_body": "These reserved, multi- or broadcast ip addresses were detected: ",
            "multi_broadcast_modal_body_end": "Are you sure that you want to save the settings?",
            "multi_broadcast_modal_save": "Yes",
            "multi_broadcast_modal_cancel": "Cancel",

            "scan_error_1": "Firmware version incompatible",
            "scan_error_2": "Charge management disabled",

            "mode_disabled": "Disabled",
            "mode_manager": "Charge manager",
            "mode_managed": "Externally controlled",

            "managed_boxes": "Managed chargers",

            "host_exists": "Host already exists"
        },
        "automation": {
          "charge_manager_wd": "Charge manager watchdog triggered",
          "automation_trigger_text": <>When the <b>charge manager watchdog</b> gets triggered, </>,
          "set_charge_manager": "Set available current for charge manager",
          "automation_action_text": /*FFN*/(current: string) => {
            return (
              <>
                set the current that is available for the <b>charge manager</b> to <b>{current} A</b>.
              </>
            );
          }/*NF*/,
          "max_current": "Maximum current"
        },
        "script": {
            "charge_state_0": "No vehicle connected",
            "charge_state_1": "Waiting for local release",
            "charge_state_2": "Waiting for manager release",
            "charge_state_3": "Charging cleared",
            "charge_state_4": "Charging",
            "charge_state_5": "Error",
            "charge_state_6": "Charging done",

            "charge_error_type_management": "Management error",
            "charge_error_type_client": "Charger error",

            "charge_error_0": "OK",
            "charge_error_1": "Communication error",
            "charge_error_2": "Packet header invalid or firmware incompatible",
            "charge_error_3": "Charge management deactivated",
            "charge_error_128": "Charger unreachable",
            "charge_error_129": "Charge controller unreachable",
            "charge_error_130": "Charge controller does not react",

            "charge_error_192": "OK",
            "charge_error_194": "Switch error",
            "charge_error_195": "DC fault protector error",
            "charge_error_196": "Contactor error",
            "charge_error_197": "Vehicle communication error",

            "charge_state_blocked_by_other_box": "Blocked",
            "charge_state_blocked_by_other_box_details": "Error with another charger",

            "ampere_allocated": "A allocated",
            "ampere_supported": "A supported",

            "last_update_prefix": "Unavailable for",
            "last_update_suffix": "",
            "save_failed": "Failed to save the charge manager settings.",
            "set_available_current_failed": "Failed to set the available current",

            "reboot_content_changed": "charge manager settings",

            "scan_failed": "Scan failed",

            "mode_explainer_0": "This WARP Charger is not part of a charge management group.",
            "mode_explainer_1": "This WARP Charger is part of a charge management group with other WARP Chargers and/or a WARP Energy Manager. Another device controls this group to make sure the configured current is never exceeded.",
            "mode_explainer_2": "This WARP Charger controls a charge management group with othter WARP Chargers to make sure the configured current is never exceeded."
        }
    }
}
