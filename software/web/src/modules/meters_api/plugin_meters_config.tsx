/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Fragment, Component, ComponentChildren } from 'preact'
import { __, translate_unchecked } from "../../ts/translation";
import * as util from "../../ts/util";
import { MeterClassID } from "../meters/meter_class_id.enum";
import { MeterLocation } from "../meters/meter_location.enum";
import { get_meter_location_items } from "../meters/meter_location";
import { MeterValueID, MeterValueTreeType, METER_VALUE_INFOS, METER_VALUE_TREE } from "../meters/meter_value_id";
import { MeterConfig } from "../meters/types";
import { Table, TableRow } from "../../ts/components/table";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from '../../ts/components/input_select';
import { SwitchableInputSelect } from "../../ts/components/switchable_input_select";
import { PRESET_VALUE_IDS, PRESET_DEFAULT_LOCATIONS } from "./presets";

const MAX_VALUES = 96;

export type APIMetersConfig = [
    MeterClassID.API,
    {
        display_name: string;
        location: number;
        value_ids: number[];
    },
];

interface MeterValueIDSelectorProps {
    value_id: number | null;
    value_ids: number[];
    on_value_id: (value_id: number) => void;
    edit_idx?: number;
}

interface MeterValueIDSelectorState {
    tree_path: string[];
    value_id: number | null;
    is_valid: boolean;
}

export function get_meter_value_id_name(value_id: number) {
    let name = translate_unchecked(`meters.content.value_${value_id}`);
    let name_muted = translate_unchecked(`meters.content.value_${value_id}_muted`);
    let unit = METER_VALUE_INFOS[value_id].unit;

    if (name_muted.length > 0) {
        name += "; " + name_muted;
    }

    if (unit.length > 0) {
        name += " [" + unit + "]";
    }

    return name;
}

export class MeterValueIDSelector extends Component<MeterValueIDSelectorProps, MeterValueIDSelectorState> {
    constructor(props: MeterValueIDSelectorProps) {
        super(props);

        this.state = {
            tree_path: props.value_id !== null ? METER_VALUE_INFOS[props.value_id].tree_path : [],
            value_id: props.value_id,
            is_valid: true,
        } as any;
    }

    tree_to_items(tree: MeterValueTreeType) {
        let items: [string, string][] = [];

        for (let key in tree) {
            let name = translate_unchecked(`meters.content.fragment_${key}`);

            if (typeof tree[key] === 'number') {
                let unit = METER_VALUE_INFOS[tree[key] as MeterValueID].unit;

                if (unit.length > 0) {
                    name += " [" + unit + "]";
                }
            }

            items.push([key, name]);
        }

        return items;
    }

    render() {
        let items: [string, string][][] = [];
        let subtree: MeterValueID | MeterValueTreeType = METER_VALUE_TREE;

        items.push(this.tree_to_items(subtree));

        for (let i = 0; i < this.state.tree_path.length; ++i) {
            subtree = subtree[this.state.tree_path[i]];

            if (typeof subtree !== 'object') {
                break;
            }

            items.push(this.tree_to_items(subtree));
        }

        return <>
            {items.map((item, i) => {
                return <div class={i > 0 ? "mt-2" : ""}>
                    <InputSelect
                        required
                        items={item}
                        onValue={(v) => {
                            let tree_path = this.state.tree_path.slice(0, i);
                            let value_id: number = null;
                            let subtree: MeterValueID | MeterValueTreeType = METER_VALUE_TREE;

                            tree_path[i] = v;

                            for (let k = 0; k < tree_path.length; ++k) {
                                subtree = subtree[tree_path[k]];

                                if (typeof subtree === 'number') {
                                    value_id = subtree;
                                    break;
                                }
                            }

                            let is_valid = true;

                            if (value_id !== null) {
                                let idx = this.props.value_ids.indexOf(value_id);

                                if (idx >= 0 && idx !== this.props.edit_idx) {
                                    is_valid = false;
                                }
                            }

                            this.setState({tree_path: tree_path, value_id: value_id, is_valid: is_valid});

                            if (is_valid) {
                                this.props.on_value_id(value_id);
                            }
                        }}
                        placeholder={__("select")}
                        value={this.state.tree_path[i]}
                        className={'form-control' + (!this.state.is_valid && i == items.length - 1 ? ' is-invalid' : '')}
                        invalidFeedback={__("meters_api.content.invalid_feedback")}
                    />
                </div>
            })}
            {this.state.value_id !== null ?
                <div class="mt-3"><span>{get_meter_value_id_name(this.state.value_id)}</span></div>
                : undefined
            }
            </>;
    }
}

interface MeterValueIDTableProps {
    config: APIMetersConfig;
    on_config: (config: APIMetersConfig) => void;
}

interface MeterValueIDTableState {
    value_id: number;
}

class MeterValueIDTable extends Component<MeterValueIDTableProps, MeterValueIDTableState> {
    constructor() {
        super();

        this.state = {
            value_id: null,
        } as any;
    }

