/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

import { h, Component, Fragment } from "preact";
import { JSX } from 'preact';
import { Collapse } from "react-bootstrap";
import { __ } from "../translation";
import { FormRow } from "./form_row";
import { InputIP6 } from "./input_ip6";
import { Switch } from "./switch";
import { Table } from "./table";

import { InputSelect } from "./input_select";

export interface ipv6_address {
    addr: string;
    flags: number;
}

export interface IP6Config {
    enable_ipv6: boolean;
    ip6: ipv6_address[];
}

interface IP6EditorState {
    edit_addr: ipv6_address
    edit_addr_is_multicast: boolean
    contains_duplicate_address: boolean
}

interface IP6ConfigurationProps extends Omit<JSX.InputHTMLAttributes<HTMLInputElement>, "class" | "id" | "type" | "onInput" | "value" | "disabled"> {
    value: Readonly<IP6Config>
    onValue: (value: IP6Config) => void
    setDirty: (dirty: boolean) => void
    setValid: (valid: boolean) => void

    maxAddresses: number
    allowMulticast?: boolean

    // forbidNetwork?: {ip: number, subnet: number, name: string}[]
}

export class IP6Configuration extends Component<IP6ConfigurationProps, IP6EditorState> {
    onUpdate<T extends keyof IP6Config>(k: T, v: IP6Config[T]) {
        if (JSON.stringify(this.props.value[k]) === JSON.stringify(v)) {
            return;
        }

        const new_values = {...this.props.value, [k]: v};
        this.props.onValue(new_values);
        this.props.setDirty(true);

        if (k == "ip6") {
            const addresses_only = new_values.ip6.map((entry, i) => entry.addr);
            const contains_duplicate_address = new Set(addresses_only).size !== addresses_only.length;

            this.setState({contains_duplicate_address: contains_duplicate_address});
            this.props.setValid(!contains_duplicate_address);
        }
    }

    onEditUpdate<T extends keyof ipv6_address>(k: T, v: ipv6_address[T]) {
        this.setState({edit_addr: {...this.state.edit_addr, [k]: v}});
    }

    onEditGetChildren() {
        let is_invalid;
        let invalid_feedback;

        if (this.state.edit_addr_is_multicast) {
            is_invalid = true;
            invalid_feedback = __("component.ip6_configuration.ip6_address_is_multicast");
        } else {
            is_invalid = false;
            invalid_feedback = __("component.ip6_configuration.ip6_address_invalid");
        }

        return [
            <FormRow label={__("component.ip6_configuration.ip6_address")}>
                <InputIP6
                    invalidFeedback={invalid_feedback}
                    moreClasses={is_invalid ? ["is-invalid"] : undefined}
                    required
                    value={this.state.edit_addr.addr}
                    onValue={(v) => {
                        this.onEditUpdate("addr", v);

                        if (is_invalid) {
                            this.onEditCheck(v);
                        }
                    }}
                />
            </FormRow>,
            <FormRow label={__("component.ip6_configuration.preferred")}>
                <Switch desc={__("component.ip6_configuration.preferred_desc")}
                        checked={(this.state.edit_addr.flags & 0x8000) != 0}
                        onClick={() => this.onEditUpdate("flags", this.state.edit_addr.flags ^ 0x8000)}
                />
            </FormRow>,
        ];
    }

    onEditCheck(addr: string) {
        let is_multicast = false;

        if (!this.props.allowMulticast) {
            const colon_at = addr.indexOf(':');

            if (colon_at > 0 && parseInt(addr.substring(0, colon_at), 16) >= 0xff00) {
                is_multicast = true;
            }
        }

        // TODO Check against ::/96 and forbidNetwork.

        return new Promise<boolean>((resolve) => {
            this.setState({edit_addr_is_multicast: is_multicast}, () => resolve(!is_multicast));
        });
    }

    render(props: IP6ConfigurationProps, state: IP6Config & IP6EditorState) {
        return <>
            <FormRow label={__("component.ip6_configuration.enable_ipv6")}>
                <Switch
                    checked={props.value.enable_ipv6}
                    onClick={() => this.onUpdate("enable_ipv6", !props.value.enable_ipv6)}
                />
            </FormRow>

            <Collapse in={props.value.enable_ipv6}>
                <div>
                    <FormRow label={__("component.ip_configuration.ip_configuration")} help={__("component.ip6_configuration.ip_configuration_help")}>
                        <InputSelect
                            value={"slaac"}
                            items={[
                                ["slaac", "SLAAC"],
                            ]}
                        />
                    </FormRow>

                    <FormRow label={__("component.ip6_configuration.static_ip6")}>
                        <Table nestingDepth={1} // We are not nested, but this also reduces the modal's size to lg
                            invalid={state.contains_duplicate_address}
                            invalidFeedback={__("component.ip6_configuration.static_ip6_duplicates")}
                            columnNames={[__("component.ip6_configuration.ip6_address"), __("component.ip6_configuration.preferred")]}
                            rows={props.value.ip6.map((entry, i) => {
                                return {
                                    columnValues: [
                                        entry.addr,
                                        (entry.flags & 0x8000) != 0 ? "✔" : null,
                                    ],
                                    editTitle: __("component.ip6_configuration.static_ip6_edit_title"),
                                    onEditShow: async () => this.setState({edit_addr: entry, edit_addr_is_multicast: false}),
                                    onEditGetChildren: () => this.onEditGetChildren(),
                                    onEditCheck: async () => this.onEditCheck(state.edit_addr.addr),
                                    onEditSubmit: async () => {
                                        this.onUpdate("ip6", props.value.ip6.map((entry, k) => i === k ? state.edit_addr : entry));
                                    },
                                    onRemoveClick: async () => {
                                        this.onUpdate("ip6", props.value.ip6.filter((v, idx) => idx != i));
                                        return true;
                                    }
                                }})
                            }
                            addEnabled={props.value.ip6.length < props.maxAddresses}
                            addTitle={__("component.ip6_configuration.static_ip6_add_title")}
                            addMessage={__("component.ip6_configuration.static_ip6_add_message")(props.value.ip6.length, props.maxAddresses)}
                            onAddShow={async () => this.setState({edit_addr: {addr: "", flags: 0}, edit_addr_is_multicast: false})}
                            onAddGetChildren={() => this.onEditGetChildren()}
                            onAddCheck={async () => this.onEditCheck(state.edit_addr.addr)}
                            onAddSubmit={async () => {
                                this.onUpdate("ip6", props.value.ip6.concat(state.edit_addr));
                            }}
                            onAddHide={async () => {}}
                        />
                    </FormRow>
                </div>
            </Collapse>
        </>;
    }
}
