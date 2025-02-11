/*
2025a;2025-02-11T10:08:00.517253+00:00
*/
/* esp32-firmware
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

let timezones: {[key: string]: {[key: string]: null | {[key: string]: null}}} = {
    "Africa": {
        "Abidjan": null,
        "Accra": null,
        "Addis_Ababa": null,
        "Algiers": null,
        "Asmara": null,
        "Asmera": null,
        "Bamako": null,
        "Bangui": null,
        "Banjul": null,
        "Bissau": null,
        "Blantyre": null,
        "Brazzaville": null,
        "Bujumbura": null,
        "Cairo": null,
        "Casablanca": null,
        "Ceuta": null,
        "Conakry": null,
        "Dakar": null,
        "Dar_es_Salaam": null,
        "Djibouti": null,
        "Douala": null,
        "El_Aaiun": null,
        "Freetown": null,
        "Gaborone": null,
        "Harare": null,
        "Johannesburg": null,
        "Juba": null,
        "Kampala": null,
        "Khartoum": null,
        "Kigali": null,
        "Kinshasa": null,
        "Lagos": null,
        "Libreville": null,
        "Lome": null,
        "Luanda": null,
        "Lubumbashi": null,
        "Lusaka": null,
        "Malabo": null,
        "Maputo": null,
        "Maseru": null,
        "Mbabane": null,
        "Mogadishu": null,
        "Monrovia": null,
        "Nairobi": null,
        "Ndjamena": null,
        "Niamey": null,
        "Nouakchott": null,
        "Ouagadougou": null,
        "Porto-Novo": null,
        "Sao_Tome": null,
        "Timbuktu": null,
        "Tripoli": null,
        "Tunis": null,
        "Windhoek": null
    },
    "America": {
        "Adak": null,
        "Anchorage": null,
        "Anguilla": null,
        "Antigua": null,
        "Araguaina": null,
        "Argentina": {
            "Buenos_Aires": null,
            "Catamarca": null,
            "ComodRivadavia": null,
            "Cordoba": null,
            "Jujuy": null,
            "La_Rioja": null,
            "Mendoza": null,
            "Rio_Gallegos": null,
            "Salta": null,
            "San_Juan": null,
            "San_Luis": null,
            "Tucuman": null,
            "Ushuaia": null
        },
        "Aruba": null,
        "Asuncion": null,
        "Atikokan": null,
        "Atka": null,
        "Bahia": null,
        "Bahia_Banderas": null,
        "Barbados": null,
        "Belem": null,
        "Belize": null,
        "Blanc-Sablon": null,
        "Boa_Vista": null,
        "Bogota": null,
        "Boise": null,
        "Buenos_Aires": null,
        "Cambridge_Bay": null,
        "Campo_Grande": null,
        "Cancun": null,
        "Caracas": null,
        "Catamarca": null,
        "Cayenne": null,
        "Cayman": null,
        "Chicago": null,
        "Chihuahua": null,
        "Ciudad_Juarez": null,
        "Coral_Harbour": null,
        "Cordoba": null,
        "Costa_Rica": null,
        "Creston": null,
        "Cuiaba": null,
        "Curacao": null,
        "Danmarkshavn": null,
        "Dawson": null,
        "Dawson_Creek": null,
        "Denver": null,
        "Detroit": null,
        "Dominica": null,
        "Edmonton": null,
        "Eirunepe": null,
        "El_Salvador": null,
        "Ensenada": null,
        "Fort_Nelson": null,
        "Fort_Wayne": null,
        "Fortaleza": null,
        "Glace_Bay": null,
        "Godthab": null,
        "Goose_Bay": null,
        "Grand_Turk": null,
        "Grenada": null,
        "Guadeloupe": null,
        "Guatemala": null,
        "Guayaquil": null,
        "Guyana": null,
        "Halifax": null,
        "Havana": null,
        "Hermosillo": null,
        "Indiana": {
            "Indianapolis": null,
            "Knox": null,
            "Marengo": null,
            "Petersburg": null,
            "Tell_City": null,
            "Vevay": null,
            "Vincennes": null,
            "Winamac": null
        },
        "Indianapolis": null,
        "Inuvik": null,
        "Iqaluit": null,
        "Jamaica": null,
        "Jujuy": null,
        "Juneau": null,
        "Kentucky": {
            "Louisville": null,
            "Monticello": null
        },
        "Knox_IN": null,
        "Kralendijk": null,
        "La_Paz": null,
        "Lima": null,
        "Los_Angeles": null,
        "Louisville": null,
        "Lower_Princes": null,
        "Maceio": null,
        "Managua": null,
        "Manaus": null,
        "Marigot": null,
        "Martinique": null,
        "Matamoros": null,
        "Mazatlan": null,
        "Mendoza": null,
        "Menominee": null,
        "Merida": null,
        "Metlakatla": null,
        "Mexico_City": null,
        "Miquelon": null,
        "Moncton": null,
        "Monterrey": null,
        "Montevideo": null,
        "Montreal": null,
        "Montserrat": null,
        "Nassau": null,
        "New_York": null,
        "Nipigon": null,
        "Nome": null,
        "Noronha": null,
        "North_Dakota": {
            "Beulah": null,
            "Center": null,
            "New_Salem": null
        },
        "Nuuk": null,
        "Ojinaga": null,
        "Panama": null,
        "Pangnirtung": null,
        "Paramaribo": null,
        "Phoenix": null,
        "Port-au-Prince": null,
        "Port_of_Spain": null,
        "Porto_Acre": null,
        "Porto_Velho": null,
        "Puerto_Rico": null,
        "Punta_Arenas": null,
        "Rainy_River": null,
        "Rankin_Inlet": null,
        "Recife": null,
        "Regina": null,
        "Resolute": null,
        "Rio_Branco": null,
        "Rosario": null,
        "Santa_Isabel": null,
        "Santarem": null,
        "Santiago": null,
        "Santo_Domingo": null,
        "Sao_Paulo": null,
        "Scoresbysund": null,
        "Shiprock": null,
        "Sitka": null,
        "St_Barthelemy": null,
        "St_Johns": null,
        "St_Kitts": null,
        "St_Lucia": null,
        "St_Thomas": null,
        "St_Vincent": null,
        "Swift_Current": null,
        "Tegucigalpa": null,
        "Thule": null,
        "Thunder_Bay": null,
        "Tijuana": null,
        "Toronto": null,
        "Tortola": null,
        "Vancouver": null,
        "Virgin": null,
        "Whitehorse": null,
        "Winnipeg": null,
        "Yakutat": null,
        "Yellowknife": null
    },
    "Antarctica": {
        "Casey": null,
        "Davis": null,
        "DumontDUrville": null,
        "Macquarie": null,
        "Mawson": null,
        "McMurdo": null,
        "Palmer": null,
        "Rothera": null,
        "South_Pole": null,
        "Syowa": null,
        "Troll": null,
        "Vostok": null
    },
    "Arctic": {
        "Longyearbyen": null
    },
    "Asia": {
        "Aden": null,
        "Almaty": null,
        "Amman": null,
        "Anadyr": null,
        "Aqtau": null,
        "Aqtobe": null,
        "Ashgabat": null,
        "Ashkhabad": null,
        "Atyrau": null,
        "Baghdad": null,
        "Bahrain": null,
        "Baku": null,
        "Bangkok": null,
        "Barnaul": null,
        "Beirut": null,
        "Bishkek": null,
        "Brunei": null,
        "Calcutta": null,
        "Chita": null,
        "Choibalsan": null,
        "Chongqing": null,
        "Chungking": null,
        "Colombo": null,
        "Dacca": null,
        "Damascus": null,
        "Dhaka": null,
        "Dili": null,
        "Dubai": null,
        "Dushanbe": null,
        "Famagusta": null,
        "Gaza": null,
        "Harbin": null,
        "Hebron": null,
        "Ho_Chi_Minh": null,
        "Hong_Kong": null,
        "Hovd": null,
        "Irkutsk": null,
        "Istanbul": null,
        "Jakarta": null,
        "Jayapura": null,
        "Jerusalem": null,
        "Kabul": null,
        "Kamchatka": null,
        "Karachi": null,
        "Kashgar": null,
        "Kathmandu": null,
        "Katmandu": null,
        "Khandyga": null,
        "Kolkata": null,
        "Krasnoyarsk": null,
        "Kuala_Lumpur": null,
        "Kuching": null,
        "Kuwait": null,
        "Macao": null,
        "Macau": null,
        "Magadan": null,
        "Makassar": null,
        "Manila": null,
        "Muscat": null,
        "Nicosia": null,
        "Novokuznetsk": null,
        "Novosibirsk": null,
        "Omsk": null,
        "Oral": null,
        "Phnom_Penh": null,
        "Pontianak": null,
        "Pyongyang": null,
        "Qatar": null,
        "Qostanay": null,
        "Qyzylorda": null,
        "Rangoon": null,
        "Riyadh": null,
        "Saigon": null,
        "Sakhalin": null,
        "Samarkand": null,
        "Seoul": null,
        "Shanghai": null,
        "Singapore": null,
        "Srednekolymsk": null,
        "Taipei": null,
        "Tashkent": null,
        "Tbilisi": null,
        "Tehran": null,
        "Tel_Aviv": null,
        "Thimbu": null,
        "Thimphu": null,
        "Tokyo": null,
        "Tomsk": null,
        "Ujung_Pandang": null,
        "Ulaanbaatar": null,
        "Ulan_Bator": null,
        "Urumqi": null,
        "Ust-Nera": null,
        "Vientiane": null,
        "Vladivostok": null,
        "Yakutsk": null,
        "Yangon": null,
        "Yekaterinburg": null,
        "Yerevan": null
    },
    "Atlantic": {
        "Azores": null,
        "Bermuda": null,
        "Canary": null,
        "Cape_Verde": null,
        "Faeroe": null,
        "Faroe": null,
        "Jan_Mayen": null,
        "Madeira": null,
        "Reykjavik": null,
        "South_Georgia": null,
        "St_Helena": null,
        "Stanley": null
    },
    "Australia": {
        "ACT": null,
        "Adelaide": null,
        "Brisbane": null,
        "Broken_Hill": null,
        "Canberra": null,
        "Currie": null,
        "Darwin": null,
        "Eucla": null,
        "Hobart": null,
        "LHI": null,
        "Lindeman": null,
        "Lord_Howe": null,
        "Melbourne": null,
        "NSW": null,
        "North": null,
        "Perth": null,
        "Queensland": null,
        "South": null,
        "Sydney": null,
        "Tasmania": null,
        "Victoria": null,
        "West": null,
        "Yancowinna": null
    },
    "Etc": {
        "GMT": null,
        "GMT+0": null,
        "GMT+1": null,
        "GMT+10": null,
        "GMT+11": null,
        "GMT+12": null,
        "GMT+2": null,
        "GMT+3": null,
        "GMT+4": null,
        "GMT+5": null,
        "GMT+6": null,
        "GMT+7": null,
        "GMT+8": null,
        "GMT+9": null,
        "GMT-0": null,
        "GMT-1": null,
        "GMT-10": null,
        "GMT-11": null,
        "GMT-12": null,
        "GMT-13": null,
        "GMT-14": null,
        "GMT-2": null,
        "GMT-3": null,
        "GMT-4": null,
        "GMT-5": null,
        "GMT-6": null,
        "GMT-7": null,
        "GMT-8": null,
        "GMT-9": null,
        "GMT0": null,
        "Greenwich": null,
        "UCT": null,
        "UTC": null,
        "Universal": null,
        "Zulu": null
    },
    "Europe": {
        "Amsterdam": null,
        "Andorra": null,
        "Astrakhan": null,
        "Athens": null,
        "Belfast": null,
        "Belgrade": null,
        "Berlin": null,
        "Bratislava": null,
        "Brussels": null,
        "Bucharest": null,
        "Budapest": null,
        "Busingen": null,
        "Chisinau": null,
        "Copenhagen": null,
        "Dublin": null,
        "Gibraltar": null,
        "Guernsey": null,
        "Helsinki": null,
        "Isle_of_Man": null,
        "Istanbul": null,
        "Jersey": null,
        "Kaliningrad": null,
        "Kiev": null,
        "Kirov": null,
        "Kyiv": null,
        "Lisbon": null,
        "Ljubljana": null,
        "London": null,
        "Luxembourg": null,
        "Madrid": null,
        "Malta": null,
        "Mariehamn": null,
        "Minsk": null,
        "Monaco": null,
        "Moscow": null,
        "Nicosia": null,
        "Oslo": null,
        "Paris": null,
        "Podgorica": null,
        "Prague": null,
        "Riga": null,
        "Rome": null,
        "Samara": null,
        "San_Marino": null,
        "Sarajevo": null,
        "Saratov": null,
        "Simferopol": null,
        "Skopje": null,
        "Sofia": null,
        "Stockholm": null,
        "Tallinn": null,
        "Tirane": null,
        "Tiraspol": null,
        "Ulyanovsk": null,
        "Uzhgorod": null,
        "Vaduz": null,
        "Vatican": null,
        "Vienna": null,
        "Vilnius": null,
        "Volgograd": null,
        "Warsaw": null,
        "Zagreb": null,
        "Zaporozhye": null,
        "Zurich": null
    },
    "Indian": {
        "Antananarivo": null,
        "Chagos": null,
        "Christmas": null,
        "Cocos": null,
        "Comoro": null,
        "Kerguelen": null,
        "Mahe": null,
        "Maldives": null,
        "Mauritius": null,
        "Mayotte": null,
        "Reunion": null
    },
    "Pacific": {
        "Apia": null,
        "Auckland": null,
        "Bougainville": null,
        "Chatham": null,
        "Chuuk": null,
        "Easter": null,
        "Efate": null,
        "Enderbury": null,
        "Fakaofo": null,
        "Fiji": null,
        "Funafuti": null,
        "Galapagos": null,
        "Gambier": null,
        "Guadalcanal": null,
        "Guam": null,
        "Honolulu": null,
        "Johnston": null,
        "Kanton": null,
        "Kiritimati": null,
        "Kosrae": null,
        "Kwajalein": null,
        "Majuro": null,
        "Marquesas": null,
        "Midway": null,
        "Nauru": null,
        "Niue": null,
        "Norfolk": null,
        "Noumea": null,
        "Pago_Pago": null,
        "Palau": null,
        "Pitcairn": null,
        "Pohnpei": null,
        "Ponape": null,
        "Port_Moresby": null,
        "Rarotonga": null,
        "Saipan": null,
        "Samoa": null,
        "Tahiti": null,
        "Tarawa": null,
        "Tongatapu": null,
        "Truk": null,
        "Wake": null,
        "Wallis": null,
        "Yap": null
    }
};
export default timezones;
