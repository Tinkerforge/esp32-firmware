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
import { useEffect, useRef } from "preact/hooks";
import { useSignal } from "@preact/signals";
//#endif


// Get NFC tags from managed chargers, merged with local seen tags.
// Returns all seen tags (local + remote), deduplicated.
export type NFCSeenTag = API.getType['nfc/seen_tags'][0] & { charger_name?: string | null };
export async function get_all_seen_tags(): Promise<NFCSeenTag[]> {
    let all_tags: NFCSeenTag[] = API.get('nfc/seen_tags')?.map(t => {
        const tag: NFCSeenTag = {...t, charger_name: null};
        return tag;
    });

    if (
        API.hasModule("charge_manager") &&
        API.get("charge_manager/config").enable_central_management
    ) {
        let cm_state = API.get_unchecked("charge_manager/state");
        if (cm_state && cm_state.chargers) {
            try {
                const response = await fetch("/charge_manager/auth_info");
                if (response.ok) {
                    const auth_info_data: {
                        ti: string;
                        tt: number;
                        ts: number;
                        am?: number;
                    }[][] = await response.json();
                    for (
                        let charger_idx = 0;
                        charger_idx < auth_info_data.length &&
                        charger_idx < cm_state.chargers.length;
                        charger_idx++
                    ) {
                        const charger_name = cm_state.chargers[charger_idx].n;
                        for (const auth_info of auth_info_data[charger_idx]) {
                            if (!auth_info.ti || auth_info.ti === "") continue;
                            // Skip EV MACs
                            if (auth_info.am === 4 || auth_info.am === 5) continue;
                            // Check if tag already exists
                            const idx = all_tags.findIndex(
                                (t) =>
                                    t.tag_id === auth_info.ti &&
                                    t.tag_type === auth_info.tt,
                            );
                            if (idx !== -1) {
                                // If remote tag is newer, update last_seen
                                if (all_tags[idx].last_seen > auth_info.ts) {
                                    all_tags[idx] = {
                                        charger_name: charger_name,
                                        tag_id: auth_info.ti,
                                        tag_type: auth_info.tt,
                                        last_seen: auth_info.ts,
                                    };
                                }
                                continue;
                            }
                            all_tags.push({
                                tag_id: auth_info.ti,
                                tag_type: auth_info.tt,
                                last_seen: auth_info.ts,
                                charger_name: charger_name,
                            });
                        }
                    }
                }
            } catch (e) {
                console.error("Failed to fetch charge_manager/auth_info:", e);
            }
        }
    }

    return all_tags;
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

interface NfcTagRef {
    tag_type: number;
    tag_id: string;
}

interface NfcTagChange {
    user_id: number;
    username: string;
    tags: NfcTagRef[];
}

interface EvRef {
    mac: string;
}

interface EvChange {
    user_id: number;
    username: string;
    evs: EvRef[];
}

interface UsersState {
    userSlotEnabled: boolean;
    addUser: User;
    editUser: User;
    addUserNfcTags: NfcTagRef[];
    editUserNfcTags: NfcTagRef[];
    nfcTagChanges: NfcTagChange[];
    nfcDeadtime: number;
    allSeenTags: NFCSeenTag[];
    editTagId: string;
    editTagType: number;
    addUserEvs: EvRef[];
    editUserEvs: EvRef[];
    evChanges: EvChange[];
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

function modify_user(user: User) {
    let { password, is_invalid, ...u } = user;
    return retry_once(
        () => API.call("users/modify", u, () => __("users.script.save_failed")),
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

function add_user(user: User) {
    let { password, is_invalid, ...u } = user;
    return retry_once(
        () => API.call("users/add", u, () => __("users.script.save_failed")),
        "users_add_failed",
    );
}

//#if MODULE_NFC_AVAILABLE
interface NfcTagsSectionProps {
    assignedTags: NfcTagRef[];
    seenTags: NFCSeenTag[];
    authorizedTags: API.getType["nfc/config"]["authorized_tags"];
    pendingTagChanges: NfcTagChange[];
    users: User[];
    currentUserId: number;
    currentUsername: string;
    editTagId: string;
    editTagType: number;
    onEditTagIdChange: (value: string) => void;
    onEditTagTypeChange: (value: number) => void;
    onRemoveTag: (index: number) => void;
    onAddTag: (tag: NfcTagRef) => void;
}

function NfcTagsSection({
    assignedTags,
    seenTags,
    authorizedTags,
    pendingTagChanges,
    users,
    currentUserId,
    currentUsername,
    editTagId,
    editTagType,
    onEditTagIdChange,
    onEditTagTypeChange,
    onRemoveTag,
    onAddTag,
}: NfcTagsSectionProps) {
    const getRows = (): TableRow[] => {
        return assignedTags.map((tag, j) => {
            const seen = seenTags.find(
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
                    onRemoveTag(j);
                    return true;
                },
            };
        });
    };

    const usedTagKeys = new Set(
        assignedTags.map((t) => `${t.tag_type}-${t.tag_id}`),
    );

    const getTagError = (t: {
        tag_type: number;
        tag_id: string;
    }): string | undefined => {
        const key = `${t.tag_type}-${t.tag_id}`;

        if (usedTagKeys.has(key)) {
            return __("component.discovery_result.already_added") as string;
        }

        const pendingUser = pendingTagChanges.find(
            (c) =>
                (currentUserId > 0
                    ? c.user_id !== currentUserId
                    : c.username !== currentUsername) &&
                c.tags.some(
                    (tag) =>
                        tag.tag_id === t.tag_id && tag.tag_type === t.tag_type,
                ),
        );

        if (pendingUser) {
            const u = users.find((u) =>
                pendingUser.user_id > 0
                    ? u.id === pendingUser.user_id
                    : u.username === pendingUser.username,
            );
            const ownerName = u ? u.display_name : pendingUser.username;
            return __("users.content.nfc_tag_already_assigned")(
                ownerName,
            ) as string;
        }

        const authorizedOwner = authorizedTags.find(
            (at) =>
                at.tag_id === t.tag_id &&
                at.tag_type === t.tag_type &&
                at.user_id !== 0 &&
                (currentUserId <= 0 || at.user_id !== currentUserId),
        );

        if (authorizedOwner) {
            const u = users.find((u) => u.id === authorizedOwner.user_id);
            if (u) {
                return __("users.content.nfc_tag_already_assigned")(
                    u.display_name,
                ) as string;
            }
        }

        return undefined;
    };

    const MAX_TAGS = API.hasModule("esp32_ethernet_brick") ? 32 : 16;

    const perUserTagCount = assignedTags.length;

    // Compute effective total tag count reflecting pending changes and the current edit.
    // 1. Saved tags for users that have no pending change and are not the current user.
    const savedTagsCount = authorizedTags.filter((at) => {
        if (currentUserId > 0 && at.user_id === currentUserId) return false;
        if (
            pendingTagChanges.some(
                (c) => c.user_id > 0 && c.user_id === at.user_id,
            )
        )
            return false;
        return true;
    }).length;
    // 2. Pending tag changes from other users (not the one being edited right now).
    const pendingOtherTagsCount = pendingTagChanges
        .filter((c) =>
            currentUserId > 0
                ? c.user_id !== currentUserId
                : c.username !== currentUsername,
        )
        .reduce((sum, c) => sum + c.tags.length, 0);
    // 3. Current user's tags come from assignedTags (live-updated).
    const effectiveTagCount =
        savedTagsCount + pendingOtherTagsCount + perUserTagCount;

    const globalTagLimitReached = effectiveTagCount >= MAX_TAGS;

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
                addMessage={__("users.content.nfc_add_tag_message")(
                    effectiveTagCount,
                    MAX_TAGS,
                    perUserTagCount
                )}
                addEnabled={!globalTagLimitReached}
                addTitle={__("users.content.nfc_add_tag")}
                onAddShow={async () => {
                    onEditTagIdChange("");
                    onEditTagTypeChange(0);
                }}
                onAddGetChildren={() => {
                    let filtered = seenTags
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
                                                onClick={() => {
                                                    onEditTagIdChange(t.tag_id);
                                                    onEditTagTypeChange(
                                                        t.tag_type,
                                                    );
                                                }}
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
                                value={editTagId}
                                onValue={(v) => onEditTagIdChange(v)}
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
                                value={editTagType.toString()}
                                onValue={(v) =>
                                    onEditTagTypeChange(parseInt(v))
                                }
                            />
                        </FormRow>
                    </>
                }}
                onAddSubmit={async () => {
                    onAddTag({
                        tag_type: editTagType,
                        tag_id: editTagId
                            .toUpperCase()
                            .replace(/([0-9A-F]{2})(?!$|:)/g, "$1:"),
                    });
                    onEditTagIdChange("");
                    onEditTagTypeChange(0);
                }}
                onAddHide={async () => {
                    onEditTagIdChange("");
                    onEditTagTypeChange(0);
                }}
            />
        </FormRow>
    );
}
//#endif

//#if MODULE_EV_AVAILABLE
interface EvsSectionProps {
    assignedEvs: EvRef[];
    evs: API.getType["ev/config"]["evs"];
    pendingEvChanges: EvChange[];
    users: User[];
    currentUserId: number;
    currentUsername: string;
    onRemoveEv: (index: number) => void;
    onAddEv: (ev: EvRef) => void;
}

function EvsSection({
    assignedEvs,
    evs,
    pendingEvChanges,
    users,
    currentUserId,
    currentUsername,
    onRemoveEv,
    onAddEv,
}: EvsSectionProps) {
    // Look up the configured name of an EV by its MAC. Falls back to the MAC itself.
    const ev_name = (mac: string): string => {
        const ev = evs.find((e) => e.mac === mac);
        return ev ? ev.name : mac;
    };

    const getRows = (): TableRow[] => {
        return assignedEvs.map((ev, j) => {
            return {
                columnValues: [ev_name(ev.mac), ev.mac],
                fieldNames: [
                    __("users.content.ev_name"),
                    __("users.content.ev_mac"),
                ],
                fieldValues: [ev_name(ev.mac), ev.mac],
                hideRemoveButton: false,
                onRemoveClick: async () => {
                    onRemoveEv(j);
                    return true;
                },
            };
        });
    };

    const assignedSet = new Set(assignedEvs.map((m) => m.mac));

    // Determine which user (display name) an EV is currently assigned to,
    // taking pending (unsaved) changes into account. Returns undefined if the
    // EV is unassigned or assigned to the user that is currently being edited.
    const getEvOwnerName = (mac: string): string | undefined => {
        // A pending change of another user takes precedence over the saved state.
        const pendingUser = pendingEvChanges.find(
            (c) => ((currentUserId > 0) ? (c.user_id !== currentUserId) : (c.username !== currentUsername)) && (c.evs.some((m) => (m.mac === mac))),
        );

        if (pendingUser) {
            const u = users.find((u) => (pendingUser.user_id > 0) ? (u.id === pendingUser.user_id) : (u.username === pendingUser.username));
            return u ? u.display_name : pendingUser.username;
        }

        // Otherwise fall back to the assignment saved on the EV profile, unless
        // that owner has a pending change (which would already be handled above).
        const ev = evs.find((e) => e.mac === mac);
        if (ev && (ev.user_id !== 0) && (currentUserId <= 0 || ev.user_id !== currentUserId)) {
            const ownerHasPendingChange = pendingEvChanges.some((c) => (c.user_id > 0) && (c.user_id === ev.user_id));
            if (!ownerHasPendingChange) {
                const u = users.find((u) => (u.id === ev.user_id));
                if (u) return u.display_name;
            }
        }

        return undefined;
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
                addMessage={__("users.content.ev_add_mac_message")(
                    assignedEvs.length,
                    evs.length,
                )}
                addEnabled={true}
                addTitle={__("users.content.ev_add_mac")}
                hideSubmitButton={true}
                onAddShow={async () => {}}
                onAddGetChildren={() => {
                    // Offer all configured EVs that are not already assigned to
                    // this user. EVs assigned to another user are shown disabled.
                    const selectable = evs.filter((ev) => ev.mac !== "" && !assignedSet.has(ev.mac));

                    let items =
                        selectable.length > 0 ? (
                            selectable.map((ev) => {
                                const ownerName = getEvOwnerName(ev.mac);
                                const error = ownerName ? (__("users.content.ev_mac_already_assigned")(ownerName) as string) : undefined;
                                return (
                                    <DiscoveryResultItem
                                        key={ev.mac}
                                        type="submit"
                                        title={<div class="h5">{ev.name}</div>}
                                        labelAdd={<Plus />}
                                        error={error}
                                        onClick={() => {
                                            onAddEv({ mac: ev.mac });
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
                                title=""
                                error={__("users.content.ev_no_evs")}
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
    requirePassword: boolean;
    //#if MODULE_NFC_AVAILABLE
    nfcTags: NfcTagRef[];
    nfcConfig: API.getType["nfc/config"];
    pendingTagChanges: NfcTagChange[];
    users: User[];
    editTagId: string;
    editTagType: number;
    onEditTagIdChange: (value: string) => void;
    onEditTagTypeChange: (value: number) => void;
    onNfcTagsChange: (tags: NfcTagRef[]) => void;
    //#endif
    //#if MODULE_EV_AVAILABLE
    evs: EvRef[];
    evConfig: API.getType["ev/config"];
    pendingEvChanges: EvChange[];
    evUsers: User[];
    onEvsChange: (evs: EvRef[]) => void;
    //#endif
    onUserChange: (changes: Partial<User>) => void;
}

function EditUserFormContent({
    user,
    errorMessage,
    requirePassword,
    //#if MODULE_NFC_AVAILABLE
    nfcTags,
    nfcConfig,
    pendingTagChanges,
    users,
    editTagId,
    editTagType,
    onEditTagIdChange,
    onEditTagTypeChange,
    onNfcTagsChange,
    //#endif
    //#if MODULE_EV_AVAILABLE
    evs,
    evConfig,
    pendingEvChanges,
    evUsers,
    onEvsChange,
    //#endif
    onUserChange,
}: EditUserFormContentProps) {
    const seenTags = useSignal<NFCSeenTag[]>([]);
    useEffect(() => {
        const interval = setInterval(() => {
            get_all_seen_tags().then((tags) => (seenTags.value = tags));
        }, 500);
        return () => clearInterval(interval);
    });
    return (
        <>
            <FormRow
                label={__("users.content.edit_user_username")}
                label_muted={__("users.content.edit_user_username_desc")}
            >
                <InputText
                    value={user.username}
                    onValue={(v) => onUserChange({ username: v })}
                    minLength={1}
                    maxLength={32}
                    required
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
                    minLength={1}
                    maxLength={32}
                    required
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
                    required={requirePassword}
                    value={
                        user.password === undefined
                            ? user.digest_hash
                            : user.password
                    }
                    onValue={(v) => onUserChange({ password: v })}
                    clearPlaceholder={__("users.script.login_disabled")}
                    clearSymbol={<Slash />}
                    allowAPIClear
                />
            </FormRow>
            {/*#if MODULE_NFC_AVAILABLE*/}
            <NfcTagsSection
                assignedTags={nfcTags}
                seenTags={seenTags.value}
                authorizedTags={nfcConfig.authorized_tags}
                pendingTagChanges={pendingTagChanges}
                users={users}
                currentUserId={user.id}
                currentUsername={user.username}
                editTagId={editTagId}
                editTagType={editTagType}
                onEditTagIdChange={onEditTagIdChange}
                onEditTagTypeChange={onEditTagTypeChange}
                onRemoveTag={(j) =>
                    onNfcTagsChange(nfcTags.filter((_, k) => k !== j))
                }
                onAddTag={(tag) => onNfcTagsChange(nfcTags.concat(tag))}
            />
            {/*#endif*/}
            {/*#if MODULE_EV_AVAILABLE*/}
            <EvsSection
                assignedEvs={evs}
                evs={evConfig.evs}
                pendingEvChanges={pendingEvChanges}
                users={evUsers}
                currentUserId={user.id}
                currentUsername={user.username}
                onRemoveEv={(j) =>
                    onEvsChange(evs.filter((_, k) => k !== j))
                }
                onAddEv={(ev) => onEvsChange(evs.concat(ev))}
            />
            {/*#endif*/}
        </>
    );
}

interface AddUserFormContentProps {
    user: User;
    errorMessage: string | undefined;
    //#if MODULE_NFC_AVAILABLE
    nfcTags: NfcTagRef[];
    nfcConfig: API.getType["nfc/config"];
    pendingTagChanges: NfcTagChange[];
    users: User[];
    editTagId: string;
    editTagType: number;
    onEditTagIdChange: (value: string) => void;
    onEditTagTypeChange: (value: number) => void;
    onNfcTagsChange: (tags: NfcTagRef[]) => void;
    //#endif
    //#if MODULE_EV_AVAILABLE
    evs: EvRef[];
    evConfig: API.getType["ev/config"];
    pendingEvChanges: EvChange[];
    evUsers: User[];
    onEvsChange: (evs: EvRef[]) => void;
    //#endif
    onUserChange: (changes: Partial<User>) => void;
}

function AddUserFormContent({
    user,
    errorMessage,
    //#if MODULE_NFC_AVAILABLE
    nfcTags,
    nfcConfig,
    pendingTagChanges,
    users,
    editTagId,
    editTagType,
    onEditTagIdChange,
    onEditTagTypeChange,
    onNfcTagsChange,
    //#endif
    //#if MODULE_EV_AVAILABLE
    evs,
    evConfig,
    pendingEvChanges,
    evUsers,
    onEvsChange,
    //#endif
    onUserChange,
}: AddUserFormContentProps) {
    const seenTags = useSignal<NFCSeenTag[]>([]);
    useEffect(() => {
        const interval = setInterval(() => {
            get_all_seen_tags().then((tags) => (seenTags.value = tags));
        }, 500);
        return () => clearInterval(interval);
    });
    return (
        <>
            <FormRow
                label={__("users.content.add_user_username")}
                label_muted={__("users.content.add_user_username_desc")}
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
                label={__("users.content.add_user_display_name")}
                label_muted={__("users.content.add_user_display_name_desc")}
            >
                <InputText
                    value={user.display_name}
                    onValue={(v) => onUserChange({ display_name: v })}
                    required
                    minLength={1}
                    maxLength={32}
                />
            </FormRow>
            <FormRow label={__("users.content.add_user_current")}>
                <InputFloat
                    unit="A"
                    value={user.current}
                    onValue={(v) => onUserChange({ current: v })}
                    digits={3}
                    min={6000}
                    max={32000}
                />
            </FormRow>
            <FormRow label={__("users.content.add_user_password")}>
                <InputPassword
                    maxLength={64}
                    value={user.password}
                    onValue={(v) => onUserChange({ password: v })}
                    hideClear
                    placeholder={__("users.content.add_user_password_desc")}
                />
            </FormRow>
            {/*#if MODULE_NFC_AVAILABLE*/}
            <NfcTagsSection
                assignedTags={nfcTags}
                seenTags={seenTags.value}
                authorizedTags={nfcConfig.authorized_tags}
                pendingTagChanges={pendingTagChanges}
                users={users}
                currentUserId={-1}
                currentUsername={user.username}
                editTagId={editTagId}
                editTagType={editTagType}
                onEditTagIdChange={onEditTagIdChange}
                onEditTagTypeChange={onEditTagTypeChange}
                onRemoveTag={(j) =>
                    onNfcTagsChange(nfcTags.filter((_, k) => k !== j))
                }
                onAddTag={(tag) => onNfcTagsChange(nfcTags.concat(tag))}
            />
            {/*#endif*/}
            {/*#if MODULE_EV_AVAILABLE*/}
            <EvsSection
                assignedEvs={evs}
                evs={evConfig.evs}
                pendingEvChanges={pendingEvChanges}
                users={evUsers}
                currentUserId={-1}
                currentUsername={user.username}
                onRemoveEv={(j) =>
                    onEvsChange(evs.filter((_, k) => k !== j))
                }
                onAddEv={(ev) => onEvsChange(evs.concat(ev))}
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
                addUser: {
                    id: -1,
                    roles: 0xffff,
                    username: "",
                    display_name: "",
                    current: 32000,
                    digest_hash: "",
                    password: "",
                    is_invalid: 0,
                },
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
                addUserNfcTags: [],
                editUserNfcTags: [],
                nfcTagChanges: [],
                nfcDeadtime: 0,
                allSeenTags: [],
                editTagId: "",
                editTagType: 0,
                addUserEvs: [],
                editUserEvs: [],
                evChanges: [],
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
        util.addApiEventListener("nfc/seen_tags", () => {
            this.refreshSeenTags();
        });

        this.refreshSeenTags();

        util.addApiEventListener("nfc/config", () => {
            this.setState({
                nfcDeadtime: API.get("nfc/config").deadtime_post_start,
            });
        });
        //#endif

        //#if MODULE_EV_AVAILABLE
        util.addApiEventListener("ev/config", () => {
            this.forceUpdate();
        });
        //#endif
    }

    refreshSeenTags = () => {
        get_all_seen_tags().then((tags) =>
            this.setState({ allSeenTags: tags }),
        );
    };

    isChangedUser(changed_user: User): boolean {
        let users = API.get("users/config").users;

        for (let user of users) {
            if (
                user.username == changed_user.username &&
                user.id == changed_user.id
            )
                return false;
        }

        return true;
    }

    override async isSaveAllowed(cfg: UsersConfig): Promise<boolean> {
        let all_usernames = await getAllUsernames();
        let save_allowed = true;
        let new_users = cfg.users.slice();

        for (let i = 0; i < cfg.users.length; i++) {
            new_users[i].is_invalid = 0;

            for (let a = 0; a < cfg.users.length; a++) {
                if (
                    this.isChangedUser(cfg.users[i]) &&
                    cfg.users[i].username == cfg.users[a].username &&
                    a != i
                ) {
                    new_users[i].is_invalid = 1;
                    this.setState({ users: new_users });
                    save_allowed = false;
                    break;
                } else if (
                    this.isChangedUser(cfg.users[i]) &&
                    cfg.next_user_id == 0
                ) {
                    new_users[i].is_invalid = 3;
                    this.setState({ users: new_users });
                    save_allowed = false;
                    break;
                }
            }

            for (let user of all_usernames[0]) {
                if (
                    this.isChangedUser(cfg.users[i]) &&
                    cfg.users[i].username == user &&
                    (cfg.users[i].is_invalid == undefined ||
                        cfg.users[i].is_invalid == 0)
                ) {
                    new_users[i].is_invalid = 2;
                    this.setState({ users: new_users });
                    save_allowed = false;
                    break;
                }
            }
        }

        return save_allowed;
    }

    user_has_password(u: User) {
        return (
            (u.digest_hash == null && // user is known and has password set, which is censored
                u.password !== "") || // password will not be removed, an empty string as password would mean to remove the password
            (u.digest_hash == "" && // user is new and/or has currently no password set
                u.password !== undefined &&
                u.password !== null && // password will be changed/set
                u.password !== "")
        ); // password will not be removed, an empty string as password would mean to remove the password
    }

    get_password_replacement(u: User) {
        if (!this.user_has_password(u)) {
            return "";
        }

        if (u.password !== undefined && u.password !== null) {
            return "\u2022".repeat(u.password.length);
        }

        return (
            <span class="text-muted">
                {__("component.input_password.unchanged")}
            </span>
        );
    }

    http_auth_allowed() {
        return (
            this.state as Readonly<
                UsersState & UsersConfig & ConfigComponentState
            >
        ).users.some((u) => this.user_has_password(u));
    }

    override async sendSave(topic: "users/config", new_config: UsersConfig) {
        let old_config = API.get("users/config");
        new_config.http_auth_enabled &&= this.http_auth_allowed();
        if (old_config.http_auth_enabled && !new_config.http_auth_enabled) {
            // If we want to disable authentication, do this first,
            // to make sure authentication is never enabled
            // while no user without password is configured.
            // Don't show the reboot modal in this case.
            await API.call_unchecked(
                "users/http_auth_update",
                {
                    enabled: new_config.http_auth_enabled,
                },
                () => __("users.script.save_failed"),
            );
        }

        if (
            new_config.users[0].display_name ==
            __("charge_tracker.script.unknown_user")
        )
            new_config.users[0].display_name = "Anonymous";

        if (new_config.users[0].display_name === "")
            await modify_unknown_user("Anonymous");
        else if (
            new_config.users[0].display_name != old_config.users[0].display_name
        )
            await modify_unknown_user(new_config.users[0].display_name);

        let ids_to_remove = old_config.users
            .slice(1)
            .filter(
                (uOld) =>
                    !new_config.users
                        .slice(1)
                        .some((uNew) => uOld.id == uNew.id),
            )
            .map((uOld) => uOld.id);
        let users_to_modify = new_config.users
            .slice(1)
            .filter((uNew) =>
                old_config.users.slice(1).some((uOld) => uNew.id == uOld.id),
            );
        let users_to_add = new_config.users
            .slice(1)
            .filter(
                (uNew) =>
                    !old_config.users
                        .slice(1)
                        .some((uOld) => uNew.id == uOld.id),
            );

        for (let i of ids_to_remove) {
            await remove_user(i);
        }

        for (let u of users_to_modify) {
            // Don't hash if u.password is falsy, i.e. null, undefined or the empty string
            u.digest_hash = u.password
                ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password)
                : u.password;
            // Always send digest_hash, but as null if we don't want to change it.
            // digest_hash can be undefined if this user was not modified.
            if (u.digest_hash === undefined) u.digest_hash = null;
            await modify_user(u);
        }

        let next_user_id = API.get("users/config").next_user_id;

        outer_loop: for (let u of users_to_add) {
            // Don't hash if u.password is falsy, i.e. null, or the empty string.
            // u.password can't be undefined (as is handled above when modifying users),
            // because adding a user sets password to "" if nothing was entered.
            u.digest_hash = u.password
                ? YaMD5.YaMD5.hashStr(u.username + ":esp32-lib:" + u.password)
                : u.password;
            u.id = next_user_id;
            await add_user(u);
            for (let i = 0; i < 20; ++i) {
                if (API.get("users/config").next_user_id != next_user_id) {
                    next_user_id = API.get("users/config").next_user_id;
                    continue outer_loop;
                }
                await util.wait(100);
            }
            // fallback: just assume the next id is free
            next_user_id = Math.max(1, (next_user_id + 1) % 256);
        }

        //#if MODULE_EVSE_COMMON_AVAILABLE
        await API.save(
            "evse/user_enabled",
            { enabled: this.state.userSlotEnabled },
            () => __("evse.script.save_failed")
        );
        //#endif

        //#if MODULE_NFC_AVAILABLE
        if (
            (this.state.nfcTagChanges && this.state.nfcTagChanges.length > 0) ||
            ids_to_remove.length > 0 ||
            this.state.nfcDeadtime !== API.get("nfc/config").deadtime_post_start
        ) {
            let nfc_config = API.get("nfc/config");
            let authorized_tags = nfc_config.authorized_tags.slice();

            // Remove NFC tags for deleted users
            for (const id of ids_to_remove) {
                authorized_tags = authorized_tags.filter(
                    (t) => t.user_id !== id,
                );
            }

            // Apply pending NFC tag changes
            for (const change of this.state.nfcTagChanges) {
                let userId = change.user_id;
                const remove_idx = ids_to_remove.find((v) => v === userId);
                // Skip changes for users that are being deleted
                if (remove_idx !== undefined && remove_idx !== -1) {
                    continue;
                }

                // For new users, find the real ID by username
                if (userId <= 0) {
                    let currentUsers = API.get("users/config").users;
                    let found = currentUsers.find(
                        (u) => u.username === change.username,
                    );
                    if (found) {
                        userId = found.id;
                    } else {
                        continue;
                    }
                }

                // Remove old tags for this user
                authorized_tags = authorized_tags.filter(
                    (t) => t.user_id !== userId,
                );

                // Add new tags
                for (const tag of change.tags) {
                    authorized_tags.push({
                        user_id: userId,
                        tag_type: tag.tag_type,
                        tag_id: tag.tag_id,
                    });
                }
            }

            await API.save(
                "nfc/config",
                {
                    ...nfc_config,
                    authorized_tags: authorized_tags,
                    deadtime_post_start: this.state.nfcDeadtime,
                },
                () => __("users.script.save_failed"),
            );
            this.setState({ nfcTagChanges: [] });
        }
        //#endif

        //#if MODULE_EV_AVAILABLE
        if ((this.state.evChanges && this.state.evChanges.length > 0) || (ids_to_remove.length > 0)) {
            let ev_config = API.get("ev/config");
            // Work on a copy of the EV profiles; the user assignment is stored
            // directly on each profile via its user_id.
            let evs = ev_config.evs.map((e) => ({ ...e }));

            // Clear the assignment of EVs that belonged to deleted users.
            for (const id of ids_to_remove) {
                for (const ev of evs) {
                    if (ev.user_id === id) ev.user_id = 0;
                }
            }

            // Apply pending EV assignment changes.
            for (const change of this.state.evChanges) {
                let userId = change.user_id;
                const remove_idx = ids_to_remove.find((v) => (v === userId));
                // Skip changes for users that are being deleted
                if (remove_idx !== undefined && remove_idx !== -1) {
                    continue;
                }

                // For new users, find the real ID by username
                if (userId <= 0) {
                    let currentUsers = API.get("users/config").users;
                    let found = currentUsers.find((u) => (u.username === change.username));
                    if (found) {
                        userId = found.id;
                    } else {
                        continue;
                    }
                }

                // Clear this user's previous assignments, then assign the EVs
                // that are now selected for the user.
                const wanted = new Set(change.evs.map((m) => m.mac));
                for (const ev of evs) {
                    if (ev.user_id === userId && !wanted.has(ev.mac)) {
                        ev.user_id = 0;
                    }
                }
                for (const ev of evs) {
                    if (wanted.has(ev.mac)) {
                        ev.user_id = userId;
                    }
                }
            }

            await API.save(
                "ev/config",
                { evs: evs },
                () => __("users.script.save_failed"),
            );
            this.setState({ evChanges: [] });
        }
        //#endif

        // This is the last API call to be done. Show reboot modal here.
        await API.call_unchecked(
            "users/http_auth_update",
            {
                enabled: new_config.http_auth_enabled,
            },
            () => __("users.script.save_failed"),
            () => __("users.script.reboot_content_changed")
        );
    }

    setUser(i: number, val: Partial<User>) {
        // We have to copy the users array here to make sure the change detection in sendSave works.
        let users = this.state.users.slice(0);

        users[i] = { ...users[i], ...val };
        this.setState({ users: users });
    }

    async checkUsername(user: User, ignore_i: number): Promise<number> {
        for (let i = 0; i < this.state.users.length; ++i) {
            if (
                i != ignore_i &&
                this.state.users[i].username.trim() == user.username.trim()
            ) {
                return 1;
            }
        }

        if (API.get("users/config").next_user_id == 0) {
            return 3;
        }

        if (this.isChangedUser(user)) {
            let all_usernames = await getAllUsernames();

            for (let i = 0; i < all_usernames[0].length; ++i) {
                if (all_usernames[0][i].trim() == user.username.trim()) {
                    return 2;
                }
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

//#if MODULE_NFC_AVAILABLE
        let seen_tags = state.allSeenTags;
        let nfc_config = API.get("nfc/config");
//#endif

//#if MODULE_EV_AVAILABLE
        let ev_config = API.get("ev/config");
//#endif

        let evse_user_component = null;
//#if MODULE_EVSE_COMMON_AVAILABLE
        evse_user_component = (
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
        );
//#endif

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

                    {evse_user_component}

                    <FormRow label={__("users.content.unknown_username")}>
                        <InputPassword
                            maxLength={32}
                            value={
                                state.users[0].display_name == "Anonymous"
                                    ? __("charge_tracker.script.unknown_user")
                                    : state.users[0].display_name
                            }
                            onValue={(v) =>
                                this.setUser(0, { display_name: v })
                            }
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
                                this.setDirty(true);
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
                                        this.user_has_password(user) ? (
                                            <Check />
                                        ) : (
                                            ""
                                        ),
                                    ],
                                    fieldValues: [
                                        user.username,
                                        user.display_name,
                                        util.toLocaleFixed(
                                            user.current / 1000,
                                            3,
                                        ) + " A",
                                        this.user_has_password(user) ? (
                                            this.get_password_replacement(user)
                                        ) : (
                                            <span class="text-muted">
                                                {__("users.script.login_disabled")}
                                            </span>
                                        ),
                                    ],
                                    editTitle: __("users.content.edit_user_title"),
                                    onEditShow: async () => {
                                        let nfcTags: NfcTagRef[] = [];
                                        //#if MODULE_NFC_AVAILABLE
                                        let pendingChange =
                                            state.nfcTagChanges.find(
                                                (c) =>
                                                    (user.id > 0 &&
                                                        c.user_id ===
                                                            user.id) ||
                                                    (user.id <= 0 &&
                                                        c.username ===
                                                            user.username),
                                            );
                                        if (pendingChange) {
                                            nfcTags =
                                                pendingChange.tags.slice();
                                        } else if (user.id > 0) {
                                            nfcTags = nfc_config.authorized_tags
                                                .filter(
                                                    (t) =>
                                                        t.user_id === user.id,
                                                )
                                                .map((t) => ({
                                                    tag_type: t.tag_type,
                                                    tag_id: t.tag_id,
                                                }));
                                        }
                                        //#endif
                                        let evs: EvRef[] = [];
                                        //#if MODULE_EV_AVAILABLE
                                        let pendingEvChange = state.evChanges.find((c) =>(user.id > 0 && (c.user_id === user.id)) || (user.id <= 0 && c.username === user.username));
                                        if (pendingEvChange) {
                                            evs = pendingEvChange.evs.slice();
                                        } else if (user.id > 0) {
                                            evs = ev_config.evs.filter((e) => (e.user_id === user.id)).map((e) => ({ mac: e.mac }));
                                        }
                                        //#endif
                                        this.setState({
                                            editUser: {
                                                id: user.id,
                                                roles: user.roles,
                                                username: user.username,
                                                display_name: user.display_name,
                                                current: user.current,
                                                digest_hash: user.digest_hash,
                                                password: user.password,
                                                is_invalid: user.is_invalid,
                                            },
                                            editUserNfcTags: nfcTags,
                                            editUserEvs: evs,
                                        });
                                    },
                                    onEditGetChildren: () => [
                                        <EditUserFormContent
                                            user={state.editUser}
                                            errorMessage={this.errorMessage(
                                                state.editUser,
                                            )}
                                            requirePassword={this.require_password(
                                                state.editUser,
                                            )}
                                            //#if MODULE_NFC_AVAILABLE
                                            nfcTags={state.editUserNfcTags}
                                            nfcConfig={nfc_config}
                                            pendingTagChanges={
                                                state.nfcTagChanges
                                            }
                                            users={state.users}
                                            editTagId={state.editTagId}
                                            editTagType={state.editTagType}
                                            onEditTagIdChange={(v) =>
                                                this.setState({
                                                    editTagId: v,
                                                })
                                            }
                                            onEditTagTypeChange={(v) =>
                                                this.setState({
                                                    editTagType: v,
                                                })
                                            }
                                            onNfcTagsChange={(tags) =>
                                                this.setState({
                                                    editUserNfcTags: tags,
                                                })
                                            }
                                            //#endif
                                            //#if MODULE_EV_AVAILABLE
                                            evs={state.editUserEvs}
                                            evConfig={ev_config}
                                            pendingEvChanges={state.evChanges}
                                            evUsers={state.users}
                                            onEvsChange={(evs) =>
                                                this.setState({
                                                    editUserEvs: evs,
                                                })
                                            }
                                            //#endif
                                            onUserChange={(changes) =>
                                                this.setState({
                                                    editUser: {
                                                        ...state.editUser,
                                                        ...changes,
                                                    },
                                                })
                                            }
                                        />,
                                    ],
                                    onEditCheck: async () => {
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
                                        this.setUser(i + 1, state.editUser);
                                        //#if MODULE_NFC_AVAILABLE
                                        let nfcTagChanges =
                                            state.nfcTagChanges.filter((c) => {
                                                if (state.editUser.id > 0)
                                                    return (
                                                        c.user_id !==
                                                        state.editUser.id
                                                    );
                                                return (
                                                    c.username !==
                                                    state.editUser.username
                                                );
                                            });
                                        nfcTagChanges.push({
                                            user_id: state.editUser.id,
                                            username: state.editUser.username,
                                            tags: state.editUserNfcTags.slice(),
                                        });
                                        this.setState({
                                            nfcTagChanges: nfcTagChanges,
                                        });
                                        //#endif
                                        //#if MODULE_EV_AVAILABLE
                                        let evChanges =
                                            state.evChanges.filter((c) => {
                                                if (state.editUser.id > 0) {
                                                    return c.user_id !== state.editUser.id;
                                                }
                                                return c.username !== state.editUser.username;
                                            });
                                        evChanges.push({
                                            user_id: state.editUser.id,
                                            username: state.editUser.username,
                                            evs: state.editUserEvs.slice(),
                                        });
                                        this.setState({
                                            evChanges: evChanges,
                                        });
                                        //#endif
                                        this.setDirty(true);
                                    },
                                    onRemoveClick: async () => {
                                        this.setState({
                                            users: state.users.filter(
                                                (v, idx) => idx != i + 1,
                                            ),
                                        });
                                        this.setDirty(true);
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
                                    addUser: {
                                        id: -1,
                                        roles: 0xffff,
                                        username: "",
                                        display_name: "",
                                        current: 32000,
                                        digest_hash: "",
                                        password: "",
                                        is_invalid: 0,
                                    },
                                    addUserNfcTags: [],
                                    addUserEvs: [],
                                })
                            }
                            onAddGetChildren={() => [
                                <AddUserFormContent
                                    user={state.addUser}
                                    errorMessage={this.errorMessage(
                                        state.addUser,
                                    )}
                                    //#if MODULE_NFC_AVAILABLE
                                    nfcTags={state.addUserNfcTags}
                                    nfcConfig={nfc_config}
                                    pendingTagChanges={state.nfcTagChanges}
                                    users={state.users}
                                    editTagId={state.editTagId}
                                    editTagType={state.editTagType}
                                    onEditTagIdChange={(v) =>
                                        this.setState({ editTagId: v })
                                    }
                                    onEditTagTypeChange={(v) =>
                                        this.setState({ editTagType: v })
                                    }
                                    onNfcTagsChange={(tags) =>
                                        this.setState({
                                            addUserNfcTags: tags,
                                        })
                                    }
                                    //#endif
                                    //#if MODULE_EV_AVAILABLE
                                    evs={state.addUserEvs}
                                    evConfig={ev_config}
                                    pendingEvChanges={state.evChanges}
                                    evUsers={state.users}
                                    onEvsChange={(evs) =>
                                        this.setState({
                                            addUserEvs: evs,
                                        })
                                    }
                                    //#endif
                                    onUserChange={(changes) =>
                                        this.setState({
                                            addUser: {
                                                ...state.addUser,
                                                ...changes,
                                            },
                                        })
                                    }
                                />,
                            ]}
                            onAddCheck={async () => {
                                let is_invalid = await this.checkUsername(
                                    state.addUser,
                                    undefined,
                                );

                                return new Promise<boolean>((resolve) => {
                                    this.setState(
                                        {
                                            addUser: {
                                                ...state.addUser,
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
                                this.setState({
                                    users: state.users.concat({
                                        ...state.addUser,
                                        id: -1,
                                        roles: 0xffff,
                                    }),
                                });
                                //#if MODULE_NFC_AVAILABLE
                                if (state.addUserNfcTags.length > 0) {
                                    let nfcTagChanges =
                                        state.nfcTagChanges.slice();
                                    nfcTagChanges.push({
                                        user_id: -1,
                                        username: state.addUser.username,
                                        tags: state.addUserNfcTags.slice(),
                                    });
                                    this.setState({
                                        nfcTagChanges: nfcTagChanges,
                                    });
                                }
                                //#endif
                                //#if MODULE_EV_AVAILABLE
                                if (state.addUserEvs.length > 0) {
                                    let evChanges = state.evChanges.slice();
                                    evChanges.push({
                                        user_id: -1,
                                        username: state.addUser.username,
                                        evs: state.addUserEvs.slice(),
                                    });
                                    this.setState({
                                        evChanges: evChanges,
                                    });
                                }
                                //#endif
                                this.setDirty(true);
                            }}
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
