const enum Language {
    German  = 0,
    English = 1,
}

export interface i18n_config {
    language: Language;
    detect_browser_language: boolean;
}
