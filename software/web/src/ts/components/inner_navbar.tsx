import { h, Fragment, VNode } from "preact";
import Nav from "react-bootstrap/Nav";
import { NavbarItem } from "./navbar_item";
import { __ } from "../translation";
import { Home, Key } from "react-feather";
import Median from "median-js-bridge";
import { NavbarGroup } from "./navbar_group";
import { RemoteCloseButton } from "./remote_close_button";
import { range, is_native_median_app } from "../util";

interface NavbarProps {
    children: VNode<any> | VNode<any>[],
    logo?: string,
    visible: boolean,
    mode: string,
}

function generateAppMenu(elements:  VNode<any> | VNode<any>[], nesting?: number): any {
    const appElements = [];
    if (!Array.isArray(elements))
        elements = [elements];

    for (const element of elements) {
        if (element.props.children && element.props.group_ref.current && !element.props.group_ref.current.props.hidden) {
            const childElements: any = generateAppMenu(element.props.children as any, nesting ? nesting + 1 : 1);
            const addElement = {
                label: element.props.group_ref.current.props.title,
                isGrouping: true,
                subLinks: childElements,
            };
            appElements.push(addElement);
        } else if (!element.props || !element.props.children) {
            const singleElement = (element.type as any)();
            let label = "";
            if (nesting) {
                for (const n in range(0, nesting)) {
                    label += "\u00A0\u00A0\u00A0\u00A0";
                }
            }
            label += singleElement.props.title;
            const addElement = {
                label: label,
                url: `javascript:window.switchTo('${singleElement.props.name}')`,
            };
            appElements.push(addElement);
        }
    }
    return appElements;
}

export function InnerNavbar(props: NavbarProps) {
    if (is_native_median_app()) {
        const appMenuItems = [{
            label: __("main.navbar_status"),
            url: "javascript:window.switchTo('status')",
        }];
        appMenuItems.push(...generateAppMenu(props.children));
        appMenuItems.push({
            label: __("component.remote_close_button.close"),
            url: "javascript:window.close()",
        });

        Median.sidebar.setItems({items: appMenuItems, enabled: true, persist: true});
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
