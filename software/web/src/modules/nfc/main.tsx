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

import * as util from "../../ts/util";
import * as API from "../../ts/api";
import { h, Fragment } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { InputText } from "../../ts/components/input_text";
import { ListGroup, ListGroupItem } from "react-bootstrap";
import { InputSelect } from "../../ts/components/input_select";
import { SubPage } from "../../ts/components/sub_page";
import { Table } from "../../ts/components/table";
import { FormRow } from "../../ts/components/form_row";
import { NavbarItem } from "../../ts/components/navbar_item";

export function NFCNavbar() {
    return (
        <NavbarItem name="nfc" module="nfc" title={__("nfc.navbar.nfc")} symbol={
            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor"><g transform="matrix(1.33 0 0 -1.33 -46.7 105)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"/><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"/></g></g></svg>
        } />);
}

type NFCConfig = API.getType['nfc/config'];

interface NFCState {
    userCfg: API.getType["users/config"];
    addTag: NFCConfig["authorized_tags"][0];
    editTag: NFCConfig["authorized_tags"][0];
}

export class NFC extends ConfigComponent<'nfc/config', {}, NFCState> {
    constructor() {
        super('nfc/config',
              __("nfc.script.save_failed"),
              __("nfc.script.reboot_content_changed"), {
                addTag: {
                    tag_id: "",
                    user_id: 0,
                    tag_type: "" as any,
                },
                editTag: {
                    tag_id: "",
                    user_id: 0,
                    tag_type: "" as any,
                },
            });

        util.addApiEventListener('users/config', () => {
            this.setState({userCfg: API.get('users/config')});
        });

       /* util.addApiEventListener('nfc/seen_tags', () => {
            console.log("nfc seen_tags");

            let x = API.get('nfc/seen_tags');


            for(let i = 0; i < x.length; ++i) {
                console.log("nfc seen_tags " + i + " " + x[i].last_seen);
            }


            this.setState({seen_tags: x});
        });*/
    }

