/* esp32-firmware
 * Copyright (C) 2025 Frederic Henrichs <frederic@tinkerforge.com>
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
import { __ } from "../../ts/translation";
import { h, Component } from "preact";
import { Alert } from "react-bootstrap";
import { StatusSection } from "../../ts/components/status_section";
import { get_status_alerts } from "../../ts/util";
import { FormRow } from "ts/components/form_row";

export class StatusAlertsStatus extends Component<{}, {}> {
    statusAlerts = get_status_alerts();
    constructor() {
        super();
    }

    render(props: {}, state: {}) {
        if (!util.render_allowed())
            return <StatusSection name="status_alert" />;

        return <StatusSection name="status_alert">
            {this.statusAlerts.map((alert) => {
                return <FormRow label={alert.moduleName()}>
                    <Alert
                        variant={alert.variant}
                        className="mb-0 mt-1"
                        onClose={() => {
                            if(alert.onClose) {
                                alert.onClose();
                            }
                            util.remove_status_alert(alert.id);
                        }}
                        dismissible={alert.onClose !== undefined}>
                        {alert.text() as any}
                    </Alert>
                </FormRow>
            })}
        </StatusSection>;
    }
}

export function init() {
}
