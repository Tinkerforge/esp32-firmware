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

import * as util from "../../ts/util";

import { h, Component, VNode, Fragment, JSX } from "preact";
import { __ } from "../translation";

interface ConfigFormState {
    saveDisabled: boolean
    wasValidated: boolean
    showSpinner: boolean
}

interface ConfigFormProps {
    children: VNode | VNode[]
    id: string
    title: string
    isModified: boolean
    onSave: () => Promise<void>
    onReset: () => Promise<void>
    onDirtyChange: (dirty: boolean) => void
}

export class ConfigForm extends Component<ConfigFormProps,ConfigFormState> {
    constructor() {
        super();
        this.state = {saveDisabled: true, wasValidated: false, showSpinner: false}
    }
    submit = (e: Event) => {
        e.preventDefault();
        e.stopPropagation();

        this.setState({saveDisabled: true, wasValidated: false});

        if (!(e.target as HTMLFormElement).checkValidity()) {
            this.setState({saveDisabled: false, wasValidated: true});
            return;
        }

        let spinnerTimeout = window.setTimeout(() => this.setState({showSpinner: true}), 1000);

        this.props.onSave().then(() => {
            window.clearTimeout(spinnerTimeout);
            this.setState({saveDisabled: true, wasValidated: false, showSpinner: false});
            this.props.onDirtyChange(false);
        }).catch(() => {
            window.clearTimeout(spinnerTimeout);
            this.setState({saveDisabled: false, wasValidated: false, showSpinner: false});
        });
    }

    resetButton = () => this.props.onReset ?
            <button onClick={async () => {await this.props.onReset()}} class="btn btn-danger mb-2 ml-sm-2 col" disabled={!this.props.isModified}>
                {__("component.config_form.reset")}
                <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!this.state.showSpinner} ></span>
            </button> : <></>

    override render (props: ConfigFormProps, state: Readonly<ConfigFormState>) {
        return (
            <>
                <div class="row sticky-under-top mb-3 pt-3">
                    <div class="col pb-2 border-bottom tab-header-shadow">
                        <div class="row no-gutters">
                            <h1 class="page-header col-12 col-sm text-center text-sm-left text-nowrap" dangerouslySetInnerHTML={{__html: props.title}}></h1>
                            <div class="col-12 col-sm row no-gutters">
                                {this.resetButton()}
                                <button type="submit" form={props.id} class="btn btn-primary col mb-2 ml-2 ml-md-3 mr-0" disabled={state.saveDisabled}>
                                    {__("component.config_form.save")}
                                    <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!state.showSpinner}></span>
                                </button>
                            </div>
                        </div>
                    </div>
                </div>
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
