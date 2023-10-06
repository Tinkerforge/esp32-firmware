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

import { h, Component, render, ComponentChildren } from "preact";
import { Button, Modal } from "react-bootstrap";
import * as util from "../../ts/util";

interface AsyncModalProps {
}

interface AsyncModalStrings {
    title: string
    body: ComponentChildren
    no_variant: string
    no_text: string

    yes_variant: string
    yes_text: string
}

interface AsyncModalState extends AsyncModalStrings {
    show: boolean

    promiseResolve: (value: boolean | PromiseLike<boolean>) => void
}

export class AsyncModal extends Component<AsyncModalProps, AsyncModalState> {
    constructor() {
        super();
    }

    show = async (strings: AsyncModalStrings) => {
        return new Promise<boolean>((resolve) => {
            this.setState({
                show: true,
                promiseResolve: resolve,
                ...strings
            });
        });
    }

    hide = (b: boolean) => {
        this.state.promiseResolve(b);
        this.setState({
            show: false,
            promiseResolve: null
        });
    }

    render(props: AsyncModalProps, state: Readonly<AsyncModalState>) {
        return (
           <Modal show={state.show} onHide={() => {this.hide(false)}} centered>
                {/* There seems to be an incompatibility between preact's and react-bootstrap's typings*/ }
                <Modal.Header {...{closeButton: true} as any}>
                    <label class="modal-title form-label">{state.title}</label>
                </Modal.Header>
                <Modal.Body>{state.body}</Modal.Body>
                <Modal.Footer>
                    <Button variant={state.no_variant} onClick={() => {this.hide(false)}}>
                        {state.no_text}
                    </Button>
                    <Button variant={state.yes_variant} onClick={() => {this.hide(true)}}>
                        {state.yes_text}
                    </Button>
                </Modal.Footer>
            </Modal>
        );
    }
}

export function init_async_modal() {
    render(<AsyncModal ref={util.async_modal_ref}/>, document.getElementById('async_modal'))
}
