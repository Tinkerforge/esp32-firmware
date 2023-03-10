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

import * as util from "../../ts/util";
import * as API from "../../ts/api";

import { h, render, Fragment } from "preact";
import { translate_unchecked, __ } from "../../ts/translation";

import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { FormGroup } from "../../ts/components/form_group";
import { InputText } from "../../ts/components/input_text";
import { Button, Card, ListGroup, ListGroupItem, Modal } from "react-bootstrap";
import { InputSelect } from "src/ts/components/input_select";
import { ItemModal } from "src/ts/components/item_modal";

const MAX_AUTHORIZED_TAGS = 16;

let nfc_card_symbol = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor" class="feather feather-nfc"><g transform="matrix(1.33 0 0 -1.33 -46.7 105)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"/><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"/></g></g></svg>;
let nfc_card_delete_symbol = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-user-x mr-2"><g transform="matrix(1.33 0 0 -1.33 -51.7 105) translate(0 0)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"></path><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"></path></g></g><line x1="18" x2="23" y1="10" y2="15"></line><line x1="23" x2="18" y1="10" y2="15"></line></svg>;
let nfc_card_add_symbol = <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-user-x mr-2"><g transform="matrix(1.33 0 0 -1.33 -51.7 105) translate(0 0)"><g transform="matrix(.0118 0 0 .0118 39.3 61.8)" fill="currentColor"><path d="m5.51 1345-2.79-149c-.457-23.2-5.91-570 .043-726 9.85-253 24.2-393 122-470h227c-10.9 2.55-21.6 5.48-32.4 9.28-150 52.7-176 187-194 474-2.98 49.9-4.48 145-4.48 250 0 108 1.14 225 2.33 315l456-456v179l-573 573"></path><path d="m798 55.1 2.79 149c.461 23.2 5.91 570-.046 725-9.84 252-24.2 393-122 470h-227c10.9-2.55 21.7-5.48 32.4-9.27 150-52.7 176-187 194-474 2.98-49.9 4.49-145 4.49-250 0-108-1.15-225-2.34-315l-456 456v-179l573-573"></path></g></g><line x1="20" y1="9" x2="20" y2="15"></line><line x1="23" y1="12" x2="17" y2="12"></line></svg>;


type NfcConfig = API.getType['nfc/config'];
interface NfcState {
    userCfg: API.getType['users/config'];
    seen_tags: Readonly<API.getType['nfc/seen_tags']>;
    showModal: boolean
    newTag: NfcConfig['authorized_tags'][0]
}

export class Nfc extends ConfigComponent<'nfc/config', {}, NfcState> {
    constructor() {
        super('nfc/config',
              __("nfc.script.save_failed"),
              __("nfc.script.reboot_content_changed"));

        util.addApiEventListener('users/config', () => {
            this.setState({userCfg: API.get('users/config')});
        });

        util.addApiEventListener('nfc/seen_tags', () => {
            this.setState({seen_tags: API.get('nfc/seen_tags')});
        });

        this.state = {
            newTag: {
                tag_id: "",
                user_id: 0,
                tag_type: "disabled" as any}
        } as any;
    }

    setTag (i: number, val: Partial<NfcConfig['authorized_tags'][0]>){
        let tags = this.state.authorized_tags;
        tags[i] = {...tags[i], ...val};
        this.setState({authorized_tags: tags});
    }

    hackToAllowSave() {
        document.getElementById("nfc_config_form").dispatchEvent(new Event('input'));
    }

