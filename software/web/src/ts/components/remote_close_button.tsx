
import { h } from "preact";
import * as util from "../util";
import { __ } from "../translation";
import Nav  from "react-bootstrap/Nav";
import { X } from "react-feather";


export function RemoteCloseButton() {
    let closeFn = util.closeRemoteConnection;
    if (util.is_warp_app() && window.tinkerforge_devices) {
      closeFn = () => window.tinkerforge_devices.resetToDevices();
    }
    return <Nav.Item as="li" hidden={!util.remoteAccessMode && !util.is_warp_app()}>
            <Nav.Link className="row gx-2 d-flex-ni align-items-center text-danger" role="button" onClick={closeFn}>
                <span class="col-auto"><X/></span><span class="col">{__("component.remote_close_button.close")}</span>
            </Nav.Link>
        </Nav.Item>
}
