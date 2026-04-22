
import { h } from "preact";
import * as util from "../util";
import { __ } from "../translation";
import Nav  from "react-bootstrap/Nav";
import { X } from "react-feather";


export function RemoteCloseButton() {
    return <Nav.Item as="li" hidden={!util.remoteAccessMode}>
            <Nav.Link className="row gx-2 d-flex-ni align-items-center text-danger" role="button" onClick={util.closeRemoteConnection}>
                <span class="col-auto"><X/></span><span class="col">{__("component.remote_close_button.close")}</span>
            </Nav.Link>
        </Nav.Item>
}
