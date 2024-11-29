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

import YaMD5 from "../../ts/yamd5";
import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Fragment } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputPassword } from "../../ts/components/input_password";
import { Button } from "react-bootstrap";
import { Switch } from "../../ts/components/switch";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Key } from "react-feather";

export function AuthenticationNavbar() {
    return <NavbarItem name="authentication" module="authentication" title={__("authentication.navbar.authentication")} symbol={<Key />} />;
}

type AuthenticationState = API.getType['authentication/config'];

export class Authentication extends ConfigComponent<'authentication/config', {}, {password: string}> {
    constructor() {
        super('authentication/config',
              () => __("authentication.script.save_failed"),
              () => __("authentication.script.reboot_content_changed"));
    }

    hash(username: string, password?: string) {
        if (password === "")
            return "";

        return YaMD5.YaMD5.hashStr(username + ":esp32-lib:" + password);
    }

    override render(props: {}, state: AuthenticationState & {password: string}) {
        if (!util.render_allowed())
            return <SubPage name="authentication" />;

        let user_required = state.enable_auth || state.digest_hash != "";
        let pass_required = (state.enable_auth && !API.get("authentication/config").enable_auth) || (state.username != API.get("authentication/config").username);

        return (
            <SubPage name="authentication">
                <ConfigForm id="auth_config_form" title={__("authentication.content.authentication")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("authentication.content.enable_authentication")}>
                        <Switch desc={__("authentication.content.enable_authentication_desc")}
                                checked={state.enable_auth}
                                onClick={this.toggle("enable_auth")}
                        />
                    </FormRow>

                    <FormRow label={__("authentication.content.username")}>
                        <InputText value={state.username}
                                   onValue={this.set("username")}
                                   maxLength={32}
                                   required={user_required}
                                   placeholder={user_required ? __("component.input_password.required") : __("component.input_password.not_set")} />
                    </FormRow>

                    <FormRow label={__("authentication.content.password")}>
                        <InputPassword maxLength={64}
                                       value={state.password}
                                       onValue={(v) => {this.setState({password: v, digest_hash: this.hash(state.username, v)})}}
                                       required={pass_required}
                                       placeholder={pass_required ? __("component.input_password.required") : (API.get("authentication/config").digest_hash == "" ? __("component.input_password.not_set") : __("component.input_password.unchanged"))}
                                       hideClear />
                    </FormRow>

                    <FormRow label="">
                        <Button variant="danger" className="form-control" disabled={state.digest_hash == ""} onClick={async () => {
                            const modal = util.async_modal_ref.current;
                            if(!await modal.show({
                                    title: () => __("authentication.content.disable_auth_title"),
                                    body: () => __("authentication.content.disable_auth_body"),
                                    no_text: () => __("authentication.content.disable_auth_abort"),
                                    yes_text: () => __("authentication.content.disable_auth_confirm"),
                                    no_variant: "secondary",
                                    yes_variant: "danger"
                                }))
                                return;
                            this.setState({
                                enable_auth: false,
                                username: "",
                                password: "",
                                digest_hash: ""}, this.save);
                        }}>{__("authentication.content.disable_auth")}</Button>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