    render(props: {}, state: NFCConfig & NFCState) {
        if (!util.render_allowed() || !API.hasFeature("nfc"))
            return <SubPage name="nfc" />;

        const MAX_AUTHORIZED_TAGS = API.hasModule("esp32_ethernet_brick") ? 32 : 16;

        type NFCSeenTag = API.getType['nfc/seen_tags'][0];

        let seen_tags = API.get('nfc/seen_tags');
        let unauth_seen_tags: NFCSeenTag[] = [];

        let auth_seen_tags: NFCSeenTag[] = [];
        let auth_seen_ids: number[] = [];

        outer_loop:
            for(let i = 0; i < seen_tags.length; ++i) {
                if (seen_tags[i].tag_id == "")
                    continue;

                for (let auth_tag_idx = 0; auth_tag_idx < state.authorized_tags.length; ++auth_tag_idx) {
                    let auth_tag = state.authorized_tags[auth_tag_idx];
                    if (auth_tag.tag_type != seen_tags[i].tag_type)
                        continue;

                    if (auth_tag.tag_id != seen_tags[i].tag_id)
                        continue;

                    auth_seen_tags.push(seen_tags[i]);
                    auth_seen_ids.push(auth_tag_idx);

                    continue outer_loop;
                }

                unauth_seen_tags.push(seen_tags[i]);
            }

        return (
            <SubPage name="nfc">
                <ConfigForm id="nfc_config_form" title={__("nfc.content.nfc")} isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("nfc.content.deadtime")} label_muted={__("nfc.content.deadtime_muted")}>
                        <InputSelect items={[
                                ["0", __("nfc.content.deadtime_min")],
                                ["3", __("nfc.content.deadtime_3")],
                                ["10", __("nfc.content.deadtime_10")],
                                ["30", __("nfc.content.deadtime_30")],
                                ["60", __("nfc.content.deadtime_60")],
                                ["4294967295", __("nfc.content.deadtime_max")],
                            ]}
                            value={state.deadtime_post_start}
                            onValue={(v) => this.setState({deadtime_post_start: parseInt(v)})}
                        />
                    </FormRow>
                    <FormRow label={__("nfc.content.tags")}>
                        <Table
                            tableTill="md"
                            columnNames={[__("nfc.content.table_tag_id"), __("nfc.content.table_tag_type"), __("nfc.content.table_user_id"), __("nfc.content.table_last_seen")]}
                            rows={state.authorized_tags.map((tag, i) =>
                                {
                                    let filtered_users = state.userCfg.users.filter((user) => user.id == tag.user_id);
                                    return {
                                    columnValues: [
                                        tag.tag_id.split(":").map((x, i) => i == 0 ? <>{x}</> : <><wbr/>:{x}</>),
                                        translate_unchecked(`nfc.content.type_${tag.tag_type}`),
                                        (tag.user_id == 0 || filtered_users.length == 0 )? __("nfc.script.not_assigned") : filtered_users[0].display_name,
                                        auth_seen_ids.indexOf(i) >= 0 ? __("nfc.content.last_seen") + util.format_timespan_ms(auth_seen_tags[auth_seen_ids.indexOf(i)].last_seen) + __("nfc.content.last_seen_suffix") : __("nfc.script.not_seen")
                                    ],
                                    editTitle: __("nfc.content.edit_tag_title"),
                                    onEditShow: async () => this.setState({editTag: {tag_id: tag.tag_id, user_id: tag.user_id, tag_type: tag.tag_type}}),
                                    onEditGetChildren: () => [<>
                                        <FormRow label={__("nfc.content.edit_tag_tag_id")}>
                                             <InputText value={state.editTag.tag_id}
                                                onValue={(v) => this.setState({editTag: {...state.editTag, tag_id: v}})}
                                                minLength={8} maxLength={29}
                                                pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                                                invalidFeedback={__("nfc.content.tag_id_invalid_feedback")}
                                                required />
                                        </FormRow>
                                        <FormRow label={__("nfc.content.edit_tag_tag_type")}>
                                             <InputSelect items={[
                                                    ["0",__("nfc.content.type_0")],
                                                    ["1",__("nfc.content.type_1")],
                                                    ["2",__("nfc.content.type_2")],
                                                    ["3",__("nfc.content.type_3")],
                                                    ["4",__("nfc.content.type_4")],
                                                ]}
                                                value={state.editTag.tag_type.toString()}
                                                onValue={(v) => this.setState({editTag: {...state.editTag, tag_type: parseInt(v)}})} />
                                        </FormRow>
                                        <FormRow label={__("nfc.content.edit_tag_user_id")}>
                                                <InputSelect items={state.userCfg.users.map((u, j) => [u.id.toString(), j == 0 ? __("nfc.script.not_assigned") : u.display_name.toString()])}
                                                    value={state.editTag.user_id.toString()}
                                                    onValue={(v) => this.setState({editTag: {...state.editTag, user_id: parseInt(v)}})}
                                                    />
                                                <div class="mt-2" style={state.editTag.user_id == 0 ? undefined : 'visibility: hidden'}>{__("nfc.script.not_assigned_desc")}</div>
                                        </FormRow>
                                    </>],
                                    onEditSubmit: async () => {
                                        this.setState({authorized_tags: state.authorized_tags.map((tag, k) => i === k ? state.editTag : tag)});
                                        this.setDirty(true);
                                    },
                                    onRemoveClick: async () => {
                                        this.setState({authorized_tags: state.authorized_tags.filter((v, idx) => idx != i)});
                                        this.setDirty(true);
                                    }
                                }})
                            }
                            addEnabled={state.authorized_tags.length < MAX_AUTHORIZED_TAGS}
                            addTitle={__("nfc.content.add_tag_title")}
                            addMessage={__("nfc.content.add_tag_prefix") + state.authorized_tags.length + __("nfc.content.add_tag_infix") + MAX_AUTHORIZED_TAGS + __("nfc.content.add_tag_suffix")}
                            onAddShow={async () => this.setState({addTag: {tag_id: "", user_id: 0, tag_type: "" as any}})}
                            onAddGetChildren={() => [<>
                                <FormRow label={__("nfc.content.add_tag_seen_tags")}>
                                    {unauth_seen_tags.length > 0 ?
                                        <ListGroup>{
                                            unauth_seen_tags.map(t => <ListGroupItem key={t.tag_id} action type="button" onClick={() => this.setState({addTag: {...state.addTag, tag_id: t.tag_id, tag_type: t.tag_type}})}>
                                                <h5 class="mb-1 pr-2">{t.tag_id}</h5>
                                                <div class="d-flex w-100 justify-content-between">
                                                    <span class="text-left">{translate_unchecked(`nfc.content.type_${t.tag_type}`)}</span>
                                                    <span class="text-right">{__("nfc.content.last_seen") + util.format_timespan_ms(t.last_seen) + __("nfc.content.last_seen_suffix")}</span>
                                                </div>
                                                </ListGroupItem>)
                                        }</ListGroup>
                                        : <span>{__("nfc.content.add_tag_description")}</span>}
                                </FormRow>
                                <FormRow label={__("nfc.content.add_tag_tag_id")}>
                                    <InputText
                                        value={state.addTag.tag_id}
                                        onValue={(v) => this.setState({addTag: {...state.addTag, tag_id: v}})}
                                        minLength={8} maxLength={29}
                                        pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                                        invalidFeedback={__("nfc.content.tag_id_invalid_feedback")}
                                        required />
                                </FormRow>
                                <FormRow label={__("nfc.content.add_tag_tag_type")}>
                                    <InputSelect
                                        items={[
                                            ["0",__("nfc.content.type_0")],
                                            ["1",__("nfc.content.type_1")],
                                            ["2",__("nfc.content.type_2")],
                                            ["3",__("nfc.content.type_3")],
                                            ["4",__("nfc.content.type_4")],
                                        ]}
                                        placeholder={__("nfc.content.select_type")}
                                        value={state.addTag.tag_type.toString()}
                                        onValue={(v) => this.setState({addTag: {...state.addTag, tag_type: parseInt(v)}})}
                                        required />
                                </FormRow>
                                <hr/>
                                <FormRow label={__("nfc.content.add_tag_user_id")}>
                                    <InputSelect
                                        items={state.userCfg.users.map((u, j) => [u.id.toString(), j == 0 ? __("nfc.script.not_assigned") : u.display_name.toString()])}
                                        value={state.addTag.user_id.toString()}
                                        onValue={(v) => this.setState({addTag: {...state.addTag, user_id: parseInt(v)}})}
                                        required />
                                    <div class="mt-2" style={state.addTag.user_id == 0 ? undefined : 'visibility: hidden'}>{__("nfc.script.not_assigned_desc")}</div>
                                </FormRow>
                            </>]}
                            onAddSubmit={async () => {
                                this.setState({authorized_tags: state.authorized_tags.concat({tag_id: state.addTag.tag_id, user_id: state.addTag.user_id, tag_type: state.addTag.tag_type})});
                                this.setDirty(true);
                            }}
                            />
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
