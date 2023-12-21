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
import { AutomationTriggerID, AutomationActionID } from "./automation_defs";
import { Task, AutomationTriggerComponents, AutomationActionComponents } from "./types";
import { plugins_init } from "./plugins";
import { SubPage } from "src/ts/components/sub_page";

const MAX_RULES = 14;

type AutomationState = {
    displayed_trigger: number;
    displayed_action: number;
    edit_task: Task;
};

let automation_trigger_components: AutomationTriggerComponents = {};
let automation_action_components: AutomationActionComponents = {};

export class Automation extends ConfigComponent<"automation/config", {}, AutomationState> {
    constructor() {
        super('automation/config',
              __("charge_manager.script.save_failed"),
              __("charge_manager.script.reboot_content_changed"), {
                  tasks: [],
                  displayed_trigger: AutomationTriggerID.None,
                  displayed_action: AutomationActionID.None,
                  edit_task: {
                      trigger: [AutomationTriggerID.None, null],
                      action: [AutomationActionID.None, null]
                  }
             });
    }

    createSelectors() {
        let trigger: [string, string][] = [];
        for (let i in automation_trigger_components) {
            const entry: [string, string] = [i, automation_trigger_components[i].name]
            if (automation_trigger_components[i].require_feature && !API.hasFeature(automation_trigger_components[i].require_feature))
                continue;
            trigger.push(entry);
        }

        let action: [string, string][] = [];
        for (let i in automation_action_components) {
            const entry: [string, string] = [i, automation_action_components[i].name];
            if (automation_action_components[i].require_feature && !API.hasFeature(automation_action_components[i].require_feature))
                continue;
            action.push(entry);
        }

        let triggerSelector: ComponentChild[] = [
            <FormRow label={__("automation.content.condition")}>
                <InputSelect
                    required
                    placeholder={__("automation.content.select")}
                    items={trigger}
                    onValue={(v) => {
                        this.setState({
                            displayed_trigger: parseInt(v),
                            edit_task: {
                                trigger: automation_trigger_components[parseInt(v)].new_config(),
                                action: this.state.edit_task.action
                            },
                        });
                    }}
                    value={this.state.displayed_trigger.toString()}
                />
            </FormRow>,
        ];

        if (this.state.displayed_trigger != AutomationTriggerID.None) {
            const trigger_config = automation_trigger_components[this.state.displayed_trigger].get_edit_children(this.state.edit_task.trigger, (trigger) => {
                this.setState({edit_task: {...this.state.edit_task, trigger: trigger}});
            });
            triggerSelector = triggerSelector.concat(toChildArray(trigger_config));
        }

        triggerSelector = triggerSelector.concat(<hr />);

        let actionSelector: ComponentChild[] = [
            <FormRow label={__("automation.content.action")}>
                <InputSelect
                    required
                    placeholder={__("automation.content.select")}
                    items={action}
                    onValue={(v) => {
                        this.setState({
                            displayed_action: parseInt(v),
                            edit_task: {
                                trigger: this.state.edit_task.trigger,
                                action: automation_action_components[parseInt(v)].new_config()
                            }
                        });
                    }}
                    value={this.state.displayed_action.toString()}
                />
            </FormRow>,
        ];

        if (this.state.displayed_action != AutomationActionID.None) {
            const action_config = automation_action_components[this.state.displayed_action].get_edit_children(this.state.edit_task.action, (action) => {
                this.setState({edit_task: {...this.state.edit_task, action: action}});
            });
            actionSelector = actionSelector.concat(toChildArray(action_config));
        }

        const preview = [];
        const trigger_children = automation_trigger_components[this.state.displayed_trigger];
        if (trigger_children) {
            preview.push(trigger_children.get_table_children(this.state.edit_task.trigger));
        }
        const action_children = automation_action_components[this.state.displayed_action];
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

            const trigger_component = automation_trigger_components[trigger_id];
            const action_component = automation_action_components[action_id];

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
                fieldValues: [__("automation.content.rule") + " #" + (idx + 1) as ComponentChild, <div class="pb-3">{trigger_children}{action_children}</div>],
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
                editTitle: __("automation.content.edit_rule_title"),
            };
            rows.push(row);
        });
        return rows;
    }

    render(props: {}, state: AutomationState) {
        if (!util.render_allowed())
            return <></>;

        return <SubPage>
             <ConfigForm
                id="automation-config-form"
                title={__("automation.content.automation")}
                isModified={this.isModified()}
                isDirty={this.isDirty()}
                onSave={this.save}
                onReset={this.reset}
                onDirtyChange={this.setDirty}
                >
                <Table tableTill="md"
                    columnNames={[
                        "#",
                        __("automation.content.condition"),
                        __("automation.content.action")]}
                    rows={this.assembleTable()}
                    addEnabled={this.state.tasks.length < MAX_RULES}
                    addTitle={__("automation.content.add_rule_title")}
                    addMessage={__("automation.content.add_rule_count")(this.state.tasks.length, MAX_RULES)}
                    onAddShow={async () => {
                        this.setState({
                            displayed_trigger: AutomationTriggerID.None,
                            displayed_action: AutomationActionID.None,
                            edit_task: {
                                trigger: [AutomationTriggerID.None, null],
                                action: [AutomationActionID.None, null]
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

render(<Automation />, $("#automation")[0]);

export function init() {
    let result = plugins_init();

    for (let item of result) {
        if (item.trigger_components) {
            for (let i in item.trigger_components) {
                if (automation_trigger_components[i]) {
                    console.log('Automation: Overwriting trigger ID ' + i);
                }

                automation_trigger_components[i] = item.trigger_components[i];
            }
        }

        if (item.action_components) {
            for (let i in item.action_components) {
                if (automation_action_components[i]) {
                    console.log('Automation: Overwriting action ID ' + i);
                }

                automation_action_components[i] = item.action_components[i];
            }
        }
    }
}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $("#sidebar-automation").prop("hidden", !module_init.automation);
}
