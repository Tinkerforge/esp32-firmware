/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

//#include "generated/module_available.inc"

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import YaMD5 from "../../ts/yamd5";
import { h, Fragment } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";
import {
    ConfigComponent,
    ConfigComponentState,
} from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputText, InputTextPatterned } from "../../ts/components/input_text";
import { InputFloat } from "../../ts/components/input_float";
import { Switch } from "../../ts/components/switch";
import { InputPassword } from "../../ts/components/input_password";
//#if MODULE_EVSE_COMMON_AVAILABLE
import { EVSE_SLOT_USER } from "../evse_common/api";
//#endif
import { SubPage } from "../../ts/components/sub_page";
import { Table, TableRow } from "../../ts/components/table";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Slash, Check, Plus, X, Users as UsersSymbol } from "react-feather";
//#if MODULE_NFC_AVAILABLE
import {
    DiscoveryResultItem,
    DiscoveryResultGroup,
} from "../../ts/components/discovery_result";
import { InputSelect } from "../../ts/components/input_select";
import { useEffect, useRef, useState } from "preact/hooks";
import { useSignal } from "@preact/signals";
//#endif
import { CMAuthType } from "../cm_networking/generated/cm_auth_type.enum";
import { ChargeAuth } from "../charge_authorization/api";

//#if MODULE_CHARGE_MANAGER_AVAILABLE
export async function get_charge_manager_auth_info(auth_type_filter?: CMAuthType[]): Promise<(ChargeAuth & {charger_name: string})[]> {
    let cm_state = API.get("charge_manager/state");
    let auths:(ChargeAuth & {charger_name?: string})[][] = [];
    try {
        auths = JSON.parse(await util.download("/charge_manager/auth_info", false).then(x => x.text()));
    } catch (e) {
        console.error("Failed to fetch charge_manager/auth_info:", e);
        return []
    }

    let chargers = Math.min(auths.length, cm_state.chargers.length);

    let result = [] as (ChargeAuth & {charger_name: string})[];
    let result_json_cache: string[] = [];

    for (let charger_idx = 0; charger_idx < chargers; charger_idx++) {
        const charger_name = cm_state.chargers[charger_idx].n;

        for (let auth of auths[charger_idx]) {
            if (auth.seen_at == 0)
                continue;

            if (auth_type_filter && auth_type_filter.indexOf(auth.auth_info[0]) == -1)
                continue;

            auth.charger_name = charger_name;

            const info_json = JSON.stringify(auth.auth_info);

            let idx = result_json_cache.indexOf(info_json);
            if (idx == -1) {
                result.push(auth as (ChargeAuth & {charger_name: string}));
                result_json_cache.push(info_json);
            } else if (auth.seen_at < result[idx].seen_at) {
                result[idx] = auth as (ChargeAuth & {charger_name: string});
            }
        }
    }

    return result;
}
//#endif

// Get NFC tags from managed chargers, merged with local seen tags.
// Returns all seen tags (local + remote), deduplicated.
type NFCSeenTag = API.getType['nfc/seen_tags'][0] & { charger_name?: string | null };

async function get_all_seen_tags(): Promise<NFCSeenTag[]> {
    let central_management_enabled = API.hasModule("charge_manager") &&
        API.get("charge_manager/config").enable_charge_manager &&
        API.get("charge_manager/config").enable_central_management;

    let now = API.get("info/keep_alive").uptime;

    if (!central_management_enabled) {
        return API.get('nfc/seen_tags')?.map(t => {
            const tag: NFCSeenTag = {...t, charger_name: null};
            return tag;
        });
    } else {
        return (await get_charge_manager_auth_info([CMAuthType.NFC, CMAuthType.InjectedNFC])).map(x => ({
                tag_id: (x.auth_info[1] as any).tag_id,
                tag_type: (x.auth_info[1] as any).tag_type,
                last_seen: now - x.seen_at,
                charger_name: x.charger_name,
            } as NFCSeenTag));
    }
}

export function UsersNavbar() {
    return (
        <NavbarItem
            name="users"
            module="users"
            title={__("users.navbar.users")}
            symbol={<UsersSymbol />}
        />
    );
}

type User = API.getType["users/config"]["users"][0] & {
    password: string;
    is_invalid: number;
};
type UsersConfig = Omit<API.getType["users/config"], "users"> & {
    users: User[];
};

interface UsersState {
    userSlotEnabled: boolean;
    nfcDeadtime: number;

    editUser: User;
    editUserNfcTags: API.getType["nfc/config"]["authorized_tags"];
//#if MODULE_EV_AVAILABLE
    editUserEvs: API.getType["ev/config"]["evs"];
//#else
    editUserEvs: {}[];
//#endif
}

