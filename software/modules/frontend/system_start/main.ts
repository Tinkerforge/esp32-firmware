/* esp32-brick
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

import $ from "jquery";

import * as util from "../util";

declare function __(s: string): string;

export function init() {
    $('#system').on('hide.bs.collapse', () => $('#system-chevron').removeClass("rotated-chevron"));
    $('#system').on('show.bs.collapse', () => $('#system-chevron').addClass("rotated-chevron"));
}

export function addEventListeners(source: EventSource) {}

export function updateLockState(module_init: any) {}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "system_start": {
                "status": {},
                "navbar": {
                    "system_start": "System"
                },
                "content": {},
                "script": {}
            }
        },
        "en": {
            "system_start": {
                "status": {},
                "navbar": {
                    "system_start": "System"
                },
                "content": {},
                "script": {}
            }
        }
    };
    return translations[lang];
}
