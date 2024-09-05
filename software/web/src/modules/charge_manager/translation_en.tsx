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
            "charge_manager_settings": "Charge management",
            "charge_manager_chargers": "Chargers"
        },
        "content": {
            "charge_manager_settings": "Charge management",
            "charge_manager_chargers": "Chargers",
            "enable_charge_manager": "Charge management mode",
            "enable_charge_manager_muted": <><a href="{{{manual_url}}}">see manual for details</a></>,
            "enable_watchdog": "Watchdog enabled",
            "enable_watchdog_muted": "only enable if using the API (not required for normal charge manager use!)",
            "enable_watchdog_desc": "Sets the available current to the default value if it is not updated every 30 seconds",
            "verbose": "Current distribution log enabled",
            "verbose_desc": "Creates log entries whenever current is redistributed",
            "default_available_current": "Default available current",
            "default_available_current_muted": "will be used after charge manager reboot if dynamic load management is not enabled",
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
            "table_charger_rotation": "Phase rotation",

            "add_charger_title": "Add charger",
            "add_charger_name": "Display name",
            "add_charger_host": "IP address or hostname",
            "add_charger_found": "Discovered chargers",
            "add_charger_count": /*SFN*/(x: number, max: number) => x + " of " + max + " chargers configured"/*NF*/,
            "add_charger_rotation": "Phase rotation",
            "charger_rotation_help": <>The chargers connection from the perspective of the grid or PV meter or the other chargers. Usually only positive sequence rotations are used.<br/><br/>A charger that only uses the grid phase L2 is then connected with the phase rotation L231.<br/><br/>If the rotation of some or all chargers is known, more vehicles can be charged in parallel and PV and grid limits can be maxed out: A charger with unknown phase rotation is assumed to be active on all three phases even if it is only charging on one phase.</>,
            "rotation_0": "Unknown",
            "rotation_1": "L123",
            "rotation_2": "L132",
            "rotation_3": "L231",
            "rotation_4": "L213",
            "rotation_5": "L321",
            "rotation_6": "L312",
            "rotation_right": "positive sequence",
            "rotation_left": "negative sequence",
            "add_charger_rotation_select": "Select...",

            "edit_charger_title": "Edit charger",
            "edit_charger_name": "Display name",
            "edit_charger_host": "IP address or hostname",
            "edit_charger_rotation": "Phase rotation",

            "multi_broadcast_modal_title": "Reserved, multi- or broadcast IP address(es) found",
            "multi_broadcast_modal_body": "These reserved, multi- or broadcast IP addresses were detected: ",
            "multi_broadcast_modal_body_end": "Are you sure that you want to save the settings?",
            "multi_broadcast_modal_save": "Yes",
            "multi_broadcast_modal_cancel": "Cancel",

            "scan_error_1": "Firmware version incompatible",
            "scan_error_2": "Charge management disabled",

            "mode_disabled": "Disabled",
            "mode_manager": "Charge manager / PV excess charging",
            "mode_managed": "Externally controlled",

            "managed_boxes": "Managed chargers",

            "host_exists": "Host already exists",

            "charge_manager_settings_reset_modal_text": "Resetting the list of controlled chargers also deletes the charge manager configuration. Continue?",
            "charge_manager_chargers_reset_modal_text": "Resetting the charge manager configuration also deletes the list of controlled chargers. Continue?",

            "header_load_management": "Dynamic load management",
            "dlm_enabled": "Enable dynamic load management",
            "dlm_enabled_desc": "Adjusts the power consumption of controlled chargers to avoid exceeding the maximum allowed current on any phase of the grid connection.",
            "dlm_meter_slot_grid_currents": "Energy meter",
            "dlm_meter_slot_grid_currents_select": "Select...",
            "dlm_meter_slot_grid_currents_none": "No power meter configured",
            "dlm_meter_slot_grid_currents_missing_values": "directional phase currents are missing",
            "dlm_current_limit": "Grid current limit",
            "dlm_current_limit_muted": "connection's fuse rating",
            "dlm_largest_consumer_current": "Largest consumer current",
            "dlm_largest_consumer_current_muted": "per phase",
            "dlm_largest_consumer_current_help": "The largest sudden rise in current draw to be expected at the meter, that the dynamic load management must be able to compensate for quickly (< 30 seconds). Could be caused by a water heater or heat pump for example. The managed chargers do not have to be taken into account here.",
            "dlm_safety_margin_pct": "Additional safety margin",
            "dlm_safety_margin_pct_muted": "Usually no additional margin is necessary.",
            "dlm_safety_margin_pct_help": "The load management will calculate a safety margin based on the grid current limit and the largest consumer current. This setting can be used to increase the safety marging above the calculated value.",

            "managed_disabled": "Charge management settings will be ignored: Charger only controls itself or is controlled externally.",

            "charge_manager_debug": "Debug",
            "protocol": "Energy Manager Protocol",
            "debug_description": "Create protocol",
            "debug_description_muted": "for diagnosing issues",
            "debug_start": "Start",
            "debug_stop": "Stop + Download",

            "internal_state": "Internal state",

            "peak_current": "Short-term allowed peak current",
            "peak_current_muted": "Overload that is compensated by the load management within 30 seconds. 140 % of the Grid current limit.",

            "expected_peak_current": "Expected peak current",
            "expected_peak_current_muted": "Short-term expected peak current including the additional safety margin.",

            "target_constant_current": "Target constant current",
            "target_constant_current_muted": "Chargers will be throttled if this current is exceeded to make sure the largest consumer will not exceed the expected peak current."
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

            "ampere_allocated": "allocated",
            "ampere_supported": "supported",

            "last_update_prefix": "Unavailable for",
            "last_update_suffix": "",
            "save_failed": "Failed to save the charge manager settings.",
            "set_available_current_failed": "Failed to set the available current",

            "reboot_content_changed": "charge manager settings",

            "scan_failed": "Scan failed",

            "mode_explainer_0": "This WARP Charger is not part of a charge management group. PV excess charging is disabled.",
            "mode_explainer_1": "This WARP Charger is part of a charge management group with other WARP Chargers and/or a WARP Energy Manager. Another device controls this group to make sure the configured current is never exceeded.",
            "mode_explainer_2": "This WARP Charger either uses the PV excess charging or controls a charge management group with other WARP Chargers to make sure the configured current is never exceeded."
        }
    }
}
