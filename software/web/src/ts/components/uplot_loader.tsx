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
import { Maximize, Maximize2 } from "react-feather";
import { __ } from "../../ts/translation";

interface UplotLoaderProps {
    show: boolean;
    marker_class: 'h3'|'h4';
    children: ComponentChildren;
    no_data: string;
    loading: string;
    fullscreen_allow?: boolean;
    fullscreen_title?: string;
    fullscreen_aspect_ratio?: number;
    fullscreen_mode?: 'button' | 'click';
}

interface UplotLoaderState {
    show_fullscreen: boolean;
}

export class UplotLoader extends Component<UplotLoaderProps, UplotLoaderState> {
    no_data_ref = createRef();
    loading_ref = createRef();
    fullscreen_chart_ref = createRef();

    // Close modal on back button
    popstate_handler: ((event: PopStateEvent) => void) | null = null;

    override componentDidMount() {
        this.popstate_handler = () => {this.setState({show_fullscreen: false})};
        window.addEventListener('popstate', this.popstate_handler);
    }

    override componentWillUnmount() {
        if (this.popstate_handler) {
            window.removeEventListener('popstate', this.popstate_handler);
            this.popstate_handler = null;
        }
    }

    constructor() {
        super();

        this.state = {
            show_fullscreen: false,
        };
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
            // Always show legend and full y-axis annotation
            legend_show: true,
            y_three_split: false,
        };

        return cloneElement(main_chart, fullscreen_props);
    }

    render() {
        const fullscreen_mode = this.props.fullscreen_mode || 'button';

        return (
            <div
                style={fullscreen_mode === 'click' ? "cursor: pointer;" : undefined}
                onClick={fullscreen_mode === 'click' ? () => this.setState({show_fullscreen: true}) : undefined}
            >
                <div ref={this.no_data_ref} style={`position: absolute; width: 100%; height: 100%; visibility: hidden; display: ${this.props.show ? 'flex' : 'none'};`}>
                    <span class={this.props.marker_class} style="margin: auto;">{this.props.no_data}</span>
                </div>
                <div ref={this.loading_ref} style={`position: absolute; width: 100%; height: 100%; visibility: ${this.props.show ? 'inherit' : 'hidden'}; display: ${this.props.show ? 'flex' : 'none'};`}>
                    <span class={this.props.marker_class} style="margin: auto;">{this.props.loading}</span>
                </div>

                { // Fullscreen button
                this.props.fullscreen_allow && ((fullscreen_mode === 'button' && (
                    <Button
                        variant="outline-secondary"
                        size="sm"
                        className="position-absolute"
                        style="top: 10px; right: 10px; z-index: 10;"
                        onClick={() => this.setState({show_fullscreen: true})}
                        title={__("component.uplot.fullscreen")}
                    >
                        <Maximize />
                    </Button>
                )) || (fullscreen_mode === 'click' && (
                    <div
                        className="position-absolute"
                        style="top: 0px; right: 0px; z-index: 10;"
                        onClick={() => this.setState({show_fullscreen: true})}
                    >
                        <Maximize2 size={12} />
                    </div>
                )))}

                {this.props.children}

                { // Fullscreen Chart Modal
                this.props.fullscreen_allow && (
                    <Modal
                        show={this.state.show_fullscreen}
                        onHide={() => this.setState({show_fullscreen: false})}
                        onExiting={() => {
                            // Disconnect observer to prevent access of "getComputedStyle"
                            // after modal is closed
                            if (this.fullscreen_chart_ref.current?.observer) {
                                this.fullscreen_chart_ref.current.observer.disconnect();
                            }
                            this.fullscreen_chart_ref.current = null;
                        }}
                        size="xl"
                        centered
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
            </div>
        );
    }
}
