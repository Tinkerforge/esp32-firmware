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

import { h, Component, Fragment, ComponentChildren } from "preact";
import { __ } from "../translation";
import * as util from "../util";
import { PageHeader } from "./page_header";

interface ConfigFormState {
    saveInProgress: boolean;
    wasValidated: boolean;
    showSpinner: boolean;
}

interface ConfigFormProps {
    children: ComponentChildren;
    id: string;
    title: ComponentChildren;
    isModified: boolean;
    isDirty: boolean;
    onSave: () => Promise<void>;
    onReset: () => Promise<void>;
    onDirtyChange: (dirty: boolean) => void;
    small?: boolean;
}

export class ConfigForm extends Component<ConfigFormProps, ConfigFormState> {
    constructor() {
        super();
        this.state = {saveInProgress: false, wasValidated: false, showSpinner: false}
    }

    submit = (e: Event) => {
        e.preventDefault();
        e.stopPropagation();

        this.setState({saveInProgress: true, wasValidated: false});

        if (!(e.target as HTMLFormElement).checkValidity()) {
            this.setState({saveInProgress: false, wasValidated: true});
            return;
        }

        let spinnerTimeout = window.setTimeout(() => this.setState({showSpinner: true}), 1000);

        this.props.onSave().then(() => {
            window.clearTimeout(spinnerTimeout);
            this.setState({saveInProgress: false, wasValidated: false, showSpinner: false});
            this.props.onDirtyChange(false);
        }).catch(() => {
            window.clearTimeout(spinnerTimeout);
            this.setState({saveInProgress: false, wasValidated: false, showSpinner: false});
        });
    }

    resetButton = () => this.props.onReset ?
        <div class="col">
            <button key="reset" onClick={async () => {await this.props.onReset()}} class="btn btn-danger w-100 text-nowrap" disabled={!this.props.isModified}>
                {__("component.config_form.reset")}
                <span class="ms-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!this.state.showSpinner}></span>
            </button>
        </div> : undefined;

    override render(props: ConfigFormProps, state: Readonly<ConfigFormState>) {
        return (
            <>
                <PageHeader title={props.title} titleColClass="col-12 col-sm" childrenColClass="col-12 col-sm" small={props.small}>
                    <div class="row g-2">
                        {this.resetButton()}
                        <div class="col">
                            <button key="save" type="submit" form={props.id} class="btn btn-primary w-100 text-nowrap" disabled={state.saveInProgress || !props.isDirty}>
                                {__("component.config_form.save")}
                                <span class="ms-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!state.showSpinner}></span>
                            </button>
                        </div>
                    </div>
                </PageHeader>

                <form id={props.id}
                      class={"needs-validation" + (state.wasValidated ? " was-validated" : "")}
                      noValidate
                      onInput={() => this.props.onDirtyChange(true)}
                      onSubmit={this.submit}>
                    {props.children}
                </form>
            </>
        );
    }
}