// This is a bit hacky: the user modification API can take some time because it writes the changed user/display name to flash
// The API will block up to five seconds, but just to be sure we try this twice.
function retry_once<T>(fn: () => Promise<T>, topic: string) {
    return fn().catch(() => {
        util.remove_alert(topic);
        return fn();
    });
}

function remove_user(id: number) {
    return retry_once(
        () =>
            API.call("users/remove", { id: id }, () =>
                __("users.script.save_failed"),
            ),
        "users_remove_failed",
    );
}

function modify_user(u: User) {
    // Don't hash if u.password is falsy, i.e. null, undefined or the empty string
    u.digest_hash = u.password
        ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password)
        : u.password;

    // Always send digest_hash, but as null if we don't want to change it.
    // digest_hash can be undefined if this user was not modified.
    if (u.digest_hash === undefined)
        u.digest_hash = null;

    let { password, is_invalid, ...user } = u;
    return retry_once(
        () => API.call("users/modify", user, () => __("users.script.save_failed")),
        "users_modify_failed",
    );
}

function modify_unknown_user(name: string) {
    return retry_once(
        () =>
            API.call(
                "users/modify",
                {
                    id: 0,
                    display_name: name,
                    username: null,
                    current: null,
                    digest_hash: null,
                    roles: null,
                },
                () => __("users.script.save_failed"),
            ),
        "users_modify_failed",
    );
}

function add_user(u: User) {
    // Don't hash if u.password is falsy, i.e. null, or the empty string.
    // u.password can't be undefined (as is handled above when modifying users),
    // because adding a user sets password to "" if nothing was entered.
    u.digest_hash = u.password
        ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password)
        : "";

    let { password, is_invalid, ...user } = u;
    return retry_once(
        () => API.call("users/add", user, () => __("users.script.save_failed")),
        "users_add_failed",
    );
}

//#if MODULE_NFC_AVAILABLE
interface NfcTagsSectionProps {
    users: User[];
    nfcConfig: API.getType["nfc/config"]["authorized_tags"];
    onNfcConfig: (tags: API.getType["nfc/config"]["authorized_tags"]) => void;
    currentUserId: number;
    currentUsername: string;
}

