/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Fragment, Component, ComponentChildren, createRef, cloneElement } from "preact";
import { Modal, Button } from "react-bootstrap";

interface UplotLoaderProps {
    show: boolean;
    marker_class: 'h3'|'h4';
    children: ComponentChildren;
    no_data: string;
    loading: string;
    fullscreen_allow?: boolean;
    fullscreen_title?: string;
    fullscreen_aspect_ratio?: number;
}

export class UplotLoader extends Component<UplotLoaderProps, {show_fullscreen: boolean}> {
    no_data_ref = createRef();
    loading_ref = createRef();
    fullscreen_chart_ref = createRef();

    constructor() {
        super();
        this.setState({show_fullscreen: false});
    }

    set_loading() {
        this.no_data_ref.current.style.visibility = 'hidden';
        this.loading_ref.current.style.visibility = 'inherit';
    }

    set_show(show: boolean) {
        this.no_data_ref.current.style.display = show ? 'flex' : 'none';
        this.loading_ref.current.style.display = show ? 'flex' : 'none';
    }

    set_data(data: boolean) {
        this.no_data_ref.current.style.visibility = data ? 'hidden' : 'inherit';
        this.loading_ref.current.style.visibility = 'hidden';

        // Update fullscreen chart data if it exists
        if (this.fullscreen_chart_ref.current && this.fullscreen_chart_ref.current.set_data) {
            const main_chart = this.props.children as any;
            if (main_chart && main_chart.ref && main_chart.ref.current && main_chart.ref.current.data) {
                this.fullscreen_chart_ref.current.set_data(main_chart.ref.current.data);
            }
        }
    }

    create_fullscreen_chart() {
        const main_chart = this.props.children as any;
        if (!main_chart || !main_chart.props) {
            return null;
        }

        // Clone the main chart with modifications for fullscreen
        const fullscreen_props = {
            ...main_chart.props,
            ref: this.fullscreen_chart_ref,
            class: main_chart.props.class + "-fullscreen",
            show: this.state.show_fullscreen,
            aspect_ratio: this.props.fullscreen_aspect_ratio || 1.777,
        };

        return cloneElement(main_chart, fullscreen_props);
    }

    render() {
        return (
            <>
                <div ref={this.no_data_ref} style={`position: absolute; width: 100%; height: 100%; visibility: hidden; display: ${this.props.show ? 'flex' : 'none'};`}>
                    <span class={this.props.marker_class} style="margin: auto;">{this.props.no_data}</span>
                </div>
                <div ref={this.loading_ref} style={`position: absolute; width: 100%; height: 100%; visibility: ${this.props.show ? 'inherit' : 'hidden'}; display: ${this.props.show ? 'flex' : 'none'};`}>
                    <span class={this.props.marker_class} style="margin: auto;">{this.props.loading}</span>
                </div>

                { // Fullscreen Button
                this.props.fullscreen_allow && (
                    <Button
                        variant="outline-secondary"
                        size="sm"
                        className="position-absolute"
                        style="top: 10px; right: 10px; z-index: 10;"
                        onClick={() => this.setState({show_fullscreen: true})}
                        title={__("component.uplot.fullscreen")}
                    >
                        <svg width="16" height="16" fill="currentColor" viewBox="0 0 16 16">
                            <path d="M1.5 1a.5.5 0 0 0-.5.5v4a.5.5 0 0 1-1 0v-4A1.5 1.5 0 0 1 1.5 0h4a.5.5 0 0 1 0 1h-4zM10 .5a.5.5 0 0 1 .5-.5h4A1.5 1.5 0 0 1 16 1.5v4a.5.5 0 0 1-1 0v-4a.5.5 0 0 0-.5-.5h-4a.5.5 0 0 1-.5-.5zM.5 10a.5.5 0 0 1 .5.5v4a.5.5 0 0 0 .5.5h4a.5.5 0 0 1 0 1h-4A1.5 1.5 0 0 1 0 14.5v-4a.5.5 0 0 1 .5-.5zm15 0a.5.5 0 0 1 .5.5v4a1.5 1.5 0 0 1-1.5 1.5h-4a.5.5 0 0 1 0-1h4a.5.5 0 0 0 .5-.5v-4a.5.5 0 0 1 .5-.5z"/>
                        </svg>
                    </Button>
                )}

                {this.props.children}

                { // Fullscreen Chart Modal
                this.props.fullscreen_allow && (
                    <Modal
                        show={this.state.show_fullscreen}
                        onHide={() => this.setState({show_fullscreen: false})}
                        size="xl"
                        centered
                        dialogClassName="modal-fullscreen-chart"
                    >
                        <Modal.Header {...{closeButton: true} as any}>
                            <Modal.Title>{this.props.fullscreen_title || __("component.uplot.fullscreen_chart")}</Modal.Title>
                        </Modal.Header>
                        <Modal.Body className="p-3 d-flex justify-content-center align-items-center">
                            {
                            // Do not try to use absolute width in px or similar here, otherwise
                            // there is a funky sizing race-condition between the modal and the
                            // chart that only happens sometimes.
                            }
                            <div style={`width: 100%;`}>
                                {this.create_fullscreen_chart()}
                            </div>
                        </Modal.Body>
                    </Modal>
                )}
            </>
        );
    }
}
