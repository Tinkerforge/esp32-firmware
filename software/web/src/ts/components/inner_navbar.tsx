import { h, Fragment, VNode } from "preact";
import Nav from "react-bootstrap/Nav";
import { NavbarItem } from "./navbar_item";
import { __ } from "../translation";
import { Home, Key } from "react-feather";
import Median from "median-js-bridge";
import { NavbarGroup } from "./navbar_group";
import { RemoteCloseButton } from "./remote_close_button";

interface NavbarProps {
    children: VNode<any>[],
    logo?: string,
    visible: boolean,
    mode: string,
}

function generateAppMenu(elements: VNode<any>[]): any {
    const appElements = [];
    for (const element of elements) {
        if (element.props.children && element.props.group_ref.current && !element.props.group_ref.current.props.hidden) {
            const childElements: any = generateAppMenu(element.props.children as any);
            const addElement = {
                label: element.props.group_ref.current.props.title,
                isGrouping: true,
                subLinks: childElements,
            };
            appElements.push(addElement);
            console.log("Group: ", element);
        } else if (!element.props || !element.props.children) {
            const singleElement = (element.type as any)();
            const addElement = {
                label: singleElement.props.title,
                url: `javascript:window.log("Test")`,
            };
            appElements.push(addElement);
        }
    }
    return appElements;
}

export function InnerNavbar(props: NavbarProps) {
    if (Median.isNativeApp()) {
        (window as any).log = alert;
        const appMenuItems = generateAppMenu(props.children);
        Median.sidebar.setItems({items: appMenuItems, enabled: true, persist: false});
    }

    return <Nav as="ul" className="flex-column" id="nav-list" style={props.visible ? undefined : "visibility: hidden;"}>
                {props.mode == "login" ?
                    <NavbarItem name="login" title={__("main.navbar_login")} symbol={<Key />} hidden={false} no_href={true} />
                    : undefined}
                {props.mode == "normal" ? <>
                    <NavbarItem name="status" title={__("main.navbar_status")} symbol={<Home />} hidden={false} />
                        {props.children}
                        <RemoteCloseButton />
                    </> : undefined}
            </Nav>
}