function NfcTagsSection({
    users,
    nfcConfig,
    onNfcConfig,
    currentUserId,
    currentUsername
}: NfcTagsSectionProps) {
    const seenTags = useSignal<NFCSeenTag[]>([]);
    useEffect(() => {
        const interval = setInterval(() => {
            get_all_seen_tags().then((tags) => (seenTags.value = tags));
        }, 1000);
        return () => clearInterval(interval);
    });

    const getRows = (): TableRow[] => {
        return nfcConfig.filter(tag => tag.user_id == currentUserId).map((tag, j) => {
            const seen = seenTags.value.find(
                (s) => s.tag_id === tag.tag_id && s.tag_type === tag.tag_type,
            );
            const lastSeenText = seen
                ? __("nfc.content.last_seen") +
                  util.format_timespan_ms(seen.last_seen) +
                  __("nfc.content.last_seen_suffix")
                : "";
            return {
                columnValues: [
                    tag.tag_id,
                    translate_unchecked(`nfc.content.type_${tag.tag_type}`),
                    lastSeenText,
                ],
                fieldNames: [
                    __("users.content.nfc_tag_id"),
                    __("users.content.nfc_tag_type"),
                    __("users.content.nfc_last_seen"),
                ],
                fieldValues: [
                    tag.tag_id,
                    translate_unchecked(`nfc.content.type_${tag.tag_type}`),
                    lastSeenText,
                ],
                hideRemoveButton: false,
                onRemoveClick: async () => {
                    onNfcConfig(
                        nfcConfig
                            // Remove unassigned tags
                            .filter(t => t.user_id != 0)
                            // Remove editTag in case it was already configured but for another (or no) user
                            .filter(confd_tag => confd_tag.tag_id != tag.tag_id || confd_tag.tag_type != tag.tag_type));
                    return true;
                },
            };
        });
    };

    const getTagError = (t: {
        tag_type: number;
        tag_id: string;
    }): string | undefined => {
        for (const i in nfcConfig) {
            const confd_tag = nfcConfig[i];
            if (confd_tag.tag_type != t.tag_type || confd_tag.tag_id != t.tag_id)
                continue;

            if (confd_tag.user_id == 0)
                return undefined;

            if (confd_tag.user_id == currentUserId)
                return __("users.content.nfc_tag_already_assigned")(currentUsername);

            let name = users.find((u) => u.id == confd_tag.user_id)?.display_name;
            if (name)
                return __("users.content.nfc_tag_already_assigned")(currentUsername);

            return __("component.discovery_result.already_added")
        }

        return undefined;
    };

    const MAX_TAGS = API.hasModule("esp32_ethernet_brick") ? 32 : 16;


    const [editTag, setEditTag] = useState<typeof nfcConfig[0]>({user_id: currentUserId, tag_type: 0, tag_id: ""});

    return (
        <FormRow label={__("users.content.nfc_tags")}>
            <Table
                columnNames={[
                    __("users.content.nfc_tag_id"),
                    __("users.content.nfc_tag_type"),
                    __("users.content.nfc_last_seen"),
                ]}
                nestingDepth={2}
                rows={getRows()}
                tableTill="md"
                addEnabled={nfcConfig.length < MAX_TAGS}
                addTitle={__("users.content.nfc_add_tag")}
                onAddShow={async () => setEditTag({user_id: currentUserId, tag_type: 0, tag_id: ""})}
                onAddGetChildren={() => {
                    let filtered = seenTags.value
                                    .filter((t) => t.tag_id !== "")
                                    .map((t) => {
                                        const error = getTagError(t);
                                        return (
                                            <DiscoveryResultItem
                                                key={`${t.tag_type}-${t.tag_id}`}
                                                title={
                                                    <div class="h5">
                                                        {t.tag_id}
                                                    </div>
                                                }
                                                labelAdd={<Plus />}
                                                error={error}
                                                onClick={() => setEditTag({...editTag, tag_id: t.tag_id, tag_type: t.tag_type})}
                                            >
                                                <div>
                                                    {translate_unchecked(`nfc.content.type_${t.tag_type}`)}
                                                </div>
                                                <div class="text-muted small">
                                                    {__("nfc.content.last_seen") + util.format_timespan_ms(t.last_seen) + __("nfc.content.last_seen_suffix")}
                                                </div>
                                            </DiscoveryResultItem>
                                        );
                                    });

                    let tags = filtered.length > 0 ?
                        filtered :
                        <DiscoveryResultItem
                            key="-1"
                            title=""
                            error={__("users.content.nfc_no_seen_tags")(
                                !API.hasFeature("nfc") || (
                                API.hasModule("charge_manager") &&
                                API.get("charge_manager/config").enable_central_management))}
                            labelAdd="">
                        </DiscoveryResultItem>;

                   return  <>
                        <FormRow label={__("users.content.nfc_seen_tags")}>
                            <DiscoveryResultGroup>
                                {tags}
                            </DiscoveryResultGroup>
                        </FormRow>
                        <FormRow label={__("users.content.nfc_tag_id")}>
                            <InputTextPatterned
                                value={editTag.tag_id}
                                onValue={(v) => setEditTag({...editTag, tag_id: v})}
                                minLength={8}
                                maxLength={29}
                                pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                                placeholder={__("users.content.nfc_tag_id_placeholder")}
                                invalidFeedback={__("users.content.nfc_tag_id_invalid")}
                            />
                        </FormRow>
                        <FormRow label={__("users.content.nfc_tag_type")}>
                            <InputSelect
                                items={[
                                    ["0", __("nfc.content.type_0")],
                                    ["1", __("nfc.content.type_1")],
                                    ["2", __("nfc.content.type_2")],
                                    ["3", __("nfc.content.type_3")],
                                    ["4", __("nfc.content.type_4")],
                                ]}
                                value={editTag.tag_type.toString()}
                                onValue={(v) => setEditTag({...editTag, tag_type: parseInt(v)})}
                            />
                        </FormRow>
                    </>
                }}
                onAddSubmit={async () => onNfcConfig(
                    nfcConfig
                        // Remove unassigned tags
                        .filter(t => t.user_id != 0)
                        // Remove editTag in case it was already configured but for another (or no) user
                        .filter(confd_tag => confd_tag.tag_id != editTag.tag_id || confd_tag.tag_type != editTag.tag_type)
                        .concat(editTag))
                }
            />
        </FormRow>
    );
}
//#endif

//#if MODULE_EV_AVAILABLE
interface EvsSectionProps {
    users: User[];
    evConfig: API.getType["ev/config"]["evs"];
    onEvConfig: (cfg: API.getType["ev/config"]["evs"]) => void;
    currentUserId: number;
    currentUsername: string;
}

