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

interface DebugState {
    uptime: number,
    free_heap: number,
    largest_free_heap_block: number
}

function update_debug_state(state: DebugState) {
    $('#debug_uptime').val(util.format_timespan(Math.round(state.uptime / 1000)));
    $('#debug_heap_free').val(state.free_heap);
    $('#debug_heap_block').val(state.largest_free_heap_block);
}

export function init() {

}

export function addEventListeners(source: EventSource) {
    source.addEventListener('debug/state', function (e: util.SSE) {
        update_debug_state(<DebugState>(JSON.parse(e.data)));
    }, false);
}

export function updateLockState(module_init: any) {
    $('#sidebar-debug').prop('hidden', !module_init.debug);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "debug": {
                "status": {
                    "debug": "Debug"
                },
                "navbar": {
                    "debug": "Debug"
                },
                "content": {
                    "debug": "Debug",
                    "uptime": "Zeit seit Neustart",
                    "heap_free": "Freie Heap-Bytes",
                    "heap_block": "Größter freier Heap-Block",
                },
                "script": {
                }
            }
        },
        "en": {
            "debug": {
                "status": {
                    "debug": "Debug"
                },
                "navbar": {
                    "debug": "Debug"
                },
                "content": {
                    "debug": "Debug",
                    "uptime": "Uptime",
                    "heap_free": "Free heap bytes",
                    "heap_block": "Largest free heap block",
                },
                "script": {
                }
            }
        }
    };
    return translations[lang];
}
