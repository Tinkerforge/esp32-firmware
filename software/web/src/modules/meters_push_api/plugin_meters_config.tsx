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

import { h, Fragment, ComponentChildren } from 'preact'
import { __, translate_unchecked } from "../../ts/translation";
import { StateUpdater, useState } from 'preact/hooks';
import { MeterClassID } from "../meters/meters_defs";
import { MeterValueID, METER_VALUE_ITEMS } from "../meters/meter_value_id";
import { MeterConfig } from "../meters/types";
import { Table, TableRow } from "../../ts/components/table";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from '../../ts/components/input_select';

const MAX_VALUES = 100;

export type PushAPIMetersConfig = [
    MeterClassID.PushAPI,
    {
        display_name: string;
        value_ids: number[];
    },
];

function createItems(subset: any) {
    let items: [string, string][] = [];
    for (let key in subset) {
        items.push([key, key]);
    }
    return items;
}

interface MeterValueIDSelectorStage {
    state: string,
    isInvalid: boolean,
}

let current_stage = 0

function getStage(i: number, stages: [MeterValueIDSelectorStage, StateUpdater<MeterValueIDSelectorStage>][], value_ids: any): any {
    if (current_stage === i) {
        current_stage = 0;
        return value_ids;
    } else if (!value_ids[stages[current_stage][0].state]) {
        current_stage = 0;
        return {};
    } else {
        current_stage += 1;
        return getStage(i, stages, value_ids[stages[current_stage - 1][0].state]);
    }
}

function clearStagesFrom(i: number, stages: [MeterValueIDSelectorStage, StateUpdater<MeterValueIDSelectorStage>][]) {
    for (; i < 5; ++i) {
        stages[i][1]({
            state: "",
            isInvalid: false,
        })
    }
}

function reverseLookup(value_id: number) {
    for (let a in METER_VALUE_ITEMS) {
        for (let b in (METER_VALUE_ITEMS as any)[a]) {
            if ((METER_VALUE_ITEMS as any)[a][b] === value_id) {
                return [a, b]
            }
            for (let c in (METER_VALUE_ITEMS as any)[a][b]) {
                if ((METER_VALUE_ITEMS as any)[a][b][c] === value_id) {
                    return [a, b, c]
                }
                for (let d in (METER_VALUE_ITEMS as any)[a][b][c]) {
                    if ((METER_VALUE_ITEMS as any)[a][b][c][d] === value_id) {
                        return [a, b, c, d]
                    }
                    for (let e in (METER_VALUE_ITEMS as any)[a][b][c][d]) {
                        if ((METER_VALUE_ITEMS as any)[a][b][c][d][e] === value_id) {
                            return [a, b, c, d, e]
                        }
                    }
                }
            }
        }
    }
    return []
}

// FIXME: need to fix invalid feedback when editing values.
export function MeterValueIDSelector(state: {value_id: {value_id: number}, value_id_vec: Array<number>, stages: [MeterValueIDSelectorStage, StateUpdater<MeterValueIDSelectorStage>][]}) {
    const stages = state.stages
    const items: [string, string][][] = [];
    for (let i = 0; i < 5; ++i) {
        const stage = getStage(i, stages, METER_VALUE_ITEMS);
        const item = createItems(stage);
        if (item.length === 1) {
            state.value_id.value_id = parseInt(stage["Register"])
            if (state.value_id_vec.find((v) => state.value_id.value_id === v) !== undefined && !stages[i - 1][0].isInvalid) {
                stages[i - 1][1]({
                    state: stages[i - 1][0].state,
                    isInvalid: true,
                });
            }
            items.push([])
            continue
        }
        items.push(item)
    }

    const inputSelects = items.map((item, i) => {
        if (item.length > 1) {
            return <div class={i > 0 ? "mt-2" : ""}>
                    <InputSelect
                        required
                        items={item}
                        onValue={(v) => {
                            const stage = getStage(i, stages, METER_VALUE_ITEMS);
                            const val_as_number = parseInt(stage[v]);
                            if (!isNaN(val_as_number)) {
                                state.value_id.value_id = val_as_number
                            }

                            let invalid = false;
                            if (state.value_id_vec.find((v) => state.value_id.value_id === v) !== undefined) {
                                invalid = true;
                            }
                            stages[i][1]({state: v, isInvalid: invalid});
                            clearStagesFrom(i + 1, stages);
                        }}
                        placeholder={__("meters_push_api.content.placeholder")}
                        value={stages[i][0].state}
                        className = {'form-control' + (stages[i][0].isInvalid ? ' is-invalid' : '')}
                        invalidFeedback={__("meters_push_api.content.invalid_feedback")}
                    />
            </div>
        }
    })

    return <>{inputSelects}</>
}