function EvsSection({
    users,
    evConfig,
    onEvConfig,
    currentUserId,
    currentUsername
}: EvsSectionProps) {
    // Look up the configured name of an EV by its MAC. Falls back to the MAC itself.
    const ev_name = (mac: string): string => {
        const ev = evConfig.find((e) => e.mac === mac);
        return ev ? ev.name : mac;
    };

    const getRows = (): TableRow[] => {
        return evConfig.map((ev, i) => {return {ev:ev, i:i};}).filter(t => t.ev.user_id == currentUserId).map(t => {
            return {
                columnValues: [ev_name(t.ev.mac), t.ev.mac],
                fieldNames: [
                    __("users.content.ev_name"),
                    __("users.content.ev_mac"),
                ],
                fieldValues: [ev_name(t.ev.mac), t.ev.mac],
                hideRemoveButton: false,
                onRemoveClick: async () => {
                    onEvConfig(evConfig.filter((cfg, j) => t.i != j));
                    return true;
                },
            };
        });
    };

    return (
        <FormRow label={__("users.content.ev_macs")}>
            <Table
                columnNames={[
                    __("users.content.ev_name"),
                    __("users.content.ev_mac"),
                ]}
                nestingDepth={2}
                rows={getRows()}
                tableTill="md"
                addEnabled={true}
                addTitle={__("users.content.ev_add_mac")}
                hideSubmitButton={true}
                onAddShow={async () => {}}
                onAddGetChildren={() => {
                    let items =
                        evConfig.length > 0 ? (
                            evConfig.map((ev, i) => {
                                const ownerName = ev.user_id == 0 ? undefined : (ev.user_id == currentUserId ? currentUsername : users.find(u => u.id == ev.user_id).display_name);
                                const error = ownerName ? (__("users.content.ev_mac_already_assigned")(ownerName) as string) : undefined;
                                return (
                                    <DiscoveryResultItem
                                        key={ev.mac}
                                        type="submit"
                                        title={<div class="h5">{ev.name}</div>}
                                        labelAdd={<Plus />}
                                        error={error}
                                        onClick={() => {
                                            evConfig[i].user_id = currentUserId;
                                            onEvConfig(evConfig);
                                        }}
                                    >
                                        <div class="text-muted small font-monospace">
                                            {ev.mac}
                                        </div>
                                    </DiscoveryResultItem>
                                );
                            })
                        ) : (
                            <DiscoveryResultItem
                                key="-1"
                                title={__("users.content.ev_no_evs")}
                                labelAdd=""
                            ></DiscoveryResultItem>
                        );

                    return (
                        <FormRow label={__("users.content.ev_available_evs")}>
                            <DiscoveryResultGroup>{items}</DiscoveryResultGroup>
                        </FormRow>
                    );
                }}
            />
        </FormRow>
    );
}
//#endif

interface EditUserFormContentProps {
    user: User;
    errorMessage: string | undefined;
    password: 'required' | 'optional' | 'add'; // pass 'add' when adding an user. pass 'required' or 'optional' when editing an existing user.
    //#if MODULE_NFC_AVAILABLE
    users: User[];
    nfcConfig: API.getType["nfc/config"]["authorized_tags"];
    onNfcConfig: (tags: API.getType["nfc/config"]["authorized_tags"]) => void;
    //#endif
    //#if MODULE_EV_AVAILABLE
    evConfig: API.getType["ev/config"]["evs"];
    onEvConfig: (evs: API.getType["ev/config"]["evs"]) => void;
    //#endif
    onUserChange: (changes: Partial<User>) => void;
}

function EditUserFormContent({
    user,
    errorMessage,
    password,
    //#if MODULE_NFC_AVAILABLE
    users,
    nfcConfig,
    onNfcConfig,
    //#endif
    //#if MODULE_EV_AVAILABLE
    evConfig,
    onEvConfig,
    //#endif
    onUserChange,
}: EditUserFormContentProps) {
    return (
        <>
            <FormRow
                label={__("users.content.edit_user_username")}
                label_muted={__("users.content.edit_user_username_desc")}
            >
                <InputText
                    value={user.username}
                    onValue={(v) => onUserChange({ username: v })}
                    required
                    minLength={1}
                    maxLength={32}
                    class={
                        user.is_invalid !== undefined && user.is_invalid !== 0
                            ? "is-invalid"
                            : ""
                    }
                    invalidFeedback={errorMessage}
                />
            </FormRow>
            <FormRow
                label={__("users.content.edit_user_display_name")}
                label_muted={__("users.content.edit_user_display_name_desc")}
            >
                <InputText
                    value={user.display_name}
                    onValue={(v) => onUserChange({ display_name: v })}
                    required
                    minLength={1}
                    maxLength={32}
                />
            </FormRow>
            <FormRow label={__("users.content.edit_user_current")}>
                <InputFloat
                    unit="A"
                    value={user.current}
                    onValue={(v) => onUserChange({ current: v })}
                    digits={3}
                    min={6000}
                    max={32000}
                />
            </FormRow>
            <FormRow label={__("users.content.edit_user_password")}>
                <InputPassword
                    maxLength={64}
                    value={user.password === undefined ? user.digest_hash : user.password}
                    onValue={(v) => onUserChange({ password: v })}
                    required={password == 'required'}
                    hideClear={password == 'add'}
                    clearPlaceholder={__("users.script.login_disabled")}
                    clearSymbol={<Slash />}
                    allowAPIClear={password != 'add'}
                />
            </FormRow>
            {/*#if MODULE_NFC_AVAILABLE*/}
            <NfcTagsSection
                users={users}
                nfcConfig={nfcConfig}
                onNfcConfig={onNfcConfig}
                currentUserId={user.id}
                currentUsername={user.username}
            />
            {/*#endif*/}
            {/*#if MODULE_EV_AVAILABLE*/}
            <EvsSection
                users={users}
                evConfig={evConfig}
                onEvConfig={onEvConfig}
                currentUserId={user.id}
                currentUsername={user.username}
            />
            {/*#endif*/}
        </>
    );
}

