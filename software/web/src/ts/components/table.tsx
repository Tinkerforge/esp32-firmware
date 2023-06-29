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
import { FormGroup } from "../../ts/components/form_group";

export interface TableRow {
    columnData: string[]
    columnRepresentations: VNode[]
    onEditClick?: () => void
    onRemoveClick?: () => void
}

export interface TableProps {
    columnNames: string[]
    rows: TableRow[]
    maxRowCount?: number
    addMessage?: string
    onAddClick?: () => void
    tableTill?: string
}

export class Table extends Component<TableProps, any> {
    render(props: TableProps) {
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
                        {props.rows.map((row) =>
                            <tr>
                                {row.columnRepresentations.map((representation) => (
                                    <td class="text-break" style="vertical-align: middle;">{representation}</td>
                                ))}
                                <td style="width: 1%; white-space: nowrap; vertical-align: middle;">
                                    <Button variant="primary"
                                            size="sm"
                                            className="mr-2"
                                            onClick={row.onEditClick}
                                            style={row.onEditClick ? "" : "visibility: hidden;"}>
                                        <Edit3/>
                                    </Button>
                                    <Button variant="danger"
                                            size="sm"
                                            onClick={row.onRemoveClick}
                                            style={row.onRemoveClick ? "" : "visibility: hidden;"}>
                                        <Trash2/>
                                    </Button>
                                </td>
                            </tr>)}
                        {this.props.onAddClick ?
                        <tr scope="col">
                            <td colSpan={props.columnNames.length} style="vertical-align: middle; width: 100%;">
                                {props.addMessage}
                            </td>
                            <td style="text-align: right;">
                                <Button variant="primary"
                                        size="sm"
                                        onClick={this.props.onAddClick}
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
                    {props.rows.map((row) => <Card className="mb-3">
                        <div class="card-header d-flex justify-content-between align-items-center">
                            <h5 class="text-break" style="margin-bottom: 0;">{row.columnRepresentations[0]}</h5>
                            <div style="white-space: nowrap; vertical-align: middle;">
                                <Button variant="primary"
                                        size="sm"
                                        className="ml-2"
                                        onClick={row.onEditClick}
                                        style={row.onEditClick ? "" : "visibility: hidden;"}>
                                    <Edit3/>
                                </Button>
                                <Button variant="danger"
                                        size="sm"
                                        className="ml-2"
                                        onClick={row.onRemoveClick}
                                        style={row.onRemoveClick ? "" : "visibility: hidden;"}>
                                    <Trash2/>
                                </Button>
                            </div>
                        </div>
                        <Card.Body style="padding-bottom: 0.5rem;">
                        {props.columnNames.slice(1).map((columnName, i) =>
                        <FormGroup label={columnName}>
                            <span class="form-control" readonly>{row.columnRepresentations[1 + i]}</span>
                        </FormGroup>)}
                    </Card.Body></Card>)}
                    {this.props.onAddClick ?
                    <Card className="mb-0">
                        <div class="card-body d-flex justify-content-between align-items-center">
                            <span class="text-break" style="font-size: 1rem;">{props.addMessage}</span>
                            <div style="white-space: nowrap; vertical-align: middle;">
                            <Button variant="primary"
                                    size="sm"
                                    className="ml-2"
                                    onClick={this.props.onAddClick}
                                    disabled={props.maxRowCount !== undefined && props.rows.length >= props.maxRowCount}>
                                <Plus/>
                            </Button>
                            </div>
                        </div>
                    </Card>
                    : undefined}
                </div>
            </>
        );
    }
}
