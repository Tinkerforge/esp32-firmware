/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import YaMD5 from '../../ts/yamd5';

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment } from "preact";
import { __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { Switch } from "src/ts/components/switch";
import { InputPassword } from "src/ts/components/input_password";
import { Slash } from "react-feather";
import { config } from "./api";

type AuthenticationState = API.getType['authentication/config'] & {password: string};

export class Authentication extends ConfigComponent<'authentication/config', {}, AuthenticationState> {
    constructor() {
        super('authentication/config',
              __("authentication.script.save_failed"),
              __("authentication.script.reboot_content_changed"));
    }

    http_auth_allowed() {
        return (this.state.digest_hash == null && (this.state.password !== "")) ||
               (this.state.digest_hash == "" && this.state.password !== undefined && this.state.password !== null && this.state.password !== "");
    };

    override transformSave(cfg: config & {password: string}): config {
        cfg.digest_hash = (this.state.password != null && this.state.password != "") ? YaMD5.YaMD5.hashStr(cfg.username + ":esp32-lib:" + this.state.password) : this.state.password;
        return cfg;
    }

    override render(props: {}, state: AuthenticationState) {
        if (!state)
            return (<></>);

        let auth_allowed = this.http_auth_allowed();

        return (
            <>
                <ConfigForm id="auth_config_form" title={__("authentication.content.authentication")} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("authentication.content.enable_authentication")}>
                        <Switch desc={__("authentication.content.enable_authentication_desc")}
                                checked={auth_allowed && state.enable_auth}
                                onClick={this.toggle("enable_auth")}
                                disabled={!auth_allowed}
                                className={!auth_allowed && state.enable_auth ? "is-invalid" : ""}
                        />
                        <div class="invalid-feedback">{__("authentication.content.enable_authentication_invalid")}</div>
                    </FormRow>

                    <FormRow label={__("authentication.content.username")}>
                        <InputText value={state.username}
                                    onValue={this.set("username")}
                                    minLength={1} maxLength={32}
                                    required/>
                    </FormRow>
                    <FormRow label={__("authentication.content.password")}>
                        <InputPassword
                            maxLength={64}
                            value={state.password === undefined ? state.digest_hash : state.password}
                            onValue={this.set("password")}
                            clearPlaceholder={__("authentication.script.login_disabled")}
                            clearSymbol={<Slash/>}
                            allowAPIClear/>
                    </FormRow>
                </ConfigForm>
            </>
        )
    }
}

render(<Authentication/>, $('#authentication')[0])

export function init() {}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-authentication').prop('hidden', !module_init.authentication);
}
