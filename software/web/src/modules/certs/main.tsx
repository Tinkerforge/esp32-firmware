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
import { h, Fragment, Component } from "preact";
import { __ } from "../../ts/translation";
import { InputText } from "../../ts/components/input_text";
import { SubPage } from "../../ts/components/sub_page";
import { PageHeader } from "../../ts/components/page_header";
import { Table } from "../../ts/components/table";
import { FormRow } from "../../ts/components/form_row";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Award } from "react-feather";

export function CertsNavbar() {
    return <NavbarItem name="certs" module="certs" title={__("certs.navbar.certs")} symbol={<Award />} />;
}

interface State {
    editCert: API.getType['certs/add'] & {'file': File, file_too_large: boolean}
    addCert: Omit<API.getType['certs/add'], 'id'> & {file: File, file_too_large: boolean}
}

const MAX_CERTS = 8;
const MAX_CERT_SIZE = 4027;

export class Certs extends Component<{}, State> {
    render(props: {}, state: Readonly<State>) {
        if (!util.render_allowed())
            return <SubPage name="certs" />;

        return (
            <SubPage name="certs">
                <PageHeader title={__("certs.content.certs")}/>
                    <div class="mb-3">
                        <Table
                            tableTill="md"
                            columnNames={[]}
                            rows={API.get('certs/state').certs.map((cert, i) =>
                                { return {
                                    key: cert.id.toString(),
                                    columnValues: [
                                        [cert.name]
                                    ],
                                    editTitle: __("certs.content.edit_cert_title"),
                                    onEditShow: async () => this.setState({editCert: {id: cert.id, name: cert.name, cert: "", file: null, file_too_large: false}}),
                                    onEditGetChildren: () => [<>
                                        <FormRow label={__("certs.content.cert_name")}>
                                            <InputText value={state.editCert.name}
                                                        onValue={(v) => this.setState({editCert: {...state.editCert, name: v}})}
                                                        maxLength={32}
                                                        required/>
                                        </FormRow>
                                        <FormRow label={__("certs.content.cert_file")} label_muted={__("certs.content.cert_file_muted")}>
                                            <div class="custom-file">
                                                <input type="file"
                                                        class={"custom-file-input form-control" + (this.state.editCert.file_too_large ? " is-invalid" : "")}
                                                        accept={"application/pem-certificate-chain"}
                                                    onChange={(ev) => {
                                                        let file = (ev.target as HTMLInputElement).files[0];
                                                        this.setState({editCert: {...state.editCert, file: file, file_too_large: file.size > MAX_CERT_SIZE}})
                                                    }}/>
                                                <label class="custom-file-label form-control rounded-left"
                                                    data-browse={__("certs.content.browse")}>{state.editCert.file ? state.editCert.file.name : __("certs.content.select_file")}</label>
                                                <div class="invalid-feedback">{__("certs.script.cert_too_large")(MAX_CERT_SIZE)}</div>
                                            </div>
                                        </FormRow>
                                    </>],
                                    onEditSubmit: async () => {
                                        await API.call('certs/modify', {
                                            id: state.editCert.id,
                                            name: state.editCert.name,
                                            cert: state.editCert.file == null ? null : await state.editCert.file.text()
                                        }, __("certs.script.mod_cert_failed"));
                                    },
                                    onRemoveClick: async () => { await API.call('certs/remove', {id: cert.id}, __("certs.script.del_cert_failed")); }
                                }})
                            }
                            addEnabled={API.get('certs/state').certs.length < MAX_CERTS}
                            addTitle={__("certs.content.add_cert_title")}
                            addMessage={__("certs.content.add_cert_message")(API.get('certs/state').certs.length, MAX_CERTS)}
                            onAddShow={async () => {
                                this.setState({addCert: {name: "", cert: "", file: null, file_too_large: false}});
                            }}
                            onAddGetChildren={() => [<>
                                <FormRow label={__("certs.content.cert_name")}>
                                    <InputText value={state.addCert.name}
                                        onValue={(v) => this.setState({addCert: {...state.addCert, name: v}})}
                                        maxLength={32}
                                        required/>
                                </FormRow>
                                <FormRow label={__("certs.content.cert_file")} label_muted={__("certs.content.cert_file_muted")}>
                                    <div class="custom-file">
                                        <input type="file"
                                                class={"custom-file-input form-control" + (this.state.addCert.file_too_large ? " is-invalid" : "")}
                                                accept="application/pem-certificate-chain"
                                            onChange={(ev) => {
                                                let file = (ev.target as HTMLInputElement).files[0];
                                                this.setState({addCert: {...state.addCert, file: file, file_too_large: file.size > MAX_CERT_SIZE}});
                                            }}/>
                                        <label class="custom-file-label form-control rounded-left"
                                            data-browse={__("certs.content.browse")}>{state.addCert.file ? state.addCert.file.name : __("certs.content.select_file")}</label>
                                        <div class="invalid-feedback">{__("certs.script.cert_too_large")(MAX_CERT_SIZE)}</div>
                                    </div>
                                </FormRow>
                            </>]}
                            onAddSubmit={async () => {
                                let ids = API.get('certs/state').certs.map(c => c.id);
                                let next_free_id = -1;
                                for(let i = 0; i < 8; ++i) {
                                    if (ids.indexOf(i) < 0) {
                                        next_free_id = i;
                                        break;
                                    }
                                }

                                await API.call('certs/add', {
                                    id: next_free_id,
                                    name: state.addCert.name,
                                    cert: await state.addCert.file.text()
                                }, __("certs.script.add_cert_failed"));
                            }}
                            />
                    </div>
            </SubPage>
        )
    }
}

export function init() {
}
