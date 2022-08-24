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

import { Component} from "preact";

// https://github.com/piotrwitek/utility-types#pickbyvaluet-valuetype
type PickByValue<T, ValueType> = Pick<
  T,
  { [Key in keyof T]-?: T[Key] extends ValueType ? Key : never }[keyof T]
>;

export abstract class ConfigComponent<P = {}, S = {}> extends Component<P, S> {
    toggle(x: keyof PickByValue<S, boolean>) {
        return () => this.setState({ [x]: !this.state[x] } as unknown as Partial<S>);
    }

    set<T extends keyof S>(x: T) {
        return (s: S[T]) => this.setState({ [x]: s } as unknown as Partial<S>);
    }
}
