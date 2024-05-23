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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Fragment, ComponentChild } from "preact";
import { ConfigComponent } from "../../ts/components/config_component";
import { Table, TableRow } from "../../ts/components/table";
import { ConfigForm } from "../../ts/components/config_form";
import { InputSelect } from "../../ts/components/input_select";
import { FormRow } from "../../ts/components/form_row";
import { __ } from "../../ts/translation";
import { AutomationTriggerID } from "./automation_trigger_id.enum";
import { AutomationActionID } from "./automation_action_id.enum";
import { Task, AutomationTriggerComponents, AutomationActionComponents } from "./types";
import { plugins_init } from "./plugins";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Tool } from "react-feather";

export function AutomationNavbar() {
    return <NavbarItem name="automation" module="automation" title={__("automation.navbar.automation")} symbol={<Tool />} />;
}

const MAX_RULES = 14;

type AutomationState = {
    displayed_trigger: number;
    displayed_action: number;
    edit_task: Task;
    registered_triggers: number[];
    registered_actions: number[];
    enabled_triggers: number[];
    enabled_actions: number[];
};

let automation_trigger_components: AutomationTriggerComponents = {};
let automation_action_components: AutomationActionComponents = {};

export class Automation extends ConfigComponent<"automation/config", {}, AutomationState> {
    constructor() {
        super('automation/config',
              __("automation.script.save_failed"),
              __("automation.script.reboot_content_changed"), {
                  tasks: [],
                  displayed_trigger: AutomationTriggerID.None,
                  displayed_action: AutomationActionID.None,
                  edit_task: {
                      trigger: [AutomationTriggerID.None, null],
                      action: [AutomationActionID.None, null]
                  }
             });

        util.addApiEventListener("automation/state", () => {
            let state = API.get("automation/state");

            this.setState({
                registered_triggers: state.registered_triggers,
                registered_actions: state.registered_actions,
                enabled_triggers: state.enabled_triggers,
                enabled_actions: state.enabled_actions,
            })
        });
    }

    createSelectors() {
        let trigger: [string, string][] = [];
        for (let i in automation_trigger_components) {
            if (this.state.registered_triggers.indexOf(parseInt(i)) < 0) {
                continue;
            }

            let trigger_component = automation_trigger_components[i];
            let name = trigger_component.name;

            if (this.state.enabled_triggers.indexOf(parseInt(i)) < 0) {
                name += " [";
                name += trigger_component.get_disabled_reason ? trigger_component.get_disabled_reason(this.state.edit_task.trigger) : __("automation.content.trigger_disabled");
                name += "]";
            }

            trigger.push([i, name]);
        }

        let action: [string, string][] = [];
        for (let i in automation_action_components) {
            if (this.state.registered_actions.indexOf(parseInt(i)) < 0) {
                continue;
            }

            let action_component = automation_action_components[i];
            let name = action_component.name;

            if (this.state.enabled_actions.indexOf(parseInt(i)) < 0) {
                name += " [";
                name += action_component.get_disabled_reason ? action_component.get_disabled_reason(this.state.edit_task.action) : __("automation.content.action_disabled");
                name += "]";
            }

            action.push([i, name]);
        }

        let triggerSelector: ComponentChild[] = [
            <FormRow key="trigger_row" label={__("automation.content.condition")}>
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
            triggerSelector = triggerSelector.concat(<div key={`trigger_config_${this.state.displayed_trigger}`}>{trigger_config}</div>);
        }

        triggerSelector = triggerSelector.concat(<hr key="trigger_action_separator"/>);

        let actionSelector: ComponentChild[] = [
            <FormRow key="action_row" label={__("automation.content.action")}>
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
            actionSelector = actionSelector.concat(<div key={`action_config_${this.state.displayed_action}`}>{action_config}</div>);
        }

        const preview = [];
        const trigger_component = automation_trigger_components[this.state.displayed_trigger];
        if (trigger_component) {
            if (this.state.enabled_triggers.indexOf(this.state.displayed_trigger) < 0) {
                let reason = trigger_component.get_disabled_reason ? trigger_component.get_disabled_reason(this.state.edit_task.trigger) : __("automation.content.trigger_disabled");
                preview.push(<span class="text-danger" key={`trigger_disabled_reason_${this.state.displayed_trigger}`}>[{reason}]</span>);
                preview.push(" ");
            }

            preview.push(<span key={`trigger_preview_${this.state.displayed_trigger}`}>{trigger_component.get_table_children(this.state.edit_task.trigger)}</span>);
        }

        const action_component = automation_action_components[this.state.displayed_action];
        if (action_component) {
            if (preview.length > 0) {
                preview.push(<br key="preview_trigger_action_separator"/>);
            }

            if (this.state.enabled_actions.indexOf(this.state.displayed_action) < 0) {
                let reason = action_component.get_disabled_reason ? action_component.get_disabled_reason(this.state.edit_task.action) : __("automation.content.action_disabled");
                preview.push(<span class="text-danger" key={`action_disabled_reason_${this.state.displayed_action}`}>[{reason}]</span>);
                preview.push(" ");
            }

            preview.push(<span key={`action_preview_${this.state.displayed_trigger}`}>{action_component.get_table_children(this.state.edit_task.action)}</span>);
        }

        if (preview.length === 0) {
            return triggerSelector.concat(actionSelector);
        }

        return triggerSelector.concat(actionSelector).concat(<hr key="action_preview_separator"/>).concat(<FormRow label={__("automation.content.preview")}><div class="form-control" style="height: unset;">{preview}</div></FormRow>);
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

            const trigger_disabled = this.state.enabled_triggers.indexOf(trigger_id) < 0
                                     ? (<span class="text-danger">[{trigger_component.get_disabled_reason ? trigger_component.get_disabled_reason(task.trigger) : __("automation.content.trigger_disabled")}]</span>)
                                     : undefined;
            const action_disabled = this.state.enabled_actions.indexOf(action_id) < 0
                                    ? (<span class="text-danger">[{action_component.get_disabled_reason ? action_component.get_disabled_reason(task.action) : __("automation.content.action_disabled")}]</span>)
                                    : undefined;

            let row: TableRow = {
                columnValues: [
                    [idx + 1],
                    [<Fragment key={`trigger_${trigger_id}`}>{trigger_disabled}{" "}{trigger_children}</Fragment>],
                    [<Fragment key={`actionr_${action_id}`}>{action_disabled}{" "}{action_children}</Fragment>],
                ],
                fieldNames: ["", ""],
                fieldValues: [__("automation.content.rule") + " #" + (idx + 1) as ComponentChild, <div class="pb-3">{trigger_disabled}{" "}{trigger_children}<hr class="my-2"/>{action_disabled}{" "}{action_children}</div>],
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
            return <SubPage name="automation" />;

        return <SubPage name="automation">
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
