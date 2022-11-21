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
import { is_modified } from "../api";

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

        if (!(document.getElementById(this.props.id) as HTMLFormElement).checkValidity()) {
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

    showResetButton = () => {
        if (typeof this.props.onReset == 'function')
        {;
            return  <button onClick={async () => {await this.props.onReset()}} class="btn btn-danger mb-2 ml-2 col" disabled={!this.props.isModified}>
                        {__("component.config_page_header.reset")}
                        <span class="ml-2 spinner-border spinner-border-sm" role="status" style="vertical-align: middle;" hidden={!this.state.showSpinner} ></span>
                    </button>
        }
        return <></>
    }

    override render (props: ConfigFormProps, state: Readonly<ConfigFormState>) {
        return (
            <>
                <div class="row sticky-under-top mb-3 pt-3">
                    <div class="col-xl-8 pb-2 border-bottom tab-header-shadow">
                        <div class="row no-gutters">
                            <h1 class="config-header col-12 col-sm text-center text-sm-left text-nowrap" dangerouslySetInnerHTML={{__html: props.title}}></h1>
                            <div class="col-12 col-sm row no-gutters">
                                {this.showResetButton()}
                                <button type="submit" form={props.id} class="btn btn-primary col mb-2 mx-2 ml-md-3 mr-sm-0" disabled={state.saveDisabled}>
                                    {__("component.config_page_header.save")}
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