export class Users extends ConfigComponent<"users/config", {}, UsersState> {
    constructor() {
        super(
            "users/config",
            () => __("users.script.save_failed"),
            () => __("users.script.reboot_content_changed"),
            {
                userSlotEnabled: false,
                editUser: {
                    id: -1,
                    roles: 0xffff,
                    username: "",
                    display_name: "",
                    current: 32000,
                    digest_hash: "",
                    password: "",
                    is_invalid: 0,
                },
                editUserNfcTags: [],
                nfcDeadtime: 0,
                editUserEvs: []
            },
        );

        //#if MODULE_EVSE_COMMON_AVAILABLE
        util.addApiEventListener("evse/slots", () => {
            this.setState({
                userSlotEnabled: API.get("evse/slots")[EVSE_SLOT_USER].active,
            });
        });
        //#endif

        //#if MODULE_NFC_AVAILABLE
        util.addApiEventListener("nfc/config", () => {
            this.setState({
                nfcDeadtime: API.get("nfc/config").deadtime_post_start,
            });
        });
        //#endif
    }

    user_has_password(u: API.getType["users/config"]["users"][0]) {
        return u.digest_hash === null;
    }

    http_auth_allowed() {
        return this.state.users.some((u) => this.user_has_password(u));
    }

    override async sendSave(topic: "users/config", new_config: UsersConfig) {
        let old_config = API.get("users/config");
        new_config.http_auth_enabled &&= this.http_auth_allowed();

        if (new_config.users[0].display_name == __("charge_tracker.script.unknown_user"))
            new_config.users[0].display_name = "Anonymous";

        if (new_config.users[0].display_name === "")
            await modify_unknown_user("Anonymous");
        else if (new_config.users[0].display_name != old_config.users[0].display_name)
            await modify_unknown_user(new_config.users[0].display_name);

        //#if MODULE_EVSE_COMMON_AVAILABLE
        await API.save(
            "evse/user_enabled",
            { enabled: this.state.userSlotEnabled },
            () => __("evse.script.save_failed")
        );
        //#endif

        //#if MODULE_NFC_AVAILABLE
        await API.save("nfc/config",
                       {...API.get("nfc/config"), deadtime_post_start: this.state.nfcDeadtime},
                       () => __("nfc.script.save_failed"));
        //#endif

        // Use call unchecked here: http_auth_update is not in api.ts for some reason
        await API.call_unchecked("users/http_auth_update",
                                {enabled: new_config.http_auth_enabled,},
                                () => __("users.script.save_failed"),
        );
    }

    async checkUsername(user: User, ignore_i: number): Promise<number> {
        let user_cfg = API.get("users/config");
        for (let i = 0; i < user_cfg.users.length; ++i) {
            if (i == ignore_i)
                continue;

            if(user_cfg.users[i].username == user.username)
                return 1;
        }

        if (user_cfg.next_user_id == 0) {
            return 3;
        }

        let all_usernames = await getAllUsernames();

        for (let i = 0; i < all_usernames[0].length; ++i) {
            if (i == user.id)
                continue;

            if (all_usernames[0][i] == user.username) {
                return 2;
            }
        }

        return 0;
    }

    errorMessage(user: User): string {
        switch (user.is_invalid) {
            case 1:
                return __("users.script.username_already_used");
            case 2:
                return __("users.script.username_already_tracked");
            case 3:
                return __("users.script.all_user_ids_in_use");
            default:
                return undefined;
        }
    }

    require_password(user: User): boolean {
        let lst = API.get("users/config").users.filter((u) => u.id == user.id);
        if (lst.length != 1) return false;

        let configured = lst[0];
        if (configured.digest_hash === "") return false;

        return user.username != configured.username;
    }

