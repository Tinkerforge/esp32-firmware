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
import { MeterValueID, MeterValueTreeType, METER_VALUE_INFOS, METER_VALUE_TREE } from "../meters/meter_value_id";
import { MeterConfig } from "../meters/types";
import { Table, TableRow } from "../../ts/components/table";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from '../../ts/components/input_select';

const MAX_VALUES = 96;

export type APIMetersConfig = [
    MeterClassID.API,
    {
        display_name: string;
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
                        placeholder={__("meters_api.content.placeholder")}
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
    preset: string
}

class PresetSelector extends Component<PresetSelectorProps, PresetSelectorState> {
    presets: Readonly<number[][]> = [
        [],
        [74,209,211,213,214,210,212],
        [1,2,3,13,17,21,39,48,57,122,130,138,83,91,99,353,354,355,365,366,367,7,29,33,74,154,115,356,368,364,209,211,273,275,341,388,4,5,6,8,25,369,370,371,377,378,379,375,380,372,373,374,376,213,277,161,177,193,163,179,195,165,181,197,225,241,257,227,243,259,229,245,261,214,210,212],
        [1,2,3,13,17,21,39,48,57,122,130,138,83,91,99,353,354,355,7,29,33,74,154,115,356,364,209,211,4,5,6,8,25,213,277,214,210,212]
    ];

    constructor(props: PresetSelectorProps) {
        super(props);

        let needle = props.config[1].value_ids.toString();
        let preset = "0";

        for (let i = 0; i < this.presets.length; ++i) {
            if (needle == this.presets[i].toString()) {
                preset = i.toString();
                break;
            }
        }

        this.state = {
            preset: preset
        } as any;
    }

    render() {
        return <>
            <InputSelect
                items={[
                    ["0", __("meters_api.content.api_meter_no_preset")],
                    ["1", __("meters.script.meter_type_1")],
                    ["2", __("meters.script.meter_type_2")],
                    ["3", __("meters.script.meter_type_3")]
                ]}
                value={this.state.preset}
                onValue={async (v) => {

                    let preset = parseInt(v);

                    let value_ids: number[] = isNaN(preset) ? [] : this.presets[preset];

                    if (this.props.config[1].value_ids.toString() !== this.presets[parseInt(this.state.preset)].toString()) {
                        if (!await util.async_modal_ref.current.show({
                            title: __("meters_api.content.override_modal_title"),
                            body: __("meters_api.content.override_modal_body"),
                            yes_text: __("meters_api.content.override_modal_confirm"),
                            no_text: __("meters_api.content.override_modal_cancel"),
                            yes_variant: "danger",
                            no_variant: "secondary",
                            nestingDepth: 2
                        })) {
                            this.setState({preset: this.state.preset});
                            return;
                        }
                    }

                    this.setState({preset: v});
                    this.props.on_config(util.get_updated_union(this.props.config, {value_ids: value_ids}));
                }}/>
        </>
    }
}

export function init() {
    return {
        [MeterClassID.API]: {
            name: __("meters_api.content.meter_class"),
            new_config: () => [MeterClassID.API, {display_name: "", value_ids: new Array<number>()}] as MeterConfig,
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
                    <FormRow label={__("meters_api.content.api_meter_preset")}>
                        <PresetSelector config={config} on_config={on_config} />
                    </FormRow>,
                    <FormRow label={__("meters_api.content.config_value_ids")}>
                        <MeterValueIDTable config={config} on_config={on_config} />
                    </FormRow>,
                ];
            },
        },
    };
}
