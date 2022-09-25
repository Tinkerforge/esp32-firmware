/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Component, VNode, Fragment } from "preact";
import { PreactConfigPageHeader } from "./preact_config_page_header";

interface ConfigFormState {
    saveDisabled: boolean
    wasValidated: boolean
}

interface ConfigFormProps {
    children: VNode | VNode[]
    id: string
    title: string
    onSave: () => Promise<void>
    onDirtyChange: (dirty: boolean) => void
}

export class ConfigForm extends Component<ConfigFormProps,ConfigFormState> {
    constructor() {
        super();
        this.state = {saveDisabled: true, wasValidated: false}
    }
    submit = (e: Event) => {
        e.preventDefault();
        e.stopPropagation();

        this.setState({saveDisabled: true, wasValidated: false});

        if (!(document.getElementById(this.props.id) as HTMLFormElement).checkValidity()) {
            this.setState({saveDisabled: false, wasValidated: true});
            return;
        }

        this.props.onSave().then(() => {
            this.setState({saveDisabled: true, wasValidated: false});
            this.props.onDirtyChange(false);
        }).catch(() => {
            this.setState({saveDisabled: false, wasValidated: false});
        });
    }

    override render (props: ConfigFormProps, state: Readonly<ConfigFormState>) {
        return (
            <>
                <PreactConfigPageHeader title={props.title} config_form_id={props.id} save_disabled={state.saveDisabled}/>
                <form id={props.id}
                      class={"needs-validation" + (state.wasValidated ? " was-validated" : "")}
                      noValidate
                      onInput={() => {this.setState({saveDisabled: false}); this.props.onDirtyChange(true);}}
                      onSubmit={this.submit}>
                    {props.children}
                </form>
            </>
        )
    }
}
