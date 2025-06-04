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

import * as util from "../../ts/util";
import { __ } from "../../ts/translation";
import { h, Fragment, Component, ComponentChildren, createRef } from "preact";
import { Button, Modal, ModalProps } from "react-bootstrap";

interface ItemModalProps extends ModalProps {
    onCheck?: () => Promise<boolean>;
    onSubmit: () => Promise<void>;
    onHide: () => Promise<void>;
    onImport?: (json: string) => Promise<void>;
    onExport?: () => Promise<string>;

    show: boolean;
    size?: "sm" | "lg" | "xl";

    title: string;
    children: ComponentChildren;
    no_variant?: string;
    no_text?: string;
    yes_variant?: string;
    yes_text?: string;

    export_basename?: string;
}

export class ItemModal extends Component<ItemModalProps, any> {
    import_input_ref = createRef();

    async report_error(title: string, message: string) {
        const modal = util.async_modal_ref.current;

        await modal.show({
            title: () => title,
            body: () => message,
            no_text: () => "",
            no_variant: "",
            yes_text: () => __("component.item_modal.error_button"),
            yes_variant: "primary",
            nestingDepth: 2,
            size: "lg",
        });
    }

    async import_file(file: File) {
        this.import_input_ref.current.value = null; // clear selected so that choosing the same file again will trigger the onchange event again

        if (!file) {
            await this.report_error(__("component.item_modal.import_error_title"), __("component.item_modal.import_error_no_file"));
            return;
        }

        if (file.type != "application/json") {
            await this.report_error(__("component.item_modal.import_error_title"), __("component.item_modal.import_error_wrong_file_type"));
            return;
        }

        let reader = new FileReader();

        reader.onload = async () => {
            try {
                await this.props.onImport(reader.result as string);
            }
            catch (error) {
                await this.report_error(__("component.item_modal.import_error_title"), error.message);
            }
        };

        reader.onerror = async () => {
            await this.report_error(__("component.item_modal.import_error_title"), __("component.item_modal.import_error_file_read_error"));
        };

        reader.readAsText(file);
    }

    async export_file() {
        let json;

        try {
            json = await this.props.onExport();
        } catch (error) {
            await this.report_error(__("component.item_modal.export_error_title"), error.message);
            return;
        }

        util.downloadToFile(json, this.props.export_basename + ".json", "application/json");
    }

    render(props: ItemModalProps) {
        let {onCheck, onSubmit, onHide, onImport, onExport, show, size, title, children, no_variant, no_text, yes_variant, yes_text, ...p} = props;

        return (
            <Modal size={size ?? "xl"} show={show} onHide={() => onHide()} centered {...p}>
                {/* There seems to be an incompatibility between preact's and react-bootstrap's typings*/ }
                <Modal.Header {...{closeButton: true} as any}>
                    <span class="modal-title form-label">{title}</span>
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
                    {onImport || onExport || (no_variant && no_text) || (yes_variant && yes_text) ?
                        <Modal.Footer>
                            {onImport ?
                            <><input ref={this.import_input_ref} class="d-none" type="file" accept=".json"
                                     onChange={async (e) => await this.import_file((e.target as HTMLInputElement).files[0])}/>
                            <Button variant="primary" onClick={() => this.import_input_ref.current.click()}>
                                {__("component.item_modal.import_button")}
                            </Button></>: undefined}
                            {onExport ?
                            <Button variant="primary" onClick={async () => await this.export_file()}>
                                {__("component.item_modal.export_button")}
                            </Button>: undefined}
                            {onImport || onExport ?
                                <div class="mr-auto" />: undefined}
                            {no_variant && no_text ?
                            <Button variant={no_variant} onClick={async () => await onHide()}>
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