    override render(
        props: {},
        state: UsersConfig & UsersState & ConfigComponentState,
    ) {
        if (!util.render_allowed()) return <SubPage name="users" />;

        const MAX_ACTIVE_USERS = API.hasModule("esp32_ethernet_brick")
            ? 33
            : 17;

        let auth_allowed = this.http_auth_allowed();

        // Only allow enabling the user slot if there are at least two users (anonymous counts as one)
        let user_slot_allowed = state.users.length > 1;

        let central_auth_enabled = false;
        let manager_ip = "";
//#if MODULE_EVSE_COMMON_AVAILABLE
        let management_state = API.get("evse/management_state");
        central_auth_enabled = management_state.central_user_management_enabled;
        manager_ip = management_state.manager_ip;
//#endif
        const MAX_TAGS = API.hasModule("esp32_ethernet_brick") ? 32 : 16;

        return (
            <SubPage name="users">
                <ConfigForm
                    id="users_config_form"
                    title={__("users.content.users")}
                    isDirty={this.isDirty()}
                    onSave={this.save}
                    onDirtyChange={this.setDirty}
                >
                    <FormRow label={__("users.content.enable_authentication")}>
                        <Switch
                            desc={__("users.content.enable_authentication_desc")}
                            checked={auth_allowed && state.http_auth_enabled}
                            onClick={this.toggle("http_auth_enabled")}
                            disabled={!auth_allowed}
                            className={
                                !auth_allowed && state.http_auth_enabled
                                    ? "is-invalid"
                                    : ""
                            }
                        />
                        <div class="invalid-feedback">
                            {__("users.content.enable_authentication_invalid")}
                        </div>
                    </FormRow>

                    {/*#if MODULE_EVSE_COMMON_AVAILABLE*/}
                        <FormRow
                            label={__("users.content.evse_user_description")}
                            warning={__(
                                "users.content.evse_user_enable_central_management_warning"
                            )(manager_ip)}
                            show_warning={user_slot_allowed && state.userSlotEnabled && central_auth_enabled}
                        >
                            <Switch
                                desc={__("users.content.evse_user_enable")}
                                checked={user_slot_allowed && state.userSlotEnabled}
                                disabled={!user_slot_allowed}
                                className={
                                    !user_slot_allowed && state.userSlotEnabled
                                        ? "is-invalid"
                                        : ""
                                }
                                onClick={this.toggle("userSlotEnabled")}
                            />
                            <div class="invalid-feedback">
                                {__("users.content.evse_user_enable_invalid")}
                            </div>
                        </FormRow>
                    {/*#endif*/}

                    <FormRow label={__("users.content.unknown_username")}>
                        <InputPassword
                            maxLength={32}
                            value={
                                state.users[0].display_name == "Anonymous"
                                    ? __("charge_tracker.script.unknown_user")
                                    : state.users[0].display_name
                            }
                            onValue={(v) => {
                                this.state.users[0].display_name = v;
                                this.setState({users: this.state.users});
                            }}
                            showAlways
                        />
                    </FormRow>

                    {/*#if MODULE_NFC_AVAILABLE*/}
                        <FormRow
                            label={__("nfc.content.deadtime")}
                            label_muted={__("nfc.content.deadtime_muted")}
                        >
                            <InputSelect
                                items={[
                                    ["0", __("nfc.content.deadtime_min")],
                                    ["3", __("nfc.content.deadtime_3")],
                                    ["10", __("nfc.content.deadtime_10")],
                                    ["30", __("nfc.content.deadtime_30")],
                                    ["60", __("nfc.content.deadtime_60")],
                                    ["4294967295", __("nfc.content.deadtime_max")],
                                ]}
                                value={state.nfcDeadtime}
                                onValue={(v) => {
                                    this.setState({ nfcDeadtime: parseInt(v) });
                                }}
                            />
                        </FormRow>
                    {/*#endif*/}

                    <FormRow label={__("users.content.authorized_users")}>
                        <Table
                            columnNames={[
                                __("users.content.table_username"),
                                __("users.content.table_display_name"),
                                __("users.content.table_current"),
                                __("users.content.table_password"),
                            ]}
                            rows={state.users.slice(1).map((user, i) => {
                                return {
                                    columnValues: [
                                        user.username,
                                        user.display_name,
                                        util.toLocaleFixed(
                                            user.current / 1000,
                                            3,
                                        ) + " A",
                                        this.user_has_password(user) ? <Check /> : "",
                                    ],
                                    fieldValues: [
                                        user.username,
                                        user.display_name,
                                        util.toLocaleFixed(
                                            user.current / 1000,
                                            3,
                                        ) + " A",
                                        <span class="text-muted">
                                            {this.user_has_password(user) ?
                                                __("component.input_password.unchanged") :
                                                __("users.script.login_disabled")}
                                        </span>,
                                    ],
                                    editTitle: __("users.content.edit_user_title"),
                                    onEditShow: async () => {
                                        this.setState({
                                            editUser: user,

                                            //#if MODULE_NFC_AVAILABLE
                                            editUserNfcTags: API.get("nfc/config").authorized_tags.slice(),
                                            //#else
                                            editUserNfcTags: [],
                                            //#endif

                                            //#if MODULE_EV_AVAILABLE
                                            editUserEvs: API.get("ev/config").evs.slice(),
                                            //#else
                                            editUserEvs: [],
                                            //#endif
                                        });
                                    },
                                    onEditGetChildren: () => [
                                        <EditUserFormContent
                                            user={state.editUser}
                                            errorMessage={this.errorMessage(
                                                state.editUser,
                                            )}
                                            password={this.require_password(state.editUser) ? 'required' : 'optional'}
                                            //#if MODULE_NFC_AVAILABLE
                                            users={state.users}
                                            nfcConfig={state.editUserNfcTags}
                                            onNfcConfig={(cfg) => this.setState({editUserNfcTags: cfg})}
                                            //#endif
                                            //#if MODULE_EV_AVAILABLE
                                            evConfig={state.editUserEvs}
                                            onEvConfig={(cfg) => {console.log(cfg); this.setState({editUserEvs: cfg});}}
                                            //#endif
                                            onUserChange={(changes) =>
                                                this.setState({
                                                    editUser: {
                                                        ...state.editUser,
                                                        ...changes,
                                                        is_invalid: 0
                                                    },
                                                })
                                            }
                                        />,
                                    ],
                                    onEditCheck: async () => {
                                        console.log("hier");
                                        let is_invalid =
                                            await this.checkUsername(
                                                state.editUser,
                                                i + 1,
                                            );

                                        return new Promise<boolean>(
                                            (resolve) => {
                                                this.setState(
                                                    {
                                                        editUser: {
                                                            ...state.editUser,
                                                            is_invalid:
                                                                is_invalid,
                                                        },
                                                    },
                                                    () =>
                                                        resolve(
                                                            is_invalid ==
                                                                undefined ||
                                                                is_invalid == 0,
                                                        ),
                                                );
                                            },
                                        );
                                    },
                                    onEditSubmit: async () => {
                                        if (API.get("users/config").users.every(user => user.id != state.editUser.id))
                                            return; // This user was removed while we were editing it. Drop it.

                                        await modify_user(state.editUser);
                                        //#if MODULE_NFC_AVAILABLE
                                        let nfc_config = API.get("nfc/config")
                                        await API.save("nfc/config", {
                                            ...nfc_config,
                                            authorized_tags: state.editUserNfcTags
                                        });
                                        //#endif
                                        //#if MODULE_EV_AVAILABLE
                                        let ev_config = API.get("ev/config")
                                        await API.save("ev/config", {
                                            ...ev_config,
                                            evs: state.editUserEvs
                                        });
                                        //#endif
                                    },
                                    onRemoveClick: async () => {
                                        await remove_user(user.id);
                                        return true;
                                    },
                                };
                            })}
                            addEnabled={
                                API.get("users/config").next_user_id != 0 &&
                                state.users.length < MAX_ACTIVE_USERS
                            }
                            addTitle={__("users.content.add_user_title")}
                            // One user slot is always taken by the unknown user, so display MAX_ACTIVE_USERS - 1 as the maximum number of users that can be added.
                            addMessage={
                                API.get("users/config").next_user_id == 0
                                    ? __("users.content.add_user_user_ids_exhausted")
                                    : __("users.content.add_user_message")(
                                          state.users.length - 1,
                                          MAX_ACTIVE_USERS - 1,
                                      )
                            }
                            onAddShow={async () =>
                                this.setState({
                                    editUser: {
                                        id: API.get("users/config").next_user_id,
                                        roles: 0xffff,
                                        username: "",
                                        display_name: "",
                                        current: 32000,
                                        digest_hash: "",
                                        password: "",
                                        is_invalid: 0,
                                    },
                                    //#if MODULE_NFC_AVAILABLE
                                    editUserNfcTags: API.get("nfc/config").authorized_tags.slice(),
                                    //#else
                                    editUserNfcTags: [],
                                    //#endif

                                    //#if MODULE_EV_AVAILABLE
                                    editUserEvs: API.get("ev/config").evs.slice(),
                                    //#else
                                    editUserEvs: [],
                                    //#endif
                                })
                            }
                            onAddGetChildren={() => [
                                <EditUserFormContent
                                    user={state.editUser}
                                    errorMessage={this.errorMessage(
                                        state.editUser,
                                    )}
                                    password="add"
                                    //#if MODULE_NFC_AVAILABLE
                                    users={state.users}
                                    nfcConfig={state.editUserNfcTags}
                                    onNfcConfig={(cfg) => this.setState({editUserNfcTags: cfg})}
                                    //#endif
                                    //#if MODULE_EV_AVAILABLE
                                    evConfig={state.editUserEvs}
                                    onEvConfig={(cfg) => {console.log(cfg); this.setState({editUserEvs: cfg});}}
                                    //#endif
                                    onUserChange={(changes) =>
                                        this.setState({
                                            editUser: {
                                                ...state.editUser,
                                                ...changes,
                                                is_invalid: 0
                                            },
                                        })
                                    }
                                />,
                            ]}
                            onAddCheck={async () => {
                                console.log("hier");
                                let is_invalid = await this.checkUsername(
                                    state.editUser,
                                    undefined,
                                );

                                return new Promise<boolean>((resolve) => {
                                    this.setState(
                                        {
                                            editUser: {
                                                ...state.editUser,
                                                is_invalid: is_invalid,
                                            },
                                        },
                                        () =>
                                            resolve(
                                                is_invalid == undefined ||
                                                    is_invalid == 0,
                                            ),
                                    );
                                });
                            }}
                            onAddSubmit={async () => {
                                // Re-read next_user_id and patch user and tags to be added
                                // in case another user was added while the add modal was open.
                                let next_user_id = API.get("users/config").next_user_id;
                                if (state.editUser.id != next_user_id) {
                                    for (let cfg of state.editUserNfcTags)
                                        if (cfg.user_id == state.editUser.id)
                                            cfg.user_id = next_user_id;

                                    state.editUser.id = next_user_id;
                                }

                                await add_user(state.editUser);

                                //#if MODULE_NFC_AVAILABLE
                                let nfc_config = API.get("nfc/config")
                                await API.save("nfc/config", {
                                    ...nfc_config,
                                    authorized_tags: state.editUserNfcTags
                                });
                                //#endif
                                //#if MODULE_EV_AVAILABLE
                                let ev_config = API.get("ev/config")
                                await API.save("ev/config", {
                                    ...ev_config,
                                    evs: state.editUserEvs
                                });
                                //#endif
                            }}
                        />
                    </FormRow>


                    <FormRow label={__("users.content.nfc_tags")}>
                        <Table
                            columnNames={[
                                __("users.content.nfc_tag_id"),
                                __("users.content.nfc_tag_type"),
                                __("users.content.nfc_table_assigned_user"),
                                __("users.content.nfc_last_seen"),
                            ]}
                            rows={API.get("nfc/config").authorized_tags.map(tag => {
                                const ownerName = tag.user_id == 0 ? "" : API.get("users/config").users.find(u => u.id == tag.user_id).display_name;

                                const seen = API.get("nfc/seen_tags").find(
                                    (s) => s.tag_id === tag.tag_id && s.tag_type === tag.tag_type,
                                );
                                const lastSeenText = seen
                                    ? __("nfc.content.last_seen") +
                                    util.format_timespan_ms(seen.last_seen) +
                                    __("nfc.content.last_seen_suffix")
                                    : "";

                                return {
                                    columnValues: [
                                        tag.tag_id,
                                        translate_unchecked(`nfc.content.type_${tag.tag_type}`),
                                        ownerName,
                                        lastSeenText,
                                    ],
                                    fieldNames: [
                                        __("users.content.nfc_tag_id"),
                                        __("users.content.nfc_tag_type"),
                                        __("users.content.nfc_table_assigned_user"),
                                        __("users.content.nfc_last_seen"),
                                    ],
                                    fieldValues: [
                                        tag.tag_id,
                                        translate_unchecked(`nfc.content.type_${tag.tag_type}`),
                                        ownerName,
                                        lastSeenText,
                                    ],
                                    hideRemoveButton: true
                                };
                            })}
                            addMessage={__("users.content.nfc_add_tag_message")(
                                    API.get("nfc/config").authorized_tags.length,
                                    MAX_TAGS)}
                        />
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export async function getAllUsernames() {
    return util
        .download("/users/all_usernames", true)
        .then((blob) => blob.arrayBuffer())
        .then((buffer) => {
            let usernames: string[] = [];
            let display_names: string[] = [];

            if (buffer.byteLength != 256 * 64) {
                console.log("Unexpected length of all_usernames!");
                return [null, null];
            }

            const decoder = new TextDecoder("utf-8");
            for (let i = 0; i < 256; ++i) {
                let view = new DataView(buffer, i * 64, 32);
                let username = decoder.decode(view).replace(/\0/g, "");

                view = new DataView(buffer, i * 64 + 32, 32);
                let display_name = decoder.decode(view).replace(/\0/g, "");

                usernames.push(username);
                display_names.push(display_name);
            }
            return [usernames, display_names];
        });
}

export function pre_init() {}

export function init() {}
