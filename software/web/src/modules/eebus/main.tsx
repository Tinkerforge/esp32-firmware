/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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


import * as API from "../../ts/api";
import * as util from "../../ts/util";
import { h, Fragment, Component, RefObject } from "preact";
import { __ } from "../../ts/translation";
import { ConfigComponent } from "../../ts/components/config_component";
import { ConfigForm } from "../../ts/components/config_form";
import { FormRow } from "../../ts/components/form_row";
import { InputSelect } from "../../ts/components/input_select";
import { InputText } from "../../ts/components/input_text";
import { SubPage } from "../../ts/components/sub_page";
import { NavbarItem } from "../../ts/components/navbar_item";
import { Table } from "../../ts/components/table";
import { Collapse, Button, Spinner, ListGroup, ListGroupItem, Alert, Form } from "react-bootstrap";
import { Share2 } from "react-feather";

export function EEBusNavbar() {
    return <NavbarItem name="eebus" module="eebus" title="EEBUS" symbol={<Share2 />} />;
}

type EEBusConfig = API.getType["eebus/config"];
type EEBusScan = API.getType["eebus/scan"];
type EEBusAddPeer = API.getType["eebus/addPeer"];
type EEBusStateType = API.getType["eebus/state"];

interface EEBusState {
    addPeer: EEBusAddPeer;
    state: EEBusStateType;
}

export class EEBus extends ConfigComponent<'eebus/config', {}, EEBusState> {
    constructor() {
        super('eebus/config',
            // TODO: Remove mqtt dependencies
            () => __("eebus.script.save_failed"),
            () => __("eebus.script.reboot_content_changed"));
    }
    scan_peers() {
        API.call('eebus/scan', {});
    }


    render(props: {}, state: Readonly<EEBusConfig> & EEBusState) {
        if (!util.render_allowed())
            return <SubPage name="eebus" />;

        let ski = API.get('eebus/state').ski
        if (ski == "") {
            ski = __("eebus.content.unknown");
        }

        let cert_state = API.get_unchecked('certs/state');
        let certs = cert_state == null ? [] : cert_state.certs.map((c: any) => [c.id.toString(), c.name]) as [string, string][];

        return (
            <SubPage name="eebus">
                <ConfigForm id="eebus_config_form" title="EEBUS" isModified={this.isModified()} isDirty={this.isDirty()} onSave={this.save} onReset={this.reset} onDirtyChange={this.setDirty}>
                    <FormRow label={__("eebus.content.cert")}>
                        <InputSelect items={[
                            ["-1", __("eebus.content.no_cert")],
                        ].concat(certs) as [string, string][]
                        }
                            value={state.cert_id}
                            onValue={(v) => this.setState({ cert_id: parseInt(v) })}
                            disabled={cert_state == null}
                            required={state.key_id != -1}
                        />
                    </FormRow>
                    <FormRow label={__("eebus.content.key")}>
                        <InputSelect items={[
                            ["-1", __("eebus.content.no_cert")],
                        ].concat(certs) as [string, string][]
                        }
                            value={state.key_id}
                            onValue={(v) => this.setState({ key_id: parseInt(v) })}
                            disabled={cert_state == null}
                            required={state.cert_id != -1}
                        />
                    </FormRow>
                    <FormRow label={__("eebus.content.ski")}>
                        <InputText value={ski} />
                    </FormRow>
                    <FormRow label={__("eebus.content.peers")}>
                        <Table
                            columnNames={[
                                __("eebus.content.model_model"),
                                __("eebus.content.model_brand"),
                                __("eebus.content.device_ip"),
                                __("eebus.content.device_trusted")]}
                            rows={
                                state.peers.map((peer) => {
                                    return {
                                        columnValues: [
                                            peer.model_model,
                                            peer.model_brand,
                                            peer.ip,
                                            peer.trusted ? __("eebus.content.trusted_yes") : __("eebus.content.trusted_no")
                                        ],
                                        fieldValues: [
                                            peer.model_model,
                                            peer.model_brand,
                                            peer.ip,
                                            peer.trusted
                                        ],
                                        editTitle: __("eebus.content.edit_peer_title"),
                                        onEditShow: async () => this.setState({ addPeer: { ski: peer.ski, trusted: peer.trusted, ip: peer.ip, port: peer.port, dns_name: peer.dns_name, wss_path: peer.wss_path } }),
                                        onEditGetChildren: () => [
                                            <>
                                                <FormRow label={__("eebus.content.ski")}>
                                                    <InputText
                                                        value={state.addPeer.ski}
                                                    />
                                                </FormRow>
                                                <FormRow label={__("eebus.content.device_trusted")}>
                                                    <InputSelect items={[
                                                        ["0", __("eebus.content.trusted_no")],
                                                        ["1", __("eebus.content.trusted_yes")]
                                                    ]} value={state.addPeer.trusted ? "1" : "0"} onValue={(v) => this.setState({ addPeer: { ...state.addPeer, trusted: v == "1" } })} />
                                                </FormRow>
                                            </>
                                        ]

                                    }
                                })
                            }
                            addEnabled={true}
                            addTitle={__("eebus.content.add_peers")}
                            addMessage={__("eebus.content.add_peers")}
                            onAddShow={async () => { }}
                            onAddGetChildren={() => [<>
                                <FormRow label={__("eebus.content.ski")}>
                                    <InputText value={""} />
                                </FormRow>
                                <FormRow label={__("eebus.content.device_trusted")}>
                                    <InputSelect items={[
                                        ["0", __("eebus.content.trusted_no")],
                                        ["1", __("eebus.content.trusted_yes")]
                                    ]} value={""} onValue={() => { }} />
                                </FormRow>
                            </>]}
                        />
                        <Button className="form-control rounded-right" variant="primary" onClick={() => this.scan_peers()}>{__("eebus.content.search_peers")}</Button>
                    </FormRow>
                </ConfigForm>
            </SubPage>
        );
    }
}

export function init() {
}
