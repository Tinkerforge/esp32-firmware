/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

import { h, Fragment, ComponentChildren, toChildArray, VNode } from "preact";
import Row from "react-bootstrap/Row";
import Tab from "react-bootstrap/Tab";
import { __ } from "../translation";
import { PageHeader } from "./page_header";
import { ConfigForm } from "./config_form";

// SubPage.Status component
// This marks content as the status section
interface SubPageStatusProps {
    header?: ComponentChildren;
    children?: ComponentChildren;
}

function SubPageStatus(props: SubPageStatusProps): VNode {
    // Used as marker
    return null;
}

// SubPage.Config component
// Wraps configuration form content
interface SubPageConfigProps {
    children: ComponentChildren;
    id: string;
    isModified: boolean;
    isDirty: boolean;
    onSave: () => Promise<void>;
    onReset: () => Promise<void>;
    onDirtyChange: (dirty: boolean) => void;
}

function SubPageConfig(props: SubPageConfigProps): VNode {
    // Used as marker
    return null;
}

interface SubPageProps {
    children?: ComponentChildren;
    name: string;
    title?: ComponentChildren;  // Main page title
    colClasses?: string;
}

interface SubPageComponent {
    (props: SubPageProps): VNode;
    Status: typeof SubPageStatus;
    Config: typeof SubPageConfig;
}

const SubPage: SubPageComponent = (props: SubPageProps) => {
    const children = toChildArray(props.children);

    // Find SubPage.Status and SubPage.Config among children
    let statusProps: SubPageStatusProps | null = null;
    let statusChildren: ComponentChildren = null;
    let configProps: SubPageConfigProps | null = null;
    let configChildren: ComponentChildren = null;
    let otherChildren: Array<VNode | string | number> = [];

    for (const child of children) {
        if (child && typeof child === 'object' && 'type' in child) {
            if (child.type === SubPageStatus) {
                statusProps = child.props as SubPageStatusProps;
                statusChildren = statusProps.children;
            } else if (child.type === SubPageConfig) {
                configProps = child.props as SubPageConfigProps;
                configChildren = configProps.children;
            } else {
                otherChildren.push(child);
            }
        } else {
            otherChildren.push(child);
        }
    }

    // Determine rendering mode
    const hasStatus = statusProps !== null;
    const hasConfig = configProps !== null;

    let content: ComponentChildren;

    if (hasStatus && hasConfig) {
        // New pattern: Status at top, then ConfigForm with small
        content = (
            <>
                <PageHeader title={props.title}>
                    {statusProps.header}
                </PageHeader>
                {statusChildren}
                <ConfigForm
                    id={configProps.id}
                    title={__("component.sub_page.settings")}
                    isModified={configProps.isModified}
                    isDirty={configProps.isDirty}
                    onSave={configProps.onSave}
                    onReset={configProps.onReset}
                    onDirtyChange={configProps.onDirtyChange}
                    small
                >
                    {configChildren}
                </ConfigForm>
                {otherChildren}
            </>
        );
    } else if (hasStatus && !hasConfig) {
        // Status-only page
        content = (
            <>
                <PageHeader title={props.title}>
                    {statusProps.header}
                </PageHeader>
                {statusChildren}
                {otherChildren}
            </>
        );
    } else if (!hasStatus && hasConfig) {
        // Config-only page
        // Use SubPage title as ConfigForm title (not small)
        content = (
            <>
                <ConfigForm
                    id={configProps.id}
                    title={props.title}
                    isModified={configProps.isModified}
                    isDirty={configProps.isDirty}
                    onSave={configProps.onSave}
                    onReset={configProps.onReset}
                    onDirtyChange={configProps.onDirtyChange}
                >
                    {configChildren}
                </ConfigForm>
                {otherChildren}
            </>
        );
    } else {
        // Backward compatible: no Status or Config markers, render children as-is
        content = props.children;
    }

    return (
        <Tab.Pane eventKey={props.name}>
            <Row>
                <div class={props.colClasses === undefined ? "col-xl-8" : props.colClasses}>
                    {content}
                </div>
            </Row>
        </Tab.Pane>
    );
};

// Attach sub-components
SubPage.Status = SubPageStatus;
SubPage.Config = SubPageConfig;

export { SubPage };