    render(props: {}, state: NfcConfig & NfcState) {
        if (!util.allow_render || !API.get("info/modules").nfc)
            return <></>

        type NFCSeenTag = API.getType['nfc/seen_tags'][0];

        let unauth_seen_tags: NFCSeenTag[] = [];

        let auth_seen_tags: NFCSeenTag[] = [];
        let auth_seen_ids: number[] = [];

        outer_loop:
            for(let i = 0; i < state.seen_tags.length; ++i) {
                if (state.seen_tags[i].tag_id == "")
                    continue;

                for (let auth_tag_idx = 0; auth_tag_idx < state.authorized_tags.length; ++auth_tag_idx) {
                    let auth_tag = state.authorized_tags[auth_tag_idx];
                    if (auth_tag.tag_type != state.seen_tags[i].tag_type)
                        continue;

                    if (auth_tag.tag_id != state.seen_tags[i].tag_id)
                        continue;

                    auth_seen_tags.push(state.seen_tags[i]);
                    auth_seen_ids.push(auth_tag_idx);

                    continue outer_loop;
                }

                unauth_seen_tags.push(state.seen_tags[i]);
            }

        let addTagCard = <div class="col mb-4">
                <Card className="h-100" key={999}>
                <div class="card-header d-flex justify-content-between align-items-center">
                    {nfc_card_add_symbol}
                    <Button variant="outline-dark" size="sm" style="visibility: hidden;">
                        {nfc_card_delete_symbol}<span style="font-size: 1rem; vertical-align: middle;">{__("nfc.script.delete")}</span>
                    </Button>
                </div>
                <Card.Body>
                    {state.authorized_tags.length >= MAX_AUTHORIZED_TAGS
                        ? <span>{__("nfc.script.add_tag_disabled_prefix") + MAX_AUTHORIZED_TAGS + __("nfc.script.add_tag_disabled_suffix")}</span>
                        : <Button variant="light" size="lg" block style="height: 100%;" onClick={() => this.setState({showModal: true})}>{__("nfc.script.add_tag")}</Button>}
                </Card.Body>
            </Card>
        </div>

        return (
            <>
                <ConfigForm id="nfc_config_form" title={__("nfc.content.nfc")} isModified={this.isModified()} onSave={() => this.save()} onReset={this.reset} onDirtyChange={(d) => this.ignore_updates = d}>
                    <FormRow label={__("nfc.content.authorized_tags")}>
                        <div class="row row-cols-1 row-cols-md-2">
                        {state.authorized_tags.map((tag, i) => (
                            <div class="col mb-4">
                            <Card className="h-100" key={tag.tag_id}>
                                <div class="card-header d-flex justify-content-between align-items-center">
                                    {nfc_card_symbol}
                                    <Button variant="outline-dark" size="sm"
                                            onClick={() => {
                                                this.setState({authorized_tags: state.authorized_tags.filter((v, idx) => idx != i)});
                                                this.hackToAllowSave();} }>
                                        {nfc_card_delete_symbol}<span style="font-size: 1rem; vertical-align: middle;">{__("nfc.script.delete")}</span>
                                    </Button>
                                </div>
                                <Card.Body>
                                    <FormGroup label={__("nfc.script.tag_id")}>
                                        <InputText value={tag.tag_id}
                                                   onValue={(v) => this.setTag(i, {tag_id: v})}
                                                   minLength={8} maxLength={29}
                                                   pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                                                   invalidFeedback={__("nfc.content.tag_id_invalid_feedback")}
                                                   required/>
                                    </FormGroup>
                                    <FormGroup label={__("nfc.script.user_id")}>
                                        <InputSelect items={state.userCfg.users.map((u, j) => [u.id.toString(), j == 0 ? __("nfc.script.not_assigned") : u.display_name.toString()])}
                                            value={tag.user_id.toString()}
                                            onValue={(v) => this.setTag(i, {user_id: parseInt(v)})}
                                            />
                                        {tag.user_id == 0 ? <div class="mt-2">{__("nfc.script.not_assigned_desc")}</div> : undefined}
                                    </FormGroup>
                                    <FormGroup label={__("nfc.script.tag_type")}>
                                        <InputSelect items={[
                                                ["0",__("nfc.content.type_0")],
                                                ["1",__("nfc.content.type_1")],
                                                ["2",__("nfc.content.type_2")],
                                                ["3",__("nfc.content.type_3")],
                                                ["4",__("nfc.content.type_4")],
                                            ]}
                                            value={tag.tag_type.toString()}
                                            onValue={(v) => this.setTag(i, {tag_type: parseInt(v)})}
                                            />
                                    </FormGroup>
                                </Card.Body>
                                <Card.Footer>
                                    {
                                        <span>{auth_seen_ids.indexOf(i) >= 0 ? __("nfc.content.last_seen") + util.format_timespan(Math.floor(auth_seen_tags[auth_seen_ids.indexOf(i)].last_seen / 1000)) + __("nfc.content.last_seen_suffix") : __("nfc.script.not_seen")}</span>
                                    }
                                </Card.Footer>
                            </Card>
                            </div>
                        )).concat(addTagCard)}
                        </div>
                    </FormRow>
                </ConfigForm>

                <ItemModal show={state.showModal}
                    onHide={() => this.setState({showModal: false})}
                    onSubmit={() => {
                        if (state.newTag.tag_id === "" && unauth_seen_tags.length == 1)
                        {
                            state.newTag.tag_id = unauth_seen_tags[0].tag_id;
                            state.newTag.tag_type = unauth_seen_tags[0].tag_type;
                        }
                        this.setState({showModal: false,
                                       authorized_tags: state.authorized_tags.concat(state.newTag),
                                       newTag: {tag_id: "", user_id: 0, tag_type: "disabled" as any}});
                        this.hackToAllowSave();}}
                    title={__("nfc.content.add_tag_modal_title")}
                    no_text={__("nfc.content.add_tag_modal_abort")}
                    no_variant={"secondary"}
                    yes_text={__("nfc.content.add_tag_modal_save")}
                    yes_variant={"primary"}>
                        <FormGroup label={__("nfc.content.add_tag_modal_user_id")}>
                            <InputSelect items={state.userCfg.users.map((u, j) => [u.id.toString(), j == 0 ? __("nfc.script.not_assigned") : u.display_name.toString()])}
                                         value={state.newTag.user_id.toString()}
                                         onValue={(v) => this.setState({newTag: {...state.newTag, user_id: parseInt(v)}})}
                                         required
                                         />
                        </FormGroup>
                        <FormGroup label={__("nfc.content.add_tag_modal_seen_tags")}>
                            {unauth_seen_tags.length == 0
                                ? <span>{__("nfc.content.add_tag_description")}</span>
                                : <ListGroup>{
                                    unauth_seen_tags.map(t => <ListGroupItem action type="button" onClick={() => this.setState({newTag: {...state.newTag, tag_id: t.tag_id, tag_type: t.tag_type}})}>
                                        <h5 class="mb-1 pr-2">{t.tag_id}</h5>
                                        <div class="d-flex w-100 justify-content-between">
                                            <span>{translate_unchecked(`nfc.content.type_${t.tag_type}`)}</span>
                                            <span>{__("nfc.content.last_seen") + util.format_timespan(Math.floor(t.last_seen / 1000)) + __("nfc.content.last_seen_suffix")}</span>
                                        </div>
                                        </ListGroupItem>)
                                 }</ListGroup>
                            }
                        </FormGroup>
                        <FormGroup label={__("nfc.content.add_tag_modal_tag_id")}>
                            <InputText value={state.newTag.tag_id}
                                       onValue={(v) => this.setState({newTag: {...state.newTag, tag_id: v}})}
                                       minLength={8} maxLength={29}
                                       pattern="^([0-9a-fA-F]{2}:?){3,9}[0-9a-fA-F]{2}$"
                                       invalidFeedback={__("nfc.content.tag_id_invalid_feedback")}
                                       required/>
                        </FormGroup>
                        <FormGroup label={__("nfc.content.add_tag_modal_tag_type")}>
                            <InputSelect items={[
                                    ["disabled", __("nfc.content.select_type")],
                                    ["0",__("nfc.content.type_0")],
                                    ["1",__("nfc.content.type_1")],
                                    ["2",__("nfc.content.type_2")],
                                    ["3",__("nfc.content.type_3")],
                                    ["4",__("nfc.content.type_4")],
                                ]}
                                value={state.newTag.tag_type.toString()}
                                onValue={(v) => this.setState({newTag: {...state.newTag, tag_type: parseInt(v)}})}
                                required
                                />
                        </FormGroup>
                </ItemModal>
            </>
        )
    }
}

render(<Nfc/>, $('#nfc')[0]);

export function init() {

}

export function add_event_listeners(source: API.APIEventTarget) {
}

export function update_sidebar_state(module_init: any) {
    $('#sidebar-nfc').prop('hidden', !module_init.nfc);
}
