import { Language } from "./language.enum";

export interface i18n_config {
    language: Language;
    detect_browser_language: boolean;
}

export interface last_reset {
    reason: number;
    show_warning: boolean;
}

export interface hide_last_reset_warning {
}
