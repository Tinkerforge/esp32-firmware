/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
import { h } from "preact";
import { __, update_languages_function } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText } from "../../ts/components/input_text";
import { InputSelect } from "../../ts/components/input_select";
import { Button } from "react-bootstrap";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Settings } from "react-feather";
import { Switch } from "../../ts/components/switch";
import { Language } from "./language.enum";

export function SystemNavbar() {
    return <NavbarItem name="system" module="system" title={__("system.navbar.system")} symbol={<Settings />} />;
}

interface SystemState {
    version: API.getType["info/version"],
}

type SystemI18nConfig = API.getType["system/i18n_config"];

export class System extends ConfigComponent<"system/i18n_config", {}, SystemState> {
    constructor() {
        super('system/i18n_config',
              () => __("system.script.save_failed"));

        this.state = {
            version: null,
        } as any;

        util.addApiEventListener('info/version', () => {
            this.setState({version: API.get('info/version')});
        });

        util.addApiEventListener('system/i18n_config', () => {
            window.dispatchEvent(new Event('languagechange'));
        });

        update_languages_function(() =>  {
            let i18n_config = API.get("system/i18n_config");

            if (!i18n_config || i18n_config.detect_browser_language) {
                return navigator.languages;
            }

            switch (i18n_config.language) {
                case Language.German:  return ["de"].concat(navigator.languages);
                case Language.English: return ["en"].concat(navigator.languages);
            }
        });
    }

    render(props: {}, state: SystemI18nConfig) {
        if (!util.render_allowed())
            return <SubPage name="system" />;

        // TODO: why not use the charge tracker module here?
        let show_config_reset = false;
        if (API.hasModule("users")) {
            show_config_reset = true;
        }

        return (
            <SubPage name="system">
                <ConfigForm id="system_config_form"
                            title={__("system.content.system")}
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("system.content.system_language")} help={__("system.content.system_language_help")}>
                        <div class="row">
                            <div class="col-md-4">
                                <div class="input-group">
                                    <InputSelect
                                        items={[
                                            [Language.German.toString(), __("system.content.german")],
                                            [Language.English.toString(), __("system.content.english")]
                                        ]}
                                        value={state.language.toString()}
                                        onValue={(v) => this.setState({language: parseInt(v)})}
                                    />
                                </div>
                            </div>
                            <div class="col-md-8">
                                <div class="input-group">
                                    <Switch desc={__("system.content.detect_browser_language")}
                                        checked={state.detect_browser_language}
                                        onClick={this.toggle('detect_browser_language')}
                                    />
                                </div>
                            </div>
                        </div>
                    </FormRow>

                    <FormRow label={__("system.content.reboot")} label_muted={__("system.content.reboot_desc")}>
                        <Button variant="primary" className="form-control" onClick={util.reboot}>{__("system.content.reboot")}</Button>
                    </FormRow>

                    <FormRow label={__("system.content.current_spiffs")}>
                        <InputText value={this.state.version.config + " (" + this.state.version.config_type + ")"}/>
                    </FormRow>

                    {show_config_reset ?
                        <FormRow label={__("system.content.config_reset")} label_muted={__("system.content.config_reset_desc")}>
                            <Button variant="danger" className="form-control" onClick={async () => {
                                    const modal = util.async_modal_ref.current;
                                    if (!await modal.show({
                                            title: () => __("system.content.config_reset"),
                                            body: () => __("system.content.config_reset_modal_text"),
                                            no_text: () => __("system.content.abort_reset"),
                                            yes_text: () => __("system.content.confirm_config_reset"),
                                            no_variant: "secondary",
                                            yes_variant: "danger"
                                        }))
                                        return;

                                    try {
                                        await util.put("/config_reset", {"do_i_know_what_i_am_doing": true});
                                        util.postReboot(__("system.script.config_reset_init"), __("util.reboot_text"));
                                    } catch (error) {
                                        util.add_alert("config_reset_failed", "danger", () => __("system.script.config_reset_error"), error);
                                    }
                                }}>{__("system.content.config_reset")}</Button>
                        </FormRow>
                        : ""
                    }

                    <FormRow label={__("system.content.factory_reset")} label_muted={__("system.content.factory_reset_desc")}>
                        <Button variant="danger" className="form-control" onClick={async () => {
                            const modal = util.async_modal_ref.current;
                            if (!await modal.show({
                                    title: () => __("system.content.factory_reset"),
                                    body: () => __("system.content.factory_reset_modal_text"),
                                    no_text: () => __("system.content.abort_reset"),
                                    yes_text: () => __("system.content.confirm_factory_reset"),
                                    no_variant: "secondary",
                                    yes_variant: "danger"
                                }))
                                return;

                            try {
                                await util.put("/factory_reset", {"do_i_know_what_i_am_doing": true});
                                util.postReboot(__("system.script.factory_reset_init"), __("util.reboot_text"));
                            } catch (error) {
                                util.add_alert("factory_reset_failed", "danger", () => __("system.script.factory_reset_error"), error);
                            }
                        }}>{__("system.content.factory_reset")}</Button>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
