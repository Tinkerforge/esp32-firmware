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

import { h, Component, ComponentChildren } from "preact";
import { Button, Modal, ModalProps } from "react-bootstrap";

interface ItemModalProps extends ModalProps {
    onCheck?: () => Promise<boolean>;
    onSubmit: () => Promise<void>;
    onHide: () => Promise<void>;

    show: boolean;
    size?: "sm" | "lg" | "xl";

    title: string;
    children: ComponentChildren;
    no_variant?: string;
    no_text?: string;
    yes_variant?: string;
    yes_text?: string;
}

export class ItemModal extends Component<ItemModalProps, any> {
    render(props: ItemModalProps) {
        let {onCheck, onSubmit, onHide, show, size, title, children, no_variant, no_text, yes_variant, yes_text, ...p} = props;

        return (
            <Modal size={size ?? "xl"} show={show} onHide={() => onHide()} centered {...p}>
                {/* There seems to be an incompatibility between preact's and react-bootstrap's typings*/ }
                <Modal.Header {...{closeButton: true} as any}>
                    <label class="modal-title form-label">{title}</label>
                </Modal.Header>
                <form onSubmit={async (e) => {
                    e.preventDefault();
                    e.stopPropagation();

                    if (!(e.target as HTMLFormElement).checkValidity() || (e.target as HTMLFormElement).querySelector(".is-invalid")) {
                        return;
                    }

                    if (onCheck && !await onCheck()) {
                        return;
                    }

                    await onSubmit();
                    await onHide();
                }}>
                    <Modal.Body className="pb-0">
                        {children}
                    </Modal.Body>
                    {(no_variant && no_text) || (yes_variant && yes_text) ?
                        <Modal.Footer>
                            {no_variant && no_text ?
                            <Button variant={no_variant} onClick={() => onHide()}>
                                {no_text}
                            </Button>: undefined}
                            {yes_variant && yes_text ?
                            <Button variant={yes_variant} type="submit">
                                {yes_text}
                            </Button> : undefined}
                        </Modal.Footer>
                        : undefined}
                </form>
            </Modal>
        );
    }
}
