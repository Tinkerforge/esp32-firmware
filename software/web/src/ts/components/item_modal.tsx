/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

import { h, Component, Context, createContext, toChildArray, VNode, cloneElement } from "preact";
import { Button, Modal, ModalProps } from "react-bootstrap";
import { __ } from "../translation";


interface ItemModalProps extends ModalProps {
    onSubmit: () => void
    onHide: () => void

    show: boolean

    title: string
    children: VNode | VNode[]
    no_variant: string
    no_text: string
    yes_variant: string
    yes_text: string
}


let id_counter = 0;

export class ItemModal extends Component<ItemModalProps, any> {
    idContext: Context<string>;
    id: string;


    constructor() {
        super();
        this.id = "itemmodal" + id_counter;
        this.idContext = createContext(this.id);
        ++id_counter;
    }

    render(props: ItemModalProps) {
        let {onSubmit, onHide, show, title, children, no_variant, no_text, yes_variant, yes_text, ...p} = props;
        
        return (
            <Modal show={show} onHide={() => onHide()} centered {...p}>
                <Modal.Header closeButton>
                    <label class="modal-title form-label">{title}</label>
                </Modal.Header>
                <form onSubmit={(e) => {
                    e.preventDefault();
                    e.stopPropagation();

                    if (!(e.target as HTMLFormElement).checkValidity() || (e.target as HTMLFormElement).querySelector(".is-invalid")) {
                        return;
                    }

                    onSubmit();
                    onHide();
                }}>
                    <Modal.Body>
                        {(toChildArray(children) as VNode[]).map(c => cloneElement(c, {idContext: this.idContext}))}
                    </Modal.Body>
                    <Modal.Footer>
                        <Button variant={no_variant} onClick={() => onHide()}>
                            {no_text}
                        </Button>
                        <Button variant={yes_variant} type="submit">
                            {yes_text}
                        </Button>
                    </Modal.Footer>
                </form>
            </Modal>
        );
    }
}
