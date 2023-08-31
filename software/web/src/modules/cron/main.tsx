/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import { Fragment, render, h, ComponentChild } from "preact";
import { ConfigComponent } from "src/ts/components/config_component";
import { ConfigForm } from "src/ts/components/config_form";
import { Table, TableModalRow, TableRow } from "src/ts/components/table";
import { cron_action, cron_action_components, cron_trigger, cron_trigger_components, Task } from "./api";
import { InputSelect } from "src/ts/components/input_select";
import { __ } from "src/ts/translation";
import "./sideeffects"

type CronState = {
    edit_task: Task,
    displayed_trigger: number,
    displayed_action: number
};

export class Cron extends ConfigComponent<'cron/config', {}, CronState> {
    constructor() {
        super('cron/config',
        __("charge_manager.script.save_failed"),
        __("charge_manager.script.reboot_content_changed"));
        this.state = {
            tasks: [],
            edit_task: {
                trigger: [0, null] as any as cron_trigger,
                action: [0, null] as any as cron_action
            },
            displayed_action: 0,
            displayed_trigger: 0
        }
    }

    setTriggerFromComponent(update: cron_trigger) {
        let edit_task = this.state.edit_task;
        edit_task.trigger = update;
        this.setState({edit_task: edit_task});
    }

    setActionFromComponent(update: cron_action) {
        let edit_task = this.state.edit_task;
        edit_task.action = update;
        this.setState({edit_task: edit_task});
    }

    hackToAllowSave() {
        document.getElementById("cron-config-form").dispatchEvent(new Event('input'));
    }

    default(object: any) {
        for (let k in object) {

        }
    }

    createSelectors() {
        let trigger: [string, string][] = [];
        for (let i in cron_trigger_components) {
            const entry: [string, string] = [i, cron_trigger_components[i].name]
            if (cron_trigger_components[i].require_feature && !API.hasFeature(cron_trigger_components[i].require_feature))
                continue;
            trigger.push(entry);
        }

        let action: [string, string][] = [];
        for (let i in cron_action_components) {
            const entry: [string, string] = [i, cron_action_components[i].name];
            if (cron_action_components[i].require_feature && !API.hasFeature(cron_action_components[i].require_feature))
                continue;
            action.push(entry);
        }

        let triggerSelector: TableModalRow[] = [{
            name: __("cron.content.condition_category"),
            value: <InputSelect
                        placeholder={__("cron.content.select")}
                        items={trigger}
                        onValue={(v) => {
                            if (v == "0") {
                                this.setState({displayed_trigger: this.state.displayed_trigger})
                                return;
                            }
                            this.setState(
                            {
                                displayed_trigger: Number(v),
                                edit_task: {
                                    trigger: cron_trigger_components[Number(v)].config_builder(),
                                    action: this.state.edit_task.action
                                }
                            })
                        }}
                        value={this.state.displayed_trigger.toString()}/>
        }];
        if (this.state.displayed_trigger != 0) {
            const trigger_config = cron_trigger_components[this.state.displayed_trigger].config_component(this, this.state.edit_task.trigger);
            triggerSelector = triggerSelector.concat(trigger_config);
        }
        triggerSelector = triggerSelector.concat({name: null, value: <hr/>});

        let actionSelector: TableModalRow[] = [{
            name: __("cron.content.action_category"),
            value: <>
            <InputSelect
                        items={action}
                        placeholder={__("cron.content.select")}
                        onValue={(v) => {
                            if (v == "0") {
                                this.setState({displayed_action: this.state.displayed_action});
                                return;
                            }
                            this.setState(
                            {
                                displayed_action: Number(v),
                                edit_task: {
                                    action: cron_action_components[Number(v)].config_builder(),
                                    trigger: this.state.edit_task.trigger
                                }
                            });
                        }}
                        value={this.state.displayed_action.toString()}/></>
        }]

        if (this.state.displayed_action != 0) {
            const action_config = cron_action_components[this.state.displayed_action].config_component(this, this.state.edit_task.action);
            actionSelector = actionSelector.concat(action_config);
        }

        return triggerSelector.concat(actionSelector);
    }

