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
import { __, set_languages_getter, get_active_language, select_language } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { Button } from "react-bootstrap";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Settings } from "react-feather";
import { Switch } from "../../ts/components/switch";
import { Language } from "../../ts/language";
import { ColorScheme } from "./color_scheme.enum";

export function applyTheme(color_scheme: number) {
    let effectiveTheme: string;
    switch (color_scheme) {
        case ColorScheme.Light:
            effectiveTheme = "light";
            break;
        case ColorScheme.Dark:
            effectiveTheme = "dark";
            break;
        case ColorScheme.Browser:
        default:
            effectiveTheme = (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) ? "dark" : "light";
            break;
    }

    const previousTheme = document.documentElement.getAttribute("data-bs-theme");
    document.documentElement.setAttribute("data-bs-theme", effectiveTheme);
    if (previousTheme !== effectiveTheme) {
        window.dispatchEvent(new Event('themechange'));
    }
}

export function SystemNavbar() {
    return <NavbarItem name="system" module="system" title={__("system.navbar.system")} symbol={<Settings />} />;
}

interface SystemState {
    version: API.getType["info/version"],
    theme_config: API.getType["system/theme_config"],
}

type SystemI18nConfig = API.getType["system/i18n_config"];

function get_languages(i18n_config: SystemI18nConfig) {
    let languages = [];

    if (i18n_config && !i18n_config.detect_browser_language) {
        switch (i18n_config.language) {
            case Language.German:
                languages.push("de");
                break;

            case Language.English:
                languages.push("en");
                break;
        }
    }

    return languages.concat(navigator.languages);
}

export class System extends ConfigComponent<"system/i18n_config", {}, SystemState> {
    constructor() {
        super('system/i18n_config',
              () => __("system.script.save_failed"));

        this.state = {
            version: null,
            theme_config: {color_scheme: ColorScheme.Browser},
        } as any;

        util.addApiEventListener('info/version', () => {
            this.setState({version: API.get('info/version')});
        });

        util.addApiEventListener('system/i18n_config', () => {
            window.dispatchEvent(new Event('languagechange'));
        });

        util.addApiEventListener('system/theme_config', () => {
            this.setState({theme_config: API.get("system/theme_config")});
        });

        set_languages_getter(() => get_languages(API.get("system/i18n_config")));
    }

    override async sendSave(topic: "system/i18n_config", config: API.getType["system/i18n_config"]) {
        // Save theme_config
        await API.save("system/theme_config", this.state.theme_config, this.error_string);
        // Save i18n_config
        await super.sendSave(topic, config);
    }

    override async sendReset(topic: "system/i18n_config") {
        // Reset theme_config
        await API.reset("system/theme_config", this.error_string, this.reboot_string);
        // Reset i18n_config
        await super.sendReset(topic);
    }

    render(props: {}, state: SystemI18nConfig & SystemState) {
        if (!util.render_allowed())
            return <SubPage name="system" />;

        // TODO: why not use the charge tracker module here?
        let show_config_reset = false;
        if (API.hasModule("users")) {
            show_config_reset = true;
        }

        let i18n_display_names = new Intl.DisplayNames([get_active_language()], {type: "language"});
        let browser_locale = Intl.NumberFormat().resolvedOptions().locale;
        let browser_language = browser_locale.substring(0, 2);
        let browser_language_name = i18n_display_names.of(browser_locale);
        let web_interface_language = select_language(get_languages(state));
        let web_interface_language_name = i18n_display_names.of(web_interface_language);

        return (
            <SubPage name="system">
                <ConfigForm id="system_config_form"
                            title={__("system.content.system")}
                            isModified={false}
                            isDirty={this.isDirty()}
                            onSave={this.save}
                            onReset={this.reset}
                            onDirtyChange={this.setDirty}>
                    <FormRow label={__("system.content.system_language")} help={__("system.content.system_language_help")} show_warning={browser_language != web_interface_language} warning={__("system.content.system_language_warning")(browser_locale, browser_language_name, web_interface_language, web_interface_language_name)}>
                        <div class="row">
                            <div class="col-md-4">
                                <InputSelect
                                    items={[
                                        [Language.German.toString(), __("system.content.german")],
                                        [Language.English.toString(), __("system.content.english")]
                                    ]}
                                    value={state.language.toString()}
                                    onValue={(v) => this.setState({language: parseInt(v)})}
                                />
                            </div>
                            <div class="col-md-8">
                                <Switch desc={__("system.content.detect_browser_language")}
                                    checked={state.detect_browser_language}
                                    onClick={this.toggle('detect_browser_language')}
                                />
                            </div>
                        </div>
                    </FormRow>

                    <FormRow label={__("system.content.color_scheme")}>
                        <InputSelect
                            items={[
                                [ColorScheme.Browser.toString(), __("system.content.color_scheme_browser")],
                                [ColorScheme.Light.toString(), __("system.content.color_scheme_light")],
                                [ColorScheme.Dark.toString(), __("system.content.color_scheme_dark")]
                            ]}
                            value={state.theme_config.color_scheme.toString()}
                            onValue={(v) => {
                                let color_scheme = parseInt(v);
                                this.setState({theme_config: {color_scheme: color_scheme}});
                            }}
                        />
                    </FormRow>

                    <FormRow label={__("system.content.reboot")} label_muted={__("system.content.reboot_desc")}>
                        <Button variant="primary" className="form-control" onClick={util.reboot}>{__("system.content.reboot")}</Button>
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
                                        await util.put("/config_reset", {"do_i_know_what_i_am_doing": true}, true);
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
                                await util.put("/factory_reset", {"do_i_know_what_i_am_doing": true}, true);
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

export function pre_init() {
    // Listen for system theme changes when using browser preference
    if (window.matchMedia) {
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', () => {
            let theme_config = API.get_unchecked("system/theme_config");
            if (theme_config && theme_config.color_scheme === ColorScheme.Browser) {
                applyTheme(ColorScheme.Browser);
            }
        });
    }
}

export function init() {
    util.addApiEventListener('system/last_reset', (v) => {
        if (v.data.show_warning) {
            const version = API.get('info/version');
            util.add_status_alert("system", "warning", () => __("system.status.system"), () => __("system.status.last_reset")(v.data.reason, version.firmware), () => {
                API.call("system/hide_last_reset_warning", {}, () => __("system.status.hide_last_reset_warning_failed"));
            });
        } else {
            util.remove_status_alert("system");
        }
    });

    util.addApiEventListener('system/theme_config', () => {
        let theme_config = API.get("system/theme_config");
        if (theme_config) {
            applyTheme(theme_config.color_scheme);
        }
    });
}
