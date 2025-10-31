/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { __, removeUnicodeHacks } from "../../ts/translation";
import { toLocaleFixed } from "../../ts/util";
import { CASState } from "modules/charge_manager/cas_state.enum";
import { CASError } from "modules/charge_manager/cas_error.enum";
let x = {
    "charge_manager": {
        "status": {
            "charge_manager": null,
            "not_configured": "Disabled",
            "manager": "Enabled",
            "error": "Error",
            "managed_boxes": "Managed chargers",
            "available_current": "Available current",
            "mode": "charge mode",
            "mode_muted": "Updates the charge mode of all controlled chargers"
        },
        "navbar": {
            "charge_manager_settings": "Charge Management",
            "charge_manager_chargers": "Chargers"
        },
        "content": {
            "charge_manager_settings": "Charge Management",
            "charge_manager_chargers": "Chargers",
            "enable_charge_manager": "Charge management mode",
            "enable_charge_manager_help": /*FFN*/(has_managed_mode: boolean) => <>
                <p>On this page the controlled chargers are configured. The settings applied here affect the {options.WARP_DOC_BASE_URL.length > 0 ? <a href={removeUnicodeHacks(`${options.WARP_DOC_BASE_URL}/docs/tutorials/chargemanagement`)}>charge management</a> : <span>charge management</span>} among the chargers.</p>
                <ul class="mb-0">
                    <li><strong>Disabled:</strong> No charge management is performed. {__("This_device")} operates completely independently.</li>
                    {has_managed_mode ?
                        <li><strong>Externally controlled:</strong> {__("This_device")} is managed by another charge manager. No further settings are required.</li>
                        : undefined}
                    <li><strong>Charge manager / PV excess charging:</strong> {__("This_device")} functions as a charge manager. This also applies when it is solely used for {options.WARP_DOC_BASE_URL.length > 0 ? <a href={removeUnicodeHacks(`${options.WARP_DOC_BASE_URL}/docs/tutorials/pv_excess_charging`)}>PV excess charging</a> : <span>PV excess charging</span>}. Additional settings must be configured.</li>
                </ul>
            </>/*NF*/,
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
            "minimum_current_vehicle_type_zoe": "Renault ZOE R135, ZOE R110, Twingo Z.E. or Smart ED/EQ",
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
            "add_charger_invalid_feedback": "At least one charger must be configured.",
            "add_charger_name": "Display name",
            "add_charger_host": "Hostname or IP address",
            "add_charger_found": "Discovered chargers",
            "add_charger_message": /*SFN*/(have: number, max: number) => `${have} of ${max} chargers configured`/*NF*/,
            "add_charger_rotation": "Phase rotation",
            "charger_rotation_help": <>
                <p>The chargers connection from the perspective of the grid or PV meter or the other chargers. Usually only positive sequence rotations are used.</p>
                <p>A charger that only uses the grid phase L2 is then connected with the phase rotation L231.</p>
                <p>If the rotation of some or all chargers is known, more vehicles can be charged in parallel and PV and grid limits can be maxed out: A charger with unknown phase rotation is assumed to be active on all three phases even if it is only charging on one phase.</p>
            </>,
            "rotation_0": "Unknown",
            "rotation_1": "L123",
            "rotation_2": "L132",
            "rotation_3": "L231",
            "rotation_4": "L213",
            "rotation_5": "L321",
            "rotation_6": "L312",
            "rotation_right": "Positive sequence",
            "rotation_left": "Negative sequence",

            "edit_charger_title": "Edit charger",
            "edit_charger_name": "Display name",
            "edit_charger_host": "Hostname or IP address",
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
            "mode_manager_em_with_ps": "Charge manager / PV excess charging / Phase switching / Externally controlled by EVCC",
            "mode_managed": "Externally controlled",
            "mode_managed_em_with_ps": "Externally controlled by WARP Charger or WARP Energy Manager / Phase switching only",

            "managed_boxes": "Managed chargers",

            "host_exists": "Host already exists",

            "charge_manager_settings_reset_modal_text": "Resetting the list of controlled chargers also deletes the charge manager configuration. Continue?",
            "charge_manager_chargers_reset_modal_text": "Resetting the charge manager configuration also deletes the list of controlled chargers. Continue?",

            "header_load_management": "Dynamic load management",
            "dlm_enabled": "Dynamic load management enabled",
            "dlm_enabled_desc": "Adjusts the power consumption of controlled chargers to avoid exceeding the maximum allowed current on any phase of the grid connection.",
            "dlm_meter_slot_grid_currents": "Energy meter",
            "dlm_meter_slot_grid_currents_none": "No power meter configured",
            "dlm_meter_slot_grid_currents_missing_values": "phase currents are missing; draw minus feed required",
            "dlm_meter_slot_grid_currents_single_phase": "single-phase only, L1",
            "dlm_meter_slot_grid_currents_two_phase": "two-phase only, L1 + L2",
            "dlm_current_limit": "Grid current limit",
            "dlm_current_limit_muted": "connection's fuse rating",
            "dlm_largest_consumer_current": "Largest consumer current",
            "dlm_largest_consumer_current_muted": "per phase",
            "dlm_largest_consumer_current_help": "The largest sudden rise in current draw to be expected at the meter, that the dynamic load management must be able to compensate for quickly (< 30 seconds). Could be caused by a water heater or heat pump for example. The managed chargers do not have to be taken into account here.",
            "dlm_safety_margin_pct": "Additional safety margin",
            "dlm_safety_margin_pct_muted": "Usually no additional margin is necessary.",
            "dlm_safety_margin_pct_help": "The load management will calculate a safety margin based on the grid current limit and the largest consumer current. This setting can be used to increase the safety marging above the calculated value.",

            "managed_disabled": <>To use {__("this_device")} as charge manager, the <a href="#charge_manager_chargers">{__("charge_manager.content.enable_charge_manager")}</a> must be set to “{__("charge_manager.content.mode_manager")}”.</>,

            "charge_manager_debug": "Debug",
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
            "target_constant_current_muted": "Chargers will be throttled if this current is exceeded to make sure the largest consumer will not exceed the expected peak current.",

            "em_controlled_charger": "Charger for phase switching",
            "em_controlled_charger_muted": "The charger connected to this Energy Manager's contactor.",
            "em_no_ps_charger": "None",
            "em_proxy_warning_not_enough": "The single charger, for which this Energy Manager performs phase switching, must be configured here.",
            "em_proxy_warning_too_many": "The Energy Manager can perform phase switching only for a single charger and there must not be any other chargers connected to the Energy Manager's contactor."
        },
        "automation": {
            "charge_manager_wd": "Charge manager watchdog triggered",
            "automation_trigger_text": <>When the <b>charge manager watchdog</b> gets triggered, </>,
            "set_charge_manager": "Set available current for charge manager",
            "automation_action_text": /*FFN*/(current: string) => <>set the current that is available for the <b>charge manager</b> to <b>{current} A</b>.</>/*NF*/,
            "max_current": "Maximum current",

            "charge_mode_switch": "Switch charge mode",
            "charge_mode": "Charge mode",
            "charge_mode_switch_action_text": /*FFN*/(mode: number, default_mode: number) => {
                const modes = ["Fast", "Disabled", "PV-Excess", "Min + PV"];

                let ret = <></>;
                switch (mode) {
                    case 0:
                        ret = <><b>Fast</b></>
                        break;

                    case 1:
                        ret = <><b>Disabled</b></>
                        break;

                    case 2:
                        ret = <><b>PV-Excess</b></>
                        break;

                    case 3:
                        ret = <><b>Min + PV</b></>
                        break;

                    default:
                        ret = <><b>Default mode ({modes[default_mode]})</b></>
                        break;
                }
                return <>switch charge mode to {ret}.</>
            }/*NF*/

        },
        "script": {
            "charge_state": /*SFN*/(s: CASState) => {
                switch (s) {
                    case CASState.NoVehicle: return "No vehicle connected";
                    case CASState.UserBlocked: return "Waiting for local release";
                    case CASState.ManagerBlocked: return "Waiting for manager release";
                    case CASState.CarBlocked: return "Charging cleared";
                    case CASState.Charging: return "Charging";
                    case CASState.Error: return "Error";
                    case CASState.Charged: return "Charging done";
                }
            }/*NF*/,

            "charge_error": /*SFN*/(e: CASError) => {
                switch (e) {
                    case CASError.OK: return "OK";
                    case CASError.InvalidHeader: return "Packet header invalid or firmware incompatible";
                    case CASError.NotManaged: return "Charge management disabled";
                    case CASError.ChargerUnreachable: return "Charger unreachable";
                    case CASError.EVSEUnreachable: return "Charge controller unreachable";
                    case CASError.EVSENonreactive: return "Charge controller does not react";
                    case CASError.ClientErrorOK: return "OK";
                    case CASError.ClientErrorSwitch: return "Switch error";
                    case CASError.ClientErrorDCFault: return "DC fault protector error";
                    case CASError.ClientErrorContactor: return "Contactor error";
                    case CASError.ClientErrorCommunication: return "Vehicle communication error";
                }
            }/*NF*/,

            "charge_error_type_management": "Management error",
            "charge_error_type_client": "Charger error",

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

            "mode_explainer_0": <>{__("This_device")} is not part of a charge management group with other WARP Chargers and/or a WARP Energy Manager. PV excess charging is disabled.</>,
            "mode_explainer_1": <>{__("This_device")} is part of a charge management group with other WARP Chargers and/or a WARP Energy Manager. Another device controls this group to make sure the configured current is never exceeded.</>,
            "mode_explainer_2": <>{__("This_device")} either uses the PV excess charging or controls a charge management group with other WARP Chargers to make sure the configured current is never exceeded.</>,

            "mode_explainer_0_em": <>{__("This_device")} does not control any chargers.</>,
            "mode_explainer_2_em": <>{__("This_device")} controls one or more chargers.</>,

            "mode_explainer_0_em_with_ps": <>{__("This_device")} does not control any chargers. Phase switching is disabled.</>,
            "mode_explainer_1_em_with_ps": <>{__("This_device")} is controlled by another WARP Energy Manager or WARP Charger and performs phase switching for a single controlled charger that is connected to its contactor.</>,
            "mode_explainer_2_em_with_ps": <>{__("This_device")} controls one or more WARP Chargers and can perform phase switching for a single controlled charger that is connected to its contactor.</>,

            "mode_change_failed": "Failed to change charge mode.",

            "phase": /*SFN*/ (phase: number) => phase == 0 ? 'PV excess' : `phase L${phase}`/*NF*/,
            "phase_current": /*SFN*/ (phase: number) => phase == 0 ? 'PV excess' : `L${phase} phase current`/*NF*/,
            "for_timespan": /*SFN*/ (timespan: string) => `for ${timespan}`/*NF*/,
            "until_alloc": `until next allocation`,

            "off": "Off",
            "current": "Current",
            "d_none": "No decision made",

            "zpd_yes_waiting_for_rotation": /*SFN*/ (timespan: string) => `Waiting ${timespan} for rotation` /*NF*/,
            "zpd_yes_not_active": "Vehicle has stopped charging",
            "zpd_yes_rotated_for_b1": "Deactivated for waiting charger",
            "zpd_yes_rotated_for_higher_prio": "Deactivating for charger with higher priority (charge mode)",
            "zpd_yes_phase_overload": /*SFN*/ (phase: number, overload: number) => `${__("charge_manager.script.phase")(phase)} overloaded by ${overload} mA` /*NF*/,
            "zpd_yes_pv_excess_overload": /*SFN*/ (overload: number) => `PV excess overloaded by ${overload} mA` /*NF*/,
            "zpd_no_cloud_filter_blocks_until": /*SFN*/ (timespan: string, overload: number) => `PV excess overloaded, but cloud filter blocks shutdown ${timespan}` /*NF*/,
            "zpd_no_hysteresis_blocks_until": /*SFN*/ (timespan: string) => `PV excess overloaded, but hysteresis blocks shutdown ${timespan}` /*NF*/,

            "pd_yes_welcome_charge_until": /*SFN*/ (timespan: string) => `Welcome charge ${timespan}` /*NF*/,
            "pd_no_phase_minimum": /*SFN*/ (timespan: string, phase: number, required: number, minimum: number) => `Waiting ${timespan} for more ${__("charge_manager.script.phase_current")(phase)}: Would have been overloaded recently by ${required - minimum} mA.` /*NF*/,
            "pd_no_phase_improvement": /*SFN*/ (timespan: string, phase: number, allocable: number, minimum: number) => `Waiting ${timespan} for more ${__("charge_manager.script.phase_current")(phase)}.` /*NF*/,
            "pd_yes_waking_up": "Attempting to wake up vehicle",
            "pd_yes": "Activated normally",
            "pd_no_hysteresis_blocks_until": /*SFN*/ (timespan: string) => `Hysteresis blocks ${timespan}` /*NF*/,

            "opd_no_force_3p_until": /*SFN*/ (timespan: string) => `Only three-phase charging allowed ${timespan} because of recent phase switch` /*NF*/,
            "opd_no_fixed_3p": "Charger is connected with all three phases and does not support phase switch",
            "opd_yes_switched_to_fixed_1p": "Charger detected vehicle charging on one phase only",

            "tpd_no_force_1p_until": /*SFN*/ (timespan: string) => `Only one-phase charging allowed ${timespan} because of recent phase switch` /*NF*/,
            "tpd_no_fixed_1p": "Charger is only connected with L1 or detected vehicle charging on L1 only",
            "tpd_yes_unknown_rot_switchable": "Charger has unknown phase rotation and supports phase switch. Preferring three-phase charging",
            "tpd_yes_switched_to_fixed_3p": "Charger detected vehicle charging on all three phases",

            "cd_minimum": "Minimum current",
            "cd_enable_not_charging": "Enable current",
            "cd_phase_limit": "Phase limit",
            "cd_requested": "Vehicle limit",
            "cd_fair": "Fair current",
            "cd_guaranteed_pv": "Guaranteed PV power",
            "cd_fast_ramp_up": "Detecting vehicle limit",
            "cd_supported_by_charger": "Charger limit",
            "cd_left_over": "Left over current"
        }
    }
}
