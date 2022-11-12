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

import { h, Component} from "preact";

interface FormSeparatorProps {
    heading?: string
    colClasses?: string
}

export class FormSeparator extends Component<FormSeparatorProps, {}> {
    constructor() {
        super();
    }

    render(props: FormSeparatorProps) {
        return (
            <div class="row mb-3 pt-3">
                <div class={"d-flex border-bottom " + (props.colClasses === undefined ? "col-xl-8" : props.colClasses)}>{props.heading ? <span class="h3">{props.heading}</span>: undefined}</div>
            </div>
        );
    }
}
