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
import { h, Component, Fragment, VNode } from "preact";
import { Card, Button } from "react-bootstrap";
import { Plus, Edit3, Trash2 } from "react-feather";
import { FormGroup } from "./form_group";
import { ItemModal } from "./item_modal";

export interface TableModalRow {
    name: string
    value: VNode
}

export interface TableRow {
    columnValues: VNode[]
    editTitle?: string
    onEditStart?: () => void
    onEditGetRows?: () => TableModalRow[]
    onEditCommit?: () => void
    onEditAbort?: () => void
    onRemoveClick?: () => void
}

export interface TableProps {
    columnNames: string[]
    rows: TableRow[]
    maxRowCount?: number
    addMessage?: string
    addTitle?: string
    onAddStart?: () => void
    onAddGetRows?: () => TableModalRow[]
    onAddCommit?: () => void
    onAddAbort?: () => void
    tableTill?: string
}

interface TableState {
    showAddModal: boolean
    showEditModal: number
}

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
                            <th scope="col"></th>
                        </tr>
                    </thead>
                    <tbody>
                        {props.rows.map((row, i) =>
                            <tr>
                                {row.columnValues.map((value) => (
                                    <td class="text-break" style="vertical-align: middle;">{value}</td>
                                ))}
                                <td style="width: 1%; white-space: nowrap; vertical-align: middle;">
                                    <Button variant="primary"
                                            size="sm"
                                            className="mr-2"
                                            onClick={() => {
                                                this.setState({showEditModal: i});
                                                row.onEditStart();
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
                            <td style="text-align: right;">
                                <Button variant="primary"
                                        size="sm"
                                        onClick={() => {
                                            this.setState({showAddModal: true});
                                            props.onAddStart();
                                        }}
                                        disabled={props.maxRowCount !== undefined && props.rows.length >= props.maxRowCount}>
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
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <h5 class="text-break" style="margin-bottom: 0;">{row.columnValues[0]}</h5>
                            <div style="white-space: nowrap; vertical-align: middle;">
                                <Button variant="primary"
                                        size="sm"
                                        className="ml-2"
                                        onClick={() => {
                                            this.setState({showEditModal: i});
                                            row.onEditStart();
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
                        <Card.Body style="padding-bottom: 0.5rem;">
                        {props.columnNames.slice(1).map((columnName, i) =>
                        <FormGroup label={columnName}>
                            <span class="form-control" readonly>{row.columnValues[1 + i]}</span>
                        </FormGroup>)}
                    </Card.Body></Card>)}
                    {props.onAddStart ?
                    <Card className="mb-0">
                        <div class="card-body d-flex justify-content-between align-items-center">
                            <span class="text-break" style="font-size: 1rem;">{props.addMessage}</span>
                            <div style="white-space: nowrap; vertical-align: middle;">
                            <Button variant="primary"
                                    size="sm"
                                    className="ml-2"
                                    onClick={() => {
                                        this.setState({showAddModal: true});
                                        props.onAddStart();
                                    }}
                                    disabled={props.maxRowCount !== undefined && props.rows.length >= props.maxRowCount}>
                                <Plus/>
                            </Button>
                            </div>
                        </div>
                    </Card>
                    : undefined}
                </div>

                <ItemModal
                    onSubmit={() => {
                        if (props.onAddCommit) {
                            props.onAddCommit();
                        }

                        this.setState({showAddModal: false});
                    }}
                    onHide={() => {
                        if (props.onAddAbort) {
                            props.onAddAbort();
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
                        props.onAddGetRows().map((addRow) =>
                        <FormGroup label={addRow.name}>
                            {addRow.value}
                        </FormGroup>) : undefined}
                </ItemModal>

                <ItemModal
                    onSubmit={() => {
                        if (props.rows[state.showEditModal].onEditCommit) {
                            props.rows[state.showEditModal].onEditCommit();
                        }

                        this.setState({showEditModal: null});
                    }}
                    onHide={() => {
                        if (props.rows[state.showEditModal].onEditAbort) {
                            props.rows[state.showEditModal].onEditAbort();
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
                        props.rows[state.showEditModal].onEditGetRows().map((editRow) =>
                        <FormGroup label={editRow.name}>
                            {editRow.value}
                        </FormGroup>) : undefined}
                </ItemModal>
            </>
        );
    }
}
