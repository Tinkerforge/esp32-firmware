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
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { Table, TableModalRow, TableRow } from "../../ts/components/table";
import { InputSelect } from "../../ts/components/input_select";
import { __ } from "../../ts/translation";
import { CronTriggerID, CronActionID } from "./cron_defs";
import { CronAction, CronTrigger, Task, CronTriggerComponents, CronActionComponents } from "./types";
import { plugins_init } from "./plugins";
import { SubPage } from "src/ts/components/sub_page";

const MAX_RULES = 20;

type CronState = {
    displayed_trigger: number,
    displayed_action: number,
    edit_task: Task,
};

let cron_trigger_components: CronTriggerComponents = {};
let cron_action_components: CronActionComponents = {};

export class Cron extends ConfigComponent<"cron/config", {}, CronState> {
    constructor() {
        super('cron/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"), {
                  tasks: [],
                  displayed_trigger: CronTriggerID.None,
                  displayed_action: CronActionID.None,
                  edit_task: {
                      trigger: [CronTriggerID.None, null],
                      action: [CronActionID.None, null]
                  }
             });
    }

    setTriggerFromComponent(update: CronTrigger) {
        let edit_task = this.state.edit_task;
        edit_task.trigger = update;
        this.setState({edit_task: edit_task});
    }

    setActionFromComponent(update: CronAction) {
        let edit_task = this.state.edit_task;
        edit_task.action = update;
        this.setState({edit_task: edit_task});
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
            name: __("cron.content.condition"),
            value: <InputSelect
                        required
                        placeholder={__("cron.content.select")}
                        items={trigger}
                        onValue={(v) => {
                            this.setState({
                                displayed_trigger: parseInt(v),
                                edit_task: {
                                    trigger: cron_trigger_components[parseInt(v)].config_builder(),
                                    action: this.state.edit_task.action
                                }
                            })
                        }}
                        value={this.state.displayed_trigger.toString()}/>
        }];
        if (this.state.displayed_trigger != CronTriggerID.None) {
            const trigger_config = cron_trigger_components[this.state.displayed_trigger].config_component(this, this.state.edit_task.trigger);
            triggerSelector = triggerSelector.concat(trigger_config);
        }
        triggerSelector = triggerSelector.concat({name: null, value: <hr/>});

        let actionSelector: TableModalRow[] = [{
            name: __("cron.content.action"),
            value: <>
            <InputSelect
                        required
                        placeholder={__("cron.content.select")}
                        items={action}
                        onValue={(v) => {
                            this.setState({
                                displayed_action: parseInt(v),
                                edit_task: {
                                    trigger: this.state.edit_task.trigger,
                                    action: cron_action_components[parseInt(v)].config_builder()
                                }
                            });
                        }}
                        value={this.state.displayed_action.toString()}/></>
        }]

        if (this.state.displayed_action != CronActionID.None) {
            const action_config = cron_action_components[this.state.displayed_action].config_component(this, this.state.edit_task.action);
            actionSelector = actionSelector.concat(action_config);
        }

        return triggerSelector.concat(actionSelector);
    }

    assembleTable() {
        let rows: TableRow[] = [];
        this.state.tasks.forEach((task, idx) => {
            let trigger_id: number = task.trigger[0];
            let action_id: number = task.action[0];

            const trigger_component = cron_trigger_components[trigger_id];
            const action_component = cron_action_components[action_id];

            const task_copy: Task = {
                trigger: trigger_component.clone(task.trigger),
                action: action_component.clone(task.action)
            };
            const trigger_row = trigger_component.table_row(task.trigger);
            const action_row = action_component.table_row(task.action);

            let row: TableRow = {
                columnValues: [
                    [idx + 1],
                    [trigger_row],
                    [action_row]
                ],
                fieldNames: ["", ""],
                fieldValues: [__("cron.content.rule") + " #" + (idx + 1) as ComponentChild, <div class="pb-3">{trigger_row}{action_row}</div>],
                onEditShow: async () => {
                    this.setState({
                        displayed_trigger: task.trigger[0] as number,
                        displayed_action: task.action[0] as number,
                        edit_task: {
                            trigger: task_copy.trigger,
                            action: task_copy.action
                        }
                    });
                },
                onEditGetRows: () => {
                    return this.createSelectors();
                },
                onEditSubmit: async () => {
                    this.setState({tasks: this.state.tasks.map((task, k) => k === idx ? this.state.edit_task : task)});
                    this.setDirty(true);
                },
                onRemoveClick: async () => {
                    this.setState({tasks: this.state.tasks.filter((_, k) => idx != k)})
                    this.setDirty(true);
                },
                editTitle: __("cron.content.edit_rule_title")
            };
            rows.push(row);
        })
        return rows
    }

    render(props: {}, state: CronState) {
        if (!util.render_allowed())
            return <></>;

        return <SubPage>
             <ConfigForm
                id="cron-config-form"
                title={__("cron.content.cron")}
                isModified={this.isModified()}
                isDirty={this.isDirty()}
                onSave={this.save}
                onReset={this.reset}
                onDirtyChange={this.setDirty}
                >
                <Table tableTill="md"
                    columnNames={[
                        "#",
                        __("cron.content.condition"),
                        __("cron.content.action")]}
                    rows={this.assembleTable()}
                    addEnabled={this.state.tasks.length < MAX_RULES}
                    addTitle={__("cron.content.add_rule_title")}
                    addMessage={__("cron.content.add_rule_count")(this.state.tasks.length, MAX_RULES)}
                    onAddShow={async () => {
                        this.setState({
                            displayed_trigger: CronTriggerID.None,
                            displayed_action: CronActionID.None,
                            edit_task: {
                                trigger: [CronTriggerID.None, null],
                                action: [CronActionID.None, null]
                            }
                        });
                    }}
                    onAddGetRows={() => {
                        return this.createSelectors()
                    }}
                    onAddSubmit={async () => {
                        this.setState({tasks: this.state.tasks.concat([this.state.edit_task])});
                        this.setDirty(true);
                    }}
                    />
            </ConfigForm>
        </SubPage>
    }
}

render(<Cron />, $("#cron")[0]);

export function init() {
    let result = plugins_init();

    for (let item of result) {
        if (item.trigger_components) {
            for (let i in item.trigger_components) {
                if (cron_trigger_components[i]) {
                    console.log('Cron: Overwriting trigger ID ' + i);
                }

                cron_trigger_components[i] = item.trigger_components[i];
            }
        }

        if (item.action_components) {
            for (let i in item.action_components) {
                if (cron_action_components[i]) {
                    console.log('Cron: Overwriting action ID ' + i);
                }

                cron_action_components[i] = item.action_components[i];
            }
        }
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-cron").prop("hidden", !module_init.cron);
}
