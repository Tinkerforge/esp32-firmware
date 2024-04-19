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

import { h, Fragment, Component, ComponentChildren, createRef } from "preact";

interface UplotLoaderProps {
    show: boolean;
    marker_class: 'h3'|'h4';
    children: ComponentChildren;
    no_data: string;
    loading: string;
}

export class UplotLoader extends Component<UplotLoaderProps, {}> {
    no_data_ref = createRef();
    loading_ref = createRef();

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
                {this.props.children}
            </>
        );
    }
}
