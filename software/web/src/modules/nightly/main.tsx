import * as API from "../../ts/api";
import { underconstruction_gif } from "./underconstruction_gif.embedded";

export function init() {
    let parent = document.querySelector('nav.navbar > a');
    if (parent == null)
        return;

    let target = document.createElement("img");
    (target.style as any) = "position: fixed; top:10px; left:0px; z-index: 10000;";
    target.src = underconstruction_gif;

    parent.after(target);
}

export function add_event_listeners(source: API.APIEventTarget) {}

export function update_sidebar_state(module_init: any) {}
