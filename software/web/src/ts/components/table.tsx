/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

import { __ } from "../translation";
import { h, Component, Fragment, ComponentChild, ComponentChildren } from "preact";
import { Card, Button, Collapse } from "react-bootstrap";
import { Plus, Edit3, Trash2, ChevronRight } from "react-feather";
import { FormRow } from "./form_row";
import { ItemModal } from "./item_modal";
import * as util from "../../ts/util";

export interface TableRow {
    key?: string;
    columnValues: ComponentChild[];
    extraFieldName?: string;
    extraValue?: ComponentChild;
    extraKey?: string;
    fieldNames?: string[];
    fieldValues?: ComponentChild[];
    fieldWithBox?: boolean[];
    editTitle?: string;
    onEditShow?: () => Promise<void>;
    onEditGetChildren?: () => ComponentChildren;
    onEditCheck?: () => Promise<boolean>;
    onEditSubmit?: () => Promise<void>;
    onEditHide?: () => Promise<void>;
    onRemoveClick?: () => Promise<void>;
    onEditStart?: never;
    onEditCommit?: never;
    onEditAbort?: never;
}

export interface TableProps {
    columnNames: string[];
    rows: TableRow[];
    addEnabled?: boolean;
    addMessage?: string;
    addTitle?: string;
    onAddShow?: () => Promise<void>;
    onAddGetChildren?: () => ComponentChildren;
    onAddCheck?: () => Promise<boolean>;
    onAddSubmit?: () => Promise<void>;
    onAddHide?: () => Promise<void>;
    tableTill?: "xs" | "sm" | "md" | "lg" | "xl";
    nestingDepth?: number;
    onAddStart?: never;
    onAddCommit?: never;
    onAddAbort?: never;
}

interface TableState {
    showAddModal: boolean;
    showEditModal: number;
    showRowExtra: boolean[/*row_index*/];
}

const value_or_else = (value: ComponentChild, replacement: ComponentChild): ComponentChild => {
    if (util.hasValue(value)) {
        return value;
    }

    return replacement;
};

export class Table extends Component<TableProps, TableState> {
    constructor() {
        super();

        this.state = {
            showAddModal: false,
            showEditModal: null,
            showRowExtra: [],
        } as any;
    }

    get_card_fields(row: TableRow) {
        let names = (util.hasValue(row.fieldNames) ? row.fieldNames : this.props.columnNames).slice(1);
        let values = (util.hasValue(row.fieldValues) ? row.fieldValues : row.columnValues).slice(1);

        while (values.length > 0 && !util.hasValue(values[values.length - 1])) {
            names.pop();
            values.pop();
        }

        return names.map((name, i) => name ?
            <FormRow label={name}>
                {!util.hasValue(row.fieldWithBox) || row.fieldWithBox[i + 1] ?
                    <span class="form-control" style="height: unset;" readonly>{value_or_else(values[i], <>&nbsp;</>)}</span>
                    : <>{values[i]}</>}
            </FormRow>
            : (value_or_else(values[i], undefined)));
    }

