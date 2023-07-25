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
import { h, Component, Fragment, VNode, ComponentChild } from "preact";
import { Card, Button, Collapse } from "react-bootstrap";
import { Plus, Edit3, Trash2 } from "react-feather";
import { FormRow } from "./form_row";
import { ItemModal } from "./item_modal";
import * as util from "../../ts/util";

export interface TableModalRow {
    name: string
    // FormRow only accepts VNodes, not other ComponentChild variants.
    value: VNode
}

export interface TableRow {
    key?: string
    columnValues: ComponentChild[]
    extraShow?: boolean
    extraFieldName?: string
    extraValue?: ComponentChild
    extraKey?: string
    fieldNames?: string[]
    fieldValues?: ComponentChild[]
    fieldWithBox?: boolean[]
    editTitle?: string
    onEditStart?: () => Promise<void>
    onEditGetRows?: () => TableModalRow[]
    onEditCheck?: () => Promise<boolean>
    onEditCommit?: () => Promise<void>
    onEditHide?: () => Promise<void>
    onRemoveClick?: () => Promise<void>
}

export interface TableProps {
    columnNames: string[]
    rows: TableRow[]
    addEnabled?: boolean
    addMessage?: string
    addTitle?: string
    onAddStart?: () => Promise<void>
    onAddGetRows?: () => TableModalRow[]
    onAddCheck?: () => Promise<boolean>
    onAddCommit?: () => Promise<void>
    onAddHide?: () => Promise<void>
    tableTill?: 'xs' | 'sm' | 'md' | 'lg' | 'xl'
}

interface TableState {
    showAddModal: boolean
    showEditModal: number
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
            addRows: [],
            showEditModal: null,
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
                    <tbody>
                        {props.rows.map((row, i) => <>
                            <tr key={row.key}>
                                {row.columnValues.map((value) => (
                                    <td class={"text-break" + (row.extraValue ? " pb-0" : "")} style="vertical-align: middle;">{value}</td>
                                ))}
                                <td class={row.extraValue ? "pb-0" : undefined} style="width: 1%; white-space: nowrap; vertical-align: middle;">
                                    <Button variant="primary"
                                            size="sm"
                                            className="mr-2"
                                            onClick={async () => {
                                                await row.onEditStart();
                                                this.setState({showEditModal: i});
                                            }}
                                            disabled={!row.onEditStart}>
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
                                <tr key={row.extraKey}>
                                    <td colSpan={props.columnNames.length + 1} class="pt-0" style="border-top: none;">
                                        <Collapse in={row.extraShow}>
                                            <div>
                                                <Card className="mt-3"><Card.Body className="pb-0" style="padding: 1rem;">{row.extraValue}</Card.Body></Card>
                                            </div>
                                        </Collapse>
                                    </td>
                                </tr>
                                : undefined}
                        </>)}
                        {props.onAddStart ?
                        <tr>
                            <td colSpan={props.columnNames.length} style="vertical-align: middle; width: 100%;">
                                {props.addMessage}
                            </td>
                            <td style="text-align: right; vertical-align: middle;">
                                <Button variant="primary"
                                        size="sm"
                                        onClick={async () => {
                                            await props.onAddStart();
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

                <div class={`d-block d-${props.tableTill ? props.tableTill : 'sm'}-none`}>
                    {props.rows.map((row, i) => <Card className="mb-3">
                        <div class="card-header d-flex justify-content-between align-items-center" style="padding: 1rem;">
                            <h5 class="text-break" style="margin-bottom: 0;">{util.hasValue(row.fieldValues) ? row.fieldValues[0] : row.columnValues[0]}</h5>
                            <div style="white-space: nowrap; vertical-align: middle;">
                                <Button variant="primary"
                                        size="sm"
                                        className="ml-2"
                                        onClick={async () => {
                                            await row.onEditStart();
                                            this.setState({showEditModal: i});
                                        }}
                                        disabled={!row.onEditStart}>
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
                        <Card.Body className="pb-0" style="padding: 1rem;">
                            {this.get_card_fields(row)}
                            {row.extraValue ?
                                <Collapse in={row.extraShow}>
                                    <div>
                                        {row.extraFieldName ?
                                            <FormRow label={row.extraFieldName}>
                                                <Card><Card.Body className="pb-0" style="padding: 1rem;">{row.extraValue}</Card.Body></Card>
                                            </FormRow>
                                            : <Card><Card.Body className="pb-0" style="padding: 1rem;">{row.extraValue}</Card.Body></Card>}
                                    </div>
                                </Collapse>
                                : undefined}
                        </Card.Body>
                    </Card>)}
                    {props.onAddStart ?
                    <Card className="mb-0">
                        <div class="card-body d-flex justify-content-between align-items-center" style="padding: 1rem;">
                            <span class="text-break" style="font-size: 1rem;">{props.addMessage}</span>
                            <div style="white-space: nowrap; vertical-align: middle;">
                            <Button variant="primary"
                                    size="sm"
                                    className="ml-2"
                                    onClick={async () => {
                                        await props.onAddStart();
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
                        if (props.onAddCommit) {
                            await props.onAddCommit();
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
                    yes_text={__("component.table.add")}>
                    {state.showAddModal && props.onAddGetRows ?
                        props.onAddGetRows().map((addRow) =>
                            addRow.name ?
                        <FormRow label={addRow.name}>
                            {addRow.value}
                        </FormRow> : addRow.value) : undefined}
                </ItemModal>

                <ItemModal
                    onCheck={async () => {
                        if (props.rows[state.showEditModal].onEditCheck) {
                            return await props.rows[state.showEditModal].onEditCheck();
                        }

                        return true;
                    }}
                    onSubmit={async () => {
                        if (props.rows[state.showEditModal].onEditCommit) {
                            await props.rows[state.showEditModal].onEditCommit();
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
                    yes_text={__("component.table.apply")}>
                    {state.showEditModal !== null && props.rows[state.showEditModal].onEditGetRows ?
                        props.rows[state.showEditModal].onEditGetRows().map((editRow) =>
                            editRow.name ?
                        <FormRow label={editRow.name}>
                            {editRow.value}
                        </FormRow> : editRow.value) : undefined}
                </ItemModal>
            </>
        );
    }
}
