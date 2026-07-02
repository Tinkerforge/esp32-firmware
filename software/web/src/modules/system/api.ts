import { Language } from "../../ts/language";
import { ColorScheme } from "./generated/color_scheme.enum";
import { Country } from "./generated/country.enum";

export interface i18n_config {
    language: Language;
    detect_browser_language: boolean;
}

export interface theme_config {
    color_scheme: ColorScheme;
}

export interface country_config {
    country: Country;
}

export interface last_reset {
    reason: number;
    show_warning: boolean;
}

export interface hide_last_reset_warning {
}
