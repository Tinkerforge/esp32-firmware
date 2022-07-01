/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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

import { h, Component } from "preact";

export interface ConfigPageHeaderProps {
    page: string
}

export class ConfigPageHeader extends Component<ConfigPageHeaderProps, any> {
    render() {
        let title_data_i18n = this.props.page + ".content." + this.props.page;
        let save_button_id = this.props.page + "_config_save_button";
        let save_button_form = this.props.page + "_config_form";
        let save_button_data_i18n = this.props.page + ".content.save";
    
        return (
            <div class="row sticky-under-top mb-3 pt-3">
                <div class="col-xl-8 d-flex justify-content-between pb-2 border-bottom tab-header-shadow">
                    <h1 class="h2" data-i18n={title_data_i18n}></h1>
                    <button id={save_button_id} type="submit" form={save_button_form} class="btn btn-primary mb-2" data-i18n={save_button_data_i18n} disabled></button>
                </div>
            </div>
        );
    }
}