    render() {
        return <Table
            nestingDepth={1}
            rows={this.props.config[1].value_ids.map((value_id, i) => {
                const row: TableRow = {
                    columnValues: [get_meter_value_id_name(value_id)],
                    onRemoveClick: async () => {
                        this.props.on_config(util.get_updated_union(this.props.config, {value_ids: this.props.config[1].value_ids.filter((v, k) => k !== i)}));
                    },
                    onEditShow: async () => {
                        this.setState({value_id: value_id});
                    },
                    onEditSubmit: async () => {
                        this.props.on_config(util.get_updated_union(this.props.config, {value_ids: this.props.config[1].value_ids.map((v, k) => k === i ? this.state.value_id : v)}));
                    },
                    onEditGetChildren: () => [
                        <FormRow label={__("meters_api.content.config_value_id")}>
                            <MeterValueIDSelector value_id={this.state.value_id} value_ids={this.props.config[1].value_ids} on_value_id={
                                (v) => this.setState({value_id: v})
                            } edit_idx={i} />
                        </FormRow>
                    ],
                    editTitle: __("meters_api.content.edit_value_title"),
                }
                return row
            })}
            columnNames={[""]}
            addEnabled={this.props.config[1].value_ids.length < MAX_VALUES}
            addMessage={__("meters_api.content.add_value_count")(this.props.config[1].value_ids.length, MAX_VALUES)}
            addTitle={__("meters_api.content.add_value_title")}
            onAddShow={async () => {
                this.setState({value_id: null});
            }}
            onAddGetChildren={() => [
                <FormRow label={__("meters_api.content.config_value_id")}>
                    <MeterValueIDSelector value_id={this.state.value_id} value_ids={this.props.config[1].value_ids} on_value_id={
                        (value_id) => this.setState({value_id: value_id})
                    } />
                </FormRow>
            ]}
            onAddSubmit={async () => {
                this.props.on_config(util.get_updated_union(this.props.config, {value_ids: this.props.config[1].value_ids.concat([this.state.value_id])}));
            }}/>;
    }
}

interface PresetSelectorProps {
    config: APIMetersConfig,
    on_config: (config: APIMetersConfig) => void
}

interface PresetSelectorState {
    preset_key: string
}

class PresetSelector extends Component<PresetSelectorProps, PresetSelectorState> {
    constructor(props: PresetSelectorProps) {
        super(props);

        let needle = props.config[1].value_ids.toString();
        let preset_key = "none";

        for (let key in PRESET_VALUE_IDS) {
            if (needle == PRESET_VALUE_IDS[key].toString()) {
                preset_key = key;
                break;
            }
        }

        this.state = {
            preset_key: preset_key
        } as any;
    }

    render() {
        let children = [
            <FormRow label={__("meters_api.content.api_meter_preset")}>
                <InputSelect
                    items={[
                        ["none", __("meters_api.content.api_meter_no_preset")],
                        ["pve", __("meters_api.content.meter_type_pv_only")],
                        ["dlm", __("meters_api.content.meter_type_dlm_only")],
                        ["pve_dlm", __("meters_api.content.meter_type_pv_dlm_only")],
                        ["eastron_sdm72", __("meters.script.meter_type_1")],
                        ["eastron_sdm630", __("meters.script.meter_type_2")],
                        ["eastron_sdm72v2", __("meters.script.meter_type_3")]
                    ]}
                    value={this.state.preset_key}
                    onValue={async (preset_key) => {
                        let value_ids = PRESET_VALUE_IDS[preset_key];

                        if (this.props.config[1].value_ids.toString() !== PRESET_VALUE_IDS[this.state.preset_key].toString()) {
                            if (!await util.async_modal_ref.current.show({
                                title: () => __("meters_api.content.override_modal_title"),
                                body: () => __("meters_api.content.override_modal_body"),
                                yes_text: () => __("meters_api.content.override_modal_confirm"),
                                no_text: () => __("meters_api.content.override_modal_cancel"),
                                yes_variant: "danger",
                                no_variant: "secondary",
                                nestingDepth: 2
                            })) {
                                this.setState({preset_key: this.state.preset_key});
                                return;
                            }
                        }

                        this.setState({preset_key: preset_key});
                        this.props.on_config(util.get_updated_union(this.props.config, {value_ids: value_ids, location: PRESET_DEFAULT_LOCATIONS[preset_key]}));
                    }}/>
            </FormRow>,
        ];

        let default_location = PRESET_DEFAULT_LOCATIONS[this.state.preset_key];

        if (default_location == MeterLocation.Unknown) {
            children.push(
                <FormRow label={__("meters_api.content.config_location")}>
                    <InputSelect
                        required
                        items={get_meter_location_items()}
                        placeholder={__("select")}
                        value={this.props.config[1].location.toString()}
                        onValue={(v) => {
                            this.props.on_config(util.get_updated_union(this.props.config, {location: parseInt(v)}));
                        }} />
                </FormRow>);
        }
        else {
            let enable_location_override = default_location != this.props.config[1].location;

            children.push(
                <FormRow label={__("meters_api.content.config_location")}>
                    <SwitchableInputSelect
                        required
                        items={get_meter_location_items()}
                        placeholder={__("select")}
                        value={this.props.config[1].location.toString()}
                        onValue={(v) => {
                            this.props.on_config(util.get_updated_union(this.props.config, {location: parseInt(v)}));
                        }}
                        checked={enable_location_override}
                        onSwitch={() => {
                            this.props.on_config(util.get_updated_union(this.props.config, {location: (enable_location_override ? default_location : MeterLocation.Unknown)}));
                        }}
                        switch_label_active={__("meters_api.content.location_different")}
                        switch_label_inactive={__("meters_api.content.location_matching")}
                        />
                </FormRow>);
        }

        return children;
    }
}

export function init() {
    return {
        [MeterClassID.API]: {
            name: () => __("meters_api.content.meter_class"),
            new_config: () => [MeterClassID.API, {display_name: "", location: MeterLocation.Unknown, value_ids: new Array<number>()}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: APIMetersConfig, on_config: (config: APIMetersConfig) => void): ComponentChildren => {
                return [
                    <FormRow label={__("meters_api.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                on_config(util.get_updated_union(config, {display_name: v}));
                            }}/>
                    </FormRow>,
                    <PresetSelector config={config} on_config={on_config} />,
                    <FormRow label={__("meters_api.content.config_value_ids")}>
                        <MeterValueIDTable config={config} on_config={on_config} />
                    </FormRow>,
                ];
            },
        },
    };
}
