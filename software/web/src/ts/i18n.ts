/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

import { signal, Signal } from "@preact/signals-core";

// get_languages can be overwritten by a module that can provide a function that returnes a user-specified preferred language.
// If no such module exists the languages preferred by the browser are used.
let get_languages: () => readonly string[] = () => navigator.languages;
export function set_languages_getter(f: () => readonly string[]) {
    get_languages = f;
}

let active_language: Signal<string> = signal("en");
let active_locale: Signal<string> = signal("en-US");

function update_active_language() {
    const languages = get_languages();

    for (let locale of languages) {
        let language = locale.substring(0, 2);

        if (language == 'en' || language == 'de') {
            if (active_language.value != language) {
                active_language.value = language;
            }

            if (active_locale.value != locale) {
                active_locale.value = locale;
            }

            return;
        }

        console.log("Sorry, got no translation for " + language);
    }

    if (active_language.value != 'en') {
        active_language.value = 'en';
    }

    if (active_locale.value != 'en-US') {
        active_locale.value = 'en-US';
    }
}

window.addEventListener('languagechange', update_active_language);

// Call update_active_language once initially.
// If system module is not used this has to be called once so the first
// language from navigator.languages is used, if there is no languagechange event.
update_active_language();

export function get_active_language() {
    return active_language.value;
}

export function get_active_locale() {
    return active_locale.value;
}

let number_formats: {[id: number]: {[locale: string]: Intl.NumberFormat}} = {};

export function toLocaleFixed(value: number, fractionDigits?: number) {
    if (fractionDigits === undefined) {
        fractionDigits = 0;
    }

    let locale = get_active_locale();

    if (!number_formats[fractionDigits]) {
        number_formats[fractionDigits] = {};
    }

    if (!number_formats[fractionDigits][locale]) {
        number_formats[fractionDigits][locale] = new Intl.NumberFormat(locale, {minimumFractionDigits: fractionDigits, maximumFractionDigits: fractionDigits});
    }

    return number_formats[fractionDigits][locale].format(value);
}

export function toLocaleString(date: Date, options?: Intl.DateTimeFormatOptions) {
    return date.toLocaleString(get_active_locale(), options);
}
