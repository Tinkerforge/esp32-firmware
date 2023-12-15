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

import { Fragment, render, h, ComponentChild, toChildArray } from "preact";
import { ConfigComponent } from "../../ts/components/config_component";
import { Table, TableRow } from "../../ts/components/table";
import { ConfigForm } from "../../ts/components/config_form";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { __ } from "../../ts/translation";
import { CronTriggerID, CronActionID } from "./cron_defs";
import { Task, CronTriggerComponents, CronActionComponents } from "./types";
import { plugins_init } from "./plugins";
import { SubPage } from "src/ts/components/sub_page";

const MAX_RULES = 14;

type CronState = {
    displayed_trigger: number;
    displayed_action: number;
    edit_task: Task;
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

        let triggerSelector: ComponentChild[] = [
            <FormRow label={__("cron.content.condition")}>
                <InputSelect
                    required
                    placeholder={__("cron.content.select")}
                    items={trigger}
                    onValue={(v) => {
                        this.setState({
                            displayed_trigger: parseInt(v),
                            edit_task: {
                                trigger: cron_trigger_components[parseInt(v)].new_config(),
                                action: this.state.edit_task.action
                            },
                        });
                    }}
                    value={this.state.displayed_trigger.toString()}
                />
            </FormRow>,
        ];

        if (this.state.displayed_trigger != CronTriggerID.None) {
            const trigger_config = cron_trigger_components[this.state.displayed_trigger].get_edit_children(this.state.edit_task.trigger, (trigger) => {
                this.setState({edit_task: {...this.state.edit_task, trigger: trigger}});
            });
            triggerSelector = triggerSelector.concat(toChildArray(trigger_config));
        }

        triggerSelector = triggerSelector.concat(<hr />);

        let actionSelector: ComponentChild[] = [
            <FormRow label={__("cron.content.action")}>
                <InputSelect
                    required
                    placeholder={__("cron.content.select")}
                    items={action}
                    onValue={(v) => {
                        this.setState({
                            displayed_action: parseInt(v),
                            edit_task: {
                                trigger: this.state.edit_task.trigger,
                                action: cron_action_components[parseInt(v)].new_config()
                            }
                        });
                    }}
                    value={this.state.displayed_action.toString()}
                />
            </FormRow>,
        ];

        if (this.state.displayed_action != CronActionID.None) {
            const action_config = cron_action_components[this.state.displayed_action].get_edit_children(this.state.edit_task.action, (action) => {
                this.setState({edit_task: {...this.state.edit_task, action: action}});
            });
            actionSelector = actionSelector.concat(toChildArray(action_config));
        }

        const preview = [];
        const trigger_children = cron_trigger_components[this.state.displayed_trigger];
        if (trigger_children) {
            preview.push(trigger_children.get_table_children(this.state.edit_task.trigger));
        }
        const action_children = cron_action_components[this.state.displayed_action];
        if (action_children) {
            preview.push(action_children.get_table_children(this.state.edit_task.action));
        }
        if (preview.length === 0) {
            return triggerSelector.concat(actionSelector);
        }
        return triggerSelector.concat(actionSelector).concat(<hr/>).concat(<div class="pb-3">{preview}</div>);
    }

    assembleTable() {
        let rows: TableRow[] = [];
        this.state.tasks.forEach((task, idx) => {
            let trigger_id: number = task.trigger[0];
            let action_id: number = task.action[0];

            const trigger_component = cron_trigger_components[trigger_id];
            const action_component = cron_action_components[action_id];

            const task_copy: Task = {
                trigger: trigger_component.clone_config(task.trigger),
                action: action_component.clone_config(task.action),
            };
            const trigger_children = trigger_component.get_table_children(task.trigger);
            const action_children = action_component.get_table_children(task.action);

            let row: TableRow = {
                columnValues: [
                    [idx + 1],
                    [trigger_children],
                    [action_children],
                ],
                fieldNames: ["", ""],
                fieldValues: [__("cron.content.rule") + " #" + (idx + 1) as ComponentChild, <div class="pb-3">{trigger_children}{action_children}</div>],
                onEditShow: async () => {
                    this.setState({
                        displayed_trigger: task.trigger[0] as number,
                        displayed_action: task.action[0] as number,
                        edit_task: {
                            trigger: task_copy.trigger,
                            action: task_copy.action,
                        },
                    });
                },
                onEditGetChildren: () => this.createSelectors(),
                onEditSubmit: async () => {
                    this.setState({tasks: this.state.tasks.map((task, k) => k === idx ? this.state.edit_task : task)});
                    this.setDirty(true);
                },
                onRemoveClick: async () => {
                    this.setState({tasks: this.state.tasks.filter((_, k) => idx != k)})
                    this.setDirty(true);
                },
                editTitle: __("cron.content.edit_rule_title"),
            };
            rows.push(row);
        });
        return rows;
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
                    onAddGetChildren={() => this.createSelectors()}
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