    render(props: TableProps, state: TableState) {
        return (
            <>
                <Card className={`d-none d-${props.tableTill ? props.tableTill : 'sm'}-block`}><Card.Body style="padding: 0;">
                <table class="table" style="font-size: 1rem; margin-bottom: 0;">
                    {props.columnNames.filter((name) => name.length > 0).length > 0 ?
                        <thead>
                            <tr>
                                {props.columnNames.map((columnName) => (
                                    <th scope="col" style="vertical-align: middle;">{columnName}</th>
                                ))}
                                <th scope="col">
                                    <Button size="sm" disabled={true} style="visibility: hidden;">
                                        <Trash2/>
                                    </Button>
                                </th>
                            </tr>
                        </thead>
                        : undefined}
                    <tbody>
                        {props.rows.map((row, i) => <>
                            <tr key={row.key}>
                                {row.columnValues.map((value, k) => (
                                    <td class={row.extraValue ? "pb-0" : ""} style={"word-wrap: break-word; vertical-align: middle;" + (i == 0 ? " border-top: none;" : "")}>
                                        {row.extraValue && k == 0 ?
                                            <span class="row mx-n1 align-items-center"><span class="col-auto px-1"><Button className="mr-2" size="sm"
                                                onClick={() => {
                                                    let showRowExtra = state.showRowExtra.concat();
                                                    showRowExtra[i] = !showRowExtra[i];
                                                    this.setState({showRowExtra: showRowExtra});
                                                }}>
                                                <ChevronRight {...{class: state.showRowExtra[i] ? "rotated-chevron" : "unrotated-chevron"} as any}/>
                                                </Button></span><span class="col px-1">{value}</span></span> : value}
                                    </td>
                                ))}
                                <td class={row.extraValue ? "pb-0" : undefined} style={"width: 1%; white-space: nowrap; vertical-align: middle;" + (i == 0 ? " border-top: none;" : "")}>
                                    <Button variant="primary"
                                            size="sm"
                                            className="mr-2"
                                            onClick={async () => {
                                                await row.onEditShow();
                                                this.setState({showEditModal: i});
                                            }}
                                            disabled={!row.onEditShow}>
                                        <Edit3/>
                                    </Button>
                                    <Button variant="danger"
                                            size="sm"
                                            onClick={row.onRemoveClick}
                                            disabled={!row.onRemoveClick}>
                                        <Trash2/>
                                    </Button>
                                </td>
                            </tr>
                            {row.extraValue ?
                                <tr key={row.extraKey} class="table-extra-value">
                                    <td colSpan={props.columnNames.length + 1} class="pt-0" style="border-top: none;">
                                        <Collapse in={state.showRowExtra[i]}>
                                            <div>
                                                <Card style="margin-top: 0.75rem;"><Card.Body className="p-2d5 pb-0">{row.extraValue}</Card.Body></Card>
                                            </div>
                                        </Collapse>
                                    </td>
                                </tr>
                                : undefined}
                        </>)}
                        {props.onAddShow ?
                        <tr>
                            <td colSpan={props.columnNames.length} style={"vertical-align: middle; width: 100%;" + (props.rows.length == 0 ? " border-top: none;" : "")}>
                                {props.addMessage}
                            </td>
                            <td style={"text-align: right; vertical-align: middle;" + (props.rows.length == 0 ? " border-top: none;" : "")}>
                                <Button variant="primary"
                                        size="sm"
                                        onClick={async () => {
                                            await props.onAddShow();
                                            this.setState({showAddModal: true});
                                        }}
                                        disabled={!props.addEnabled}>
                                    <Plus/>
                                </Button>
                            </td>
                        </tr>
                        : undefined}
                    </tbody>
                </table>
                </Card.Body></Card>

                <div class={`d-block d-${props.tableTill ? props.tableTill : 'sm'}-none table-card-mode`}>
                    {props.rows.map((row, i) => {
                        let card_fields = this.get_card_fields(row);
                        let needs_body = card_fields.length > 0 || (row.extraValue && state.showRowExtra[i]);

                        return <><Card className="mb-3">
                        <div class="card-header d-flex justify-content-between align-items-center p-2d5" style={needs_body ? "" : "border-bottom: 0;"}>
                            <h5 class="text-break" style="margin-bottom: 0;">
                                {row.extraValue ?
                                    <span class="row mx-n1 align-items-center">
                                        <span class="col-auto px-1"><Button className="mr-2" size="sm"
                                            onClick={() => {
                                                let showRowExtra = state.showRowExtra.concat();
                                                showRowExtra[i] = !showRowExtra[i];
                                                this.setState({showRowExtra: showRowExtra});
                                            }}>
                                            <ChevronRight {...{class: state.showRowExtra[i] ? "rotated-chevron" : "unrotated-chevron"} as any}/>
                                            </Button>
                                        </span>
                                        <span class="col px-1">{util.hasValue(row.fieldValues) ? row.fieldValues[0] : row.columnValues[0]}</span>
                                    </span>
                                    : util.hasValue(row.fieldValues) ? row.fieldValues[0] : row.columnValues[0]}
                            </h5>
                            <div style="white-space: nowrap; vertical-align: middle;">
                                <Button variant="primary"
                                        size="sm"
                                        className="ml-2"
                                        onClick={async () => {
                                            await row.onEditShow();
                                            this.setState({showEditModal: i});
                                        }}
                                        disabled={!row.onEditShow}>
                                    <Edit3/>
                                </Button>
                                <Button variant="danger"
                                        size="sm"
                                        className="ml-2"
                                        onClick={row.onRemoveClick}
                                        disabled={!row.onRemoveClick}>
                                    <Trash2/>
                                </Button>
                            </div>
                        </div>
                        {needs_body ?
                            <Card.Body className="p-2d5 pb-0">
                                {card_fields}
                                {row.extraValue ?
                                    <Collapse in={state.showRowExtra[i]}>
                                        <div>
                                            {row.extraFieldName ?
                                                <FormRow label={row.extraFieldName}>
                                                    <Card><Card.Body className="p-2d5 pb-0">{row.extraValue}</Card.Body></Card>
                                                </FormRow>
                                                : <Card><Card.Body className="p-2d5 pb-0">{row.extraValue}</Card.Body></Card>}
                                        </div>
                                    </Collapse>
                                    : undefined}
                            </Card.Body>
                            : undefined}
                        </Card></>
                    })}
                    {props.onAddShow ?
                    <Card className="mb-0">
                        <div class="card-body d-flex justify-content-between align-items-center p-2d5">
                            <span class="text-break" style="font-size: 1rem;">{props.addMessage}</span>
                            <div style="white-space: nowrap; vertical-align: middle;">
                            <Button variant="primary"
                                    size="sm"
                                    className="ml-2"
                                    onClick={async () => {
                                        await props.onAddShow();
                                        this.setState({showAddModal: true});
                                    }}
                                    disabled={!props.addEnabled}>
                                <Plus/>
                            </Button>
                            </div>
                        </div>
                    </Card>
                    : undefined}
                </div>

                <ItemModal
                    onCheck={async () => {
                        if (props.onAddCheck) {
                            return await props.onAddCheck();
                        }

                        return true;
                    }}
                    onSubmit={async () => {
                        if (props.onAddSubmit) {
                            await props.onAddSubmit();
                        }
                    }}
                    onHide={async () => {
                        this.setState({showAddModal: false}, async () => {
                            if (props.onAddHide) {
                                await props.onAddHide();
                            }
                        });
                    }}
                    show={state.showAddModal}
                    no_variant="secondary"
                    yes_variant="primary"
                    title={props.addTitle}
                    no_text={__("component.table.abort")}
                    yes_text={__("component.table.add")}
                    backdropClassName={props.nestingDepth === undefined ? undefined : ("modal-backdrop-" + props.nestingDepth)}
                    className={props.nestingDepth === undefined ? undefined : ("modal-" + props.nestingDepth)}
                    size={props.nestingDepth === undefined ? "xl" : {0: "xl", 1: "lg", 2: "md", 3: "sm"}[props.nestingDepth] as 'xl' | 'lg' | 'sm'} >{/* "md" doesn't exist but is just the normal size, the cast make it ignore "md"*/}
                    {state.showAddModal && props.onAddGetChildren ?
                        props.onAddGetChildren()
                        : undefined}
                </ItemModal>

                <ItemModal
                    onCheck={async () => {
                        if (props.rows[state.showEditModal].onEditCheck) {
                            return await props.rows[state.showEditModal].onEditCheck();
                        }

                        return true;
                    }}
                    onSubmit={async () => {
                        if (props.rows[state.showEditModal].onEditSubmit) {
                            await props.rows[state.showEditModal].onEditSubmit();
                        }
                    }}
                    onHide={async () => {
                        this.setState({showEditModal: null}, async () => {
                            if (props.rows[state.showEditModal].onEditHide) {
                                await props.rows[state.showEditModal].onEditHide();
                            }
                        });
                    }}
                    show={state.showEditModal !== null}
                    no_variant="secondary"
                    yes_variant="primary"
                    title={state.showEditModal !== null ? props.rows[state.showEditModal].editTitle : ''}
                    no_text={__("component.table.abort")}
                    yes_text={__("component.table.apply")}
                    backdropClassName={props.nestingDepth === undefined ? undefined : ("modal-backdrop-" + props.nestingDepth)}
                    className={props.nestingDepth === undefined ? undefined : ("modal-" + props.nestingDepth)}
                    size={props.nestingDepth === undefined ? "xl" : {0: "xl", 1: "lg", 2: "md", 3: "sm"}[props.nestingDepth] as 'xl' | 'lg' | 'sm'} >{/* "md" doesn't exist but is just the normal size, the cast make it ignore "md"*/}
                    {state.showEditModal !== null && props.rows[state.showEditModal].onEditGetChildren ?
                        props.rows[state.showEditModal].onEditGetChildren()
                        : undefined}
                </ItemModal>
            </>
        );
    }
}
