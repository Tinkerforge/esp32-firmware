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
import { Card, Button } from "react-bootstrap";
import { Plus, Edit3, Trash2 } from "react-feather";
import { FormGroup } from "./form_group";
import { ItemModal } from "./item_modal";

export interface TableModalRow {
    name: string
    // FormRow only accepts VNodes, not other ComponentChild variants.
    value: VNode
    valueClassList?: string
}

export interface TableRow {
    columnValues: ComponentChild[][]
    editTitle?: string
    onEditStart?: () => Promise<void>
    onEditGetRows?: () => TableModalRow[]
    onEditCheck?: () => Promise<boolean>
    onEditCommit?: () => Promise<void>
    onEditAbort?: () => Promise<void>
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
    onAddAbort?: () => Promise<void>
    tableTill?: string
}

interface TableState {
    showAddModal: boolean
    showEditModal: number
}

const get_column_value = (value: ComponentChild[], i: number): ComponentChild => {
    if (i < value.length) {
        return value[i];
    }

    return value[0];
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

    render(props: TableProps, state: TableState) {
        return (
            <>
                <Card className={`d-none d-${props.tableTill ? props.tableTill : 'sm'}-block`}><Card.Body style="padding: 0;">
                <table class="table table-responsive" style="font-size: 1rem; margin-bottom: 0;">
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
                        {props.rows.map((row, i) =>
                            <tr>
                                {row.columnValues.map((value) => (
                                    <td class="text-break" style="vertical-align: middle;">{get_column_value(value, 0)}</td>
                                ))}
                                <td style="width: 1%; white-space: nowrap; vertical-align: middle;">
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
                            </tr>)}
                        {props.onAddStart ?
                        <tr scope="col">
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
                            <h5 class="text-break" style="margin-bottom: 0;">{get_column_value(row.columnValues[0], 1)}</h5>
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
                        <Card.Body style="padding: 1rem;">
                        {props.columnNames.slice(1).map((columnName, i, array) =>
                        <FormGroup label={columnName} classList={i == array.length - 1 ? " mb-0" : ""}>
                            <span class="form-control" style="height: unset;" readonly>{get_column_value(row.columnValues[1 + i], 1) ? get_column_value(row.columnValues[1 + i], 1) : <>&nbsp;</>}</span>
                        </FormGroup>)}
                    </Card.Body></Card>)}
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

                        this.setState({showAddModal: false});
                    }}
                    onHide={async () => {
                        if (props.onAddAbort) {
                            await props.onAddAbort();
                        }

                        this.setState({showAddModal: false});
                    }}
                    show={state.showAddModal}
                    no_variant="secondary"
                    yes_variant="primary"
                    title={props.addTitle}
                    no_text={__("component.table.abort")}
                    yes_text={__("component.table.add")}>
                    {state.showAddModal && props.onAddGetRows ?
                        props.onAddGetRows().map((addRow, i, array) =>
                        <FormGroup label={addRow.name} classList={i == array.length - 1 ? " mb-0" : ""} valueClassList={addRow.valueClassList}>
                            {addRow.value}
                        </FormGroup>) : undefined}
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

                        this.setState({showEditModal: null});
                    }}
                    onHide={async () => {
                        if (props.rows[state.showEditModal].onEditAbort) {
                            await props.rows[state.showEditModal].onEditAbort();
                        }

                        this.setState({showEditModal: null});
                    }}
                    show={state.showEditModal !== null}
                    no_variant="secondary"
                    yes_variant="primary"
                    title={state.showEditModal !== null ? props.rows[state.showEditModal].editTitle : ''}
                    no_text={__("component.table.abort")}
                    yes_text={__("component.table.apply")}>
                    {state.showEditModal !== null && props.rows[state.showEditModal].onEditGetRows ?
                        props.rows[state.showEditModal].onEditGetRows().map((editRow, i, array) =>
                        <FormGroup label={editRow.name} classList={i == array.length - 1 ? " mb-0" : ""} valueClassList={editRow.valueClassList}>
                            {editRow.value}
                        </FormGroup>) : undefined}
                </ItemModal>
            </>
        );
    }
}