    assembleTable() {
        let rows: TableRow[] = [];
        this.state.tasks.forEach((task, idx) => {
            let action: number;
            let action_obj: any;
            let trigger: number;
            let trigger_obj: any;
            if (isNaN(Number(task.action[0]))) {
                action = task.action[0][0];
                action_obj = task.action[0][1];
            } else {
                action = Number(task.action[0]);
                action_obj = (task.action as any)[1];
            }

            if (isNaN(Number(task.trigger[0]))) {
                trigger = task.trigger[0][0];
                trigger_obj = task.trigger[0][1];
            } else {
                trigger = Number(task.trigger[0]);
                trigger_obj = (task.trigger as any)[1];
            }


            const ActionComponent = cron_action_components[action];
            const TriggerComponent = cron_trigger_components[trigger];

            const task_copy = {
                action: [action, {...action_obj}] as any as cron_action,
                trigger: [trigger, {...trigger_obj}] as any as cron_trigger
            };
            const trigger_row = TriggerComponent.table_row(task.trigger);
            const action_row = ActionComponent.table_row(task.action);
            trigger_row.fieldNames = [__("cron.content.condition")].concat(trigger_row.fieldNames);
            trigger_row.fieldValues = [TriggerComponent.name as ComponentChild].concat(trigger_row.fieldValues);
            trigger_row.fieldNames.push(null);
            trigger_row.fieldValues.push(<hr/>);

            action_row.fieldNames = [__("cron.content.action")].concat(action_row.fieldNames);
            action_row.fieldValues = [ActionComponent.name as ComponentChild].concat(action_row.fieldValues);

            let row: TableRow = {
                columnValues: [
                    [idx + 1],
                    [TriggerComponent.name],
                    [trigger_row.text],
                    [ActionComponent.name],
                    [action_row.text]
                ],
                fieldNames: [""].concat(trigger_row.fieldNames.concat(action_row.fieldNames)),
                fieldValues: [__("cron.content.task") + " #" + (idx + 1) as ComponentChild].concat(trigger_row.fieldValues.concat(action_row.fieldValues)),
                onEditStart: async () => {
                    this.setState(
                        {
                            edit_task: {
                                action: task_copy.action,
                                trigger: task_copy.trigger
                            },
                            displayed_trigger: task.trigger[0] as any as number,
                            displayed_action: task.action[0] as any as number
                        })
                },
                onEditGetRows: () => {
                    return this.createSelectors();
                },
                onEditCommit: async () => {
                    if (this.state.displayed_action == 0 || this.state.displayed_trigger == 0) {
                        return;
                    }

                    this.setState({tasks: this.state.tasks.map((task, k) => k === idx ? this.state.edit_task : task)});
                    this.hackToAllowSave();
                },
                onEditAbort: async () => {},
                onRemoveClick: async () => {
                    this.setState({tasks: this.state.tasks.filter((_, k) => idx != k)})
                    this.hackToAllowSave();
                },
                editTitle: __("cron.content.edit_rule")
            };
            rows.push(row);
        })
        return rows
    }

    render(props: {}, state: CronState) {
        if (!util.render_allowed())
            return <></>;

        return <ConfigForm
                id="cron-config-form"
                title={__("cron.content.cron")}
                isModified={this.isModified()}
                onSave={this.save}
                onReset={this.reset}
                onDirtyChange={(d) => this.ignore_updates = d}
                >
                    <div class="col-xl-12">
                    <Table tableTill="md"
                        columnNames={[
                            "#",
                            __("cron.content.condition"),
                            "",
                            __("cron.content.action"),
                            ""]}
                        rows={this.assembleTable()}
                        addEnabled={this.state.tasks.length < 20}
                        addTitle={__("cron.content.add_rule")}
                        addMessage={__("cron.content.add_rule")}
                        onAddStart={async () => this.setState(
                            {
                                edit_task: {trigger: [[0, {}]], action: [[0, {}]]},
                                displayed_action: 0,
                                displayed_trigger: 0
                            })}
                        onAddGetRows={() => {
                            return this.createSelectors()
                        }}
                        onAddCommit={async () => {
                            if (this.state.displayed_action == 0 || this.state.displayed_trigger == 0) {
                                return;
                            }
                            this.setState({tasks: this.state.tasks.concat([this.state.edit_task])});
                            this.hackToAllowSave();
                        }}

                        />
                    </div>
            </ConfigForm>
    }
}

render(<Cron/>, $('#cron')[0]);

export function init() {
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-cron').prop('hidden', !module_init.cron);
}

// cron_trigger_components[0] = {
//     config_builder: () => [0 as any, {}],
//     config_component: () => [],
//     table_row: () => "",
//     name: __("cron.content.select")
// };

// cron_action_components[0] = {
//     config_builder: () => [0 as any, {}],
//     config_component: () => [],
//     table_row: () => "",
//     name: __("cron.content.select")
// }
