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

import { Fragment, render, h, FunctionComponent, Component } from "preact";
import { ConfigComponent } from "src/ts/components/config_component";
import { SubPage } from "src/ts/components/sub_page";
import { ConfigForm } from "src/ts/components/config_form";
import { Table, TableRow } from "src/ts/components/table";
import { type } from "jquery";

type CronConfig = API.getType['cron/config'];

export const cron_trigger: {[key: number]: FunctionComponent} = {0: null};
export const cron_action: {[key: number]: FunctionComponent<{cron: any}>} = {0: null};

export class Cron extends ConfigComponent<'cron/config', {}, CronConfig> {
    constructor() {
        super('cron/config');

    }

    assembleTable() {
        let rows: TableRow[] = [];
        console.log(this.state.tasks);
        this.state.tasks.forEach((task) => {
            const test = task.action[0];
            console.log("Cron action", task.action);
            const ActionComponent = cron_action[Number(test)];
            const TriggerComponent = cron_trigger[Number(task.trigger[0])];
            console.log(ActionComponent);
            // const action_element = <ActionComponent />
            // const trigger_element = <TriggerComponent/>
            let row: TableRow = {
                columnValues: [[<TriggerComponent/>], [<ActionComponent cron={task.action}/>]]
            };
            console.log("Row:" + row);
            rows.push(row);
        })
        console.log("Rows:" + rows)
        return rows
    }

    render(props: {}, state: CronConfig) {
        if (!util.render_allowed())
            return <></>;

        return <SubPage>
            <ConfigForm id="cron-config" title="Cron config" isModified={this.isModified()} onSave={this.save} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                <div class="mb-3">
                    <Table tableTill="md"
                        columnNames={["trigger", "action"]}
                        rows={this.assembleTable()}>

                    </Table>
                </div>

            </ConfigForm>
        </SubPage>
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