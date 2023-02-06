/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

import $ from "../../ts/jq";

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { __ } from "../../ts/translation";

import { h, render, Fragment, Component } from "preact";
import { FormRow    } from "../../ts/components/form_row";
import { InputText  } from "../../ts/components/input_text";
import { PageHeader } from "../../ts/components/page_header";

type EMSDcardState = API.getType['em_sdcard/state'];

export class EMSDcard extends Component<{}, EMSDcardState> {
    constructor() {
        super();

        util.eventTarget.addEventListener('em_sdcard/state', () => {
            this.setState(API.get('em_sdcard/state'));
        });
    }

    render(props: {}, state: Readonly<EMSDcardState>) {
        if (!state || state.manufacturer_id === undefined)
            return (<></>);

        var manufacturer;
        switch(state.manufacturer_id) {
            case 0x01: manufacturer = "Panasonic"; break;
            case 0x02: manufacturer = "Toshiba"; break;
            case 0x03: manufacturer = "SanDisk"; break;
            case 0x1b: manufacturer = "Samsung"; break;
            case 0x1d: manufacturer = "AData"; break;
            case 0x27: manufacturer = "Phison, rebranded"; break;
            case 0x28: manufacturer = "Lexar"; break;
            case 0x31: manufacturer = "Silicon Power"; break;
            case 0x41: manufacturer = "Kingston"; break;
            case 0x74: manufacturer = "Transcend"; break;
            case 0x76: manufacturer = "Patriot"; break;
            case 0x82: manufacturer = "Sony"; break;
            case 0x9c: manufacturer = "Angelbird"; break;
            default:   manufacturer = "Unknown (" + state.manufacturer_id.toString(16) + ")";
        }

        var card_type;
        switch(state.card_type) {
            case 0x01: card_type = "MMC"; break;
            case 0x02: card_type = "SD"; break;
            case 0x04: card_type = "SDSC"; break;
            case 0x0c: card_type = "SDHC/SDXC"; break;
            default:   card_type = "Unknown (0x" + state.card_type.toString(16) + ")";
        }

        var sd_status;
        switch(state.sd_status) {
            case  0: sd_status = "OK"; break;
            case  1: sd_status = "READ_BLOCK_TIMEOUT"; break;
            case  2: sd_status = "WRITE_BLOCK_TIMEOUT"; break;
            case  3: sd_status = "RESPONSE_TIMEOUT"; break;
            case 11: sd_status = "ERROR_INIT_TYPE"; break;
            case 12: sd_status = "ERROR_INIT_VER_OR_VOLTAGE"; break;
            case 13: sd_status = "ERROR_INIT_ACMD41"; break;
            case 14: sd_status = "ERROR_INIT_CMD58"; break;
            case 15: sd_status = "ERROR_INIT_CMD0 (" + __("em_sdcard.content.error_no_card") + "?)"; break;
            case 21: sd_status = "ERROR_CID_START"; break;
            case 22: sd_status = "ERROR_CID_CMD10"; break;
            case 31: sd_status = "ERROR_CSD_START"; break;
            case 32: sd_status = "ERROR_CSD_CMD9"; break;
            case 41: sd_status = "ERROR_COUNT_TO_HIGH"; break;
            default: sd_status = "Unknown error code " + state.sd_status;
        }

        var lfs_status;
        switch(state.lfs_status) {
            case  0: lfs_status = "OK"; break;
            default: lfs_status = "ERROR " + state.lfs_status;
        }

        return (
            <>
                <PageHeader title={__("em_sdcard.content.header")} />

                <FormRow label={__("em_sdcard.content.manufacturer_id")}>
                    <InputText value={manufacturer}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.product_name")}>
                    <InputText value={state.product_name}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.product_rev")}>
                    <InputText value={(state.product_rev >> 4).toString() + '.' + (state.product_rev & 0x3).toString()}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.card_type")}>
                    <InputText value={card_type}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.card_size")}>
                    <InputText value={((state.sector_count * state.sector_size) / 1000000000).toFixed(2) + " GB (" + ((state.sector_count * state.sector_size) / 1073741824).toFixed(2) + " GiB)"}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.sector_size")}>
                    <InputText value={state.sector_size.toString() + " Bytes"}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.sector_count")}>
                    <InputText value={state.sector_count}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.sd_status")}>
                    <InputText value={sd_status}/>
                </FormRow>
                <FormRow label={__("em_sdcard.content.lfs_status")}>
                    <InputText value={lfs_status}/>
                </FormRow>
            </>
        )
    }
}

render(<EMSDcard/>, $('#em_sdcard')[0])

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-em_sdcard').prop('hidden', !module_init.energy_manager);
}
