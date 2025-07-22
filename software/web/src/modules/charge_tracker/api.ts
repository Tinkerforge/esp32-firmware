//#include "module_available.inc"

//#if MODULE_REMOTE_ACCESS_AVAILABLE
import { FileType } from "./file_type.enum";
import { CSVFlavor } from "./csv_flavor.enum";
//#endif

interface Charge {
    timestamp_minutes: number;
    charge_duration: number;
    user_id: number;
    energy_charged: number;
}

//#if MODULE_REMOTE_ACCESS_AVAILABLE
interface remote_upload_config {
    user_filter: number;
    file_type: FileType;
    english: boolean;
    letterhead: string;
    user_id: number;
    csv_delimiter: CSVFlavor;
}
//#endif

export interface config {
    electricity_price: number;
//#if MODULE_REMOTE_ACCESS_AVAILABLE
    remote_upload_configs: remote_upload_config[];
    last_upload_timestamp_min: number
//#endif
}

export interface pdf_letterhead_config {
    letterhead: string;
}

export type last_charges = Charge[];

interface current_charge_base {
    user_id: number;
    meter_start: number;
    evse_uptime_start: number;
    timestamp_minutes: number;
}

interface current_charge_nfc {
    authorization_type: 2;
    authorization_info?: {
        tag_type: number;
        tag_id: string;
    };
}

interface current_charge_nfc_inject {
    authorization_type: 3;
    authorization_info?: {
        tag_type: number;
        tag_id: string;
    };
}

interface current_charge_lost {
    authorization_type: 1;
    authorization_info: null
}

interface current_charge_none {
    authorization_type: 0;
    authorization_info: null
}

export type current_charge = current_charge_base & (current_charge_nfc
                                                    | current_charge_nfc_inject
                                                    | current_charge_lost
                                                    | current_charge_none);

export interface state {
    tracked_charges: number;
    first_charge_timestamp: number;
}

export interface remove_all_charges {
    do_i_know_what_i_am_doing: boolean;
}

export interface pdf {
    api_not_final_acked: boolean;
    english: boolean;
    start_timestamp_min?: number;
    end_timestamp_min?: number;
    user_filter?: number;
    letterhead?: string;
}