export function init() {
    return {
        [MeterClassID.PushAPI]: {
            name: __("meters_push_api.content.meter_class"),
            new_config: () => [MeterClassID.PushAPI, {display_name: "", value_ids: new Array<number>()}] as MeterConfig,
            clone_config: (config: MeterConfig) => [config[0], {...config[1]}] as MeterConfig,
            get_edit_children: (config: PushAPIMetersConfig, on_value: (config: PushAPIMetersConfig) => void): ComponentChildren => {
                const value_id_obj = {value_id: -1}
                const stages: [MeterValueIDSelectorStage, StateUpdater<MeterValueIDSelectorStage>][]  = [];

                for (let i = 0; i < 5; ++i) {
                    stages.push(useState<MeterValueIDSelectorStage>({state: "", isInvalid: false}));
                }

                return [<>
                    <FormRow label={__("meters_push_api.content.config_display_name")}>
                        <InputText
                            required
                            maxLength={32}
                            value={config[1].display_name}
                            onValue={(v) => {
                                config[1].display_name = v;
                                on_value(config);
                            }}/>
                    </FormRow>
                    <FormRow label={__("meters_push_api.content.config_value_ids")}>
                        <Table
                            nestingDepth={1}
                            rows={config[1].value_ids.map((value_id) => {
                                const row: TableRow = {
                                    columnValues: [translate_unchecked(`meters.content.value_${value_id}`)],
                                    onRemoveClick: async () => {
                                        on_value([config[0], {display_name: config[1].display_name, value_ids: config[1].value_ids.filter((v) => v !== value_id)}])
                                    },
                                    onEditShow: async () => {
                                        value_id_obj.value_id = value_id;
                                        clearStagesFrom(0, stages);
                                        reverseLookup(value_id).map((v, i) => {
                                            stages[i][1]({
                                                state: v,
                                                isInvalid: false,
                                            });
                                        });
                                    },
                                    onEditSubmit: async () => {
                                        config[1].value_ids.push(value_id_obj.value_id)
                                        on_value(config)
                                    },
                                    onEditGetChildren: () => [
                                        <FormRow label={__("meters_push_api.content.config_value_id")}>
                                            <MeterValueIDSelector value_id={value_id_obj} value_id_vec={config[1].value_ids} stages={stages} />
                                        </FormRow>
                                    ],
                                    editTitle: __("meters_push_api.content.edit_value_title"),
                                }
                                return row
                            })}
                            columnNames={[""]}
                            addEnabled={true}
                            addMessage={__("meters_push_api.content.add_value_count")(config[1].value_ids.length, MAX_VALUES)}
                            addTitle={__("meters_push_api.content.add_value_title")}
                            onAddShow={async () => {
                                value_id_obj.value_id = -1;
                                clearStagesFrom(0, stages);
                            }}
                            onAddGetChildren={() => [
                                <FormRow label={__("meters_push_api.content.config_value_id")}>
                                    <MeterValueIDSelector value_id={value_id_obj} value_id_vec={config[1].value_ids} stages={stages} />
                                </FormRow>
                            ]}
                            onAddSubmit={async () => {
                                config[1].value_ids.push(value_id_obj.value_id)
                                on_value(config)
                            }}/>
                    </FormRow>
                </>];
            },
        },
    };
}
