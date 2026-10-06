#!/usr/bin/env -S uv run --group tests --script

from dataclasses import dataclass, field
from pathlib import Path
import re
import typing
import itertools
from datetime import datetime, timedelta
import struct
import io
import csv
import json
import math
import time
from urllib.error import HTTPError, URLError
from zoneinfo import ZoneInfo

from pikepdf import Pdf
import pikepdf

from tinkerforge.ip_connection import base58encode

import tinkerforge_util as tfutil
tfutil.create_parent_module(__file__, 'software')
from software.test_runner.test_context import run_testsuite, TestContext

@dataclass
class ChargeLogEntry:
    timestamp: int
    meter_start: float
    user_id: int
    duration: int
    meter_end: float

    def pack(self):
        r = struct.pack("< I f B 3B f",
                        self.timestamp,
                        self.meter_start,
                        self.user_id,
                        *self.duration.to_bytes(3, 'little'),
                        self.meter_end)
        return r

    @classmethod
    def unpack(cls, b):
        return ChargeLogEntry(*struct.unpack("<IfBIf", b[:12] + b'\0' + b[12:]))

# The following parsers were written for the PDF layout before the redesign of the charge log PDF.
# They don't match the current layout anymore and are kept for reference only.
# See parse_pdf_v2 and the tests below for the current layout.
#
# @dataclass
# class Charge:
#     start: datetime | None
#     user: str | None
#     charged_kwh: float | None
#     duration: int
#     meter_start: float | None
#     cost: float | None
#
# @dataclass
# class Stats:
#     charger: str
#     exported_at: datetime
#     users: int
#     start: datetime | None
#     end: datetime | None
#     energy_sum_kwh: float
#
# @dataclass
# class ChargeLog:
#     stats: Stats | None
#     letterhead: str | None
#     charges: list[Charge]
#
# Language = typing.Literal['de', 'en']
#
# def verify_page_num(tc: TestContext, language: Language, page_num: int, page_count: int, texts: list[str]):
#     tc.assert_eq(1, len(texts))
#
#     if language == 'de':
#         m = re.fullmatch(r"Seite (\d+) von (\d+)", texts[0])
#     elif language == 'en':
#         m = re.fullmatch(r"Page (\d+) of (\d+)", texts[0])
#
#     tc.assert_eq(page_num + 1, int(m.group(1)))
#     tc.assert_eq(page_count, int(m.group(2)))
#
# def parse_date(tc: TestContext, language: Language, x: str, unknowns: list[str] = []):
#     if x in unknowns:
#         return None
#
#     if language == 'de':
#         date_exp = r"(?P<day>\d{2})\.(?P<month>\d{2})\.(?P<year>\d{4}) (?P<hour>\d{2}):(?P<minute>\d{2})"
#     elif language == 'en':
#         date_exp = r"(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2}) (?P<hour>\d{2}):(?P<minute>\d{2})"
#
#     m = re.fullmatch(date_exp, x)
#     tc.assert_(m)
#     return datetime(int(m.group('year')), int(m.group('month')), int(m.group('day')), int(m.group('hour')), int(m.group('minute')))
#
#
# def parse_stats(tc: TestContext, language: Language, texts: list[str]):
#     if language == 'de':
#         exps = [
#             r"Wallbox: (.*)",
#             rf"Exportiert am: (.*)",
#             r"Exportierte Benutzer: (.*)",
#             rf"Exportierter Zeitraum: (.*) bis (.*)",
#             r"Gesamtenergie exportierter Ladevorgänge:\s+(\d+,\d{3}) kWh",
#         ]
#     elif language == 'en':
#         exps = [
#             r"Charger: (.*)",
#             rf"Exported on: (.*)",
#             r"Exported users: (.*)",
#             rf"Exported period: (.*) to (.*)",
#             r"Total energy of exported charges: (\d+.\d{3}) kWh",
#         ]
#
#     tc.assert_eq(5, len(texts))
#
#     m = [re.fullmatch(e, t) for e, t in zip(exps, texts)]
#
#     s = Stats(
#         m[0].group(1),
#         parse_date(tc, language, m[1].group(1), ['unknown'] if language == 'en' else ['unbekannt']),
#         m[2].group(1),
#         parse_date(tc, language, m[3].group(1), ['record start'] if language == 'en' else ['Aufzeichnungsbeginn']),
#         parse_date(tc, language, m[3].group(2), ['record end'] if language == 'en' else ['Aufzeichnungsende', '-ende']),
#         float(m[4].group(1).replace(",", ".")),
#     )
#
#     return s
#
# def verify_header(tc: TestContext, language: Language, texts: list[str]):
#     if language == 'de':
#         tc.assert_eq([
#                 'Startzeit',
#                 'Benutzer',
#                 'geladen (kWh)',
#                 'Ladedauer',
#                 'Zählerstand Start',
#                 'Kosten (€)'
#             ],
#             texts)
#     elif language == 'en':
#         tc.assert_eq([
#                 'Start time',
#                 'User',
#                 'Charged (kWh)',
#                 'Duration',
#                 'Meter start',
#                 'Cost (€)'
#             ],
#             texts)
#     else:
#         tc.fail("Unknown language")
#
# def parse_charge(tc: TestContext, language: Language, texts: list[str]):
#     if language == 'de':
#         exps = [
#             r"(.*)",
#             r"(.*)",
#             r"(?:(\d+,\d{3})|(N/A))",
#             r"(\d+):(\d{2}):(\d{2})",
#             r"(?:(\d+,\d{3})|(N/A))",
#             r"(?:(\d+,\d{2})|(---))",
#         ]
#     elif language == 'en':
#         exps = [
#             r"(.*)",
#             r"(.*)",
#             r"(?:(\d+\.\d{3})|(N/A))",
#             r"(\d+):(\d{2}):(\d{2})",
#             r"(?:(\d+\.\d{3})|(N/A))",
#             r"(?:(\d+\.\d{2})|(---))",
#         ]
#
#     tc.assert_eq(6, len(texts))
#
#     m = [re.fullmatch(e, t) for e, t in zip(exps, texts)]
#
#     c = Charge(
#         parse_date(tc, language, texts[0], ['unknown'] if language == 'en' else ['unbekannt']),
#         texts[1],
#         float(texts[2].replace(",", ".")) if texts[2] != "N/A" else None,
#         timedelta(hours=int(m[3].group(1)),minutes=int(m[3].group(2)),seconds=int(m[3].group(3))),
#         float(texts[4].replace(",", ".")) if texts[4] != "N/A" else None,
#         float(texts[5].replace(",", ".")) if texts[5] != "---" else None,)
#
#     return c
#
# def read_pdf(tc: TestContext, language: Language, path: Path):
#     with Pdf.open(path) as pdf:
#         tc.assert_eq([], pdf.check_pdf_syntax())
#
#         cl = ChargeLog(None, None, [])
#
#         for page_num, page in enumerate(pdf.pages):
#             next_text_stream = 0
#             for stream in page["/Contents"]:
#                 s = stream.read_bytes().decode('cp1252')
#                 if not s.startswith('BT'):
#                     continue
#
#                 # Replace escaped parenthesis with characters that are not in cp1252
#                 s = s.replace("\\(", "💩").replace("\\)", "🪿")
#                 texts = [x.replace("💩", "(").replace("🪿", ")") for x in re.findall(r"\(([^\)]+)\) Tj", s)]
#
#                 if page_num == 0:
#                     match next_text_stream:
#                         case 0: verify_page_num(tc, language, page_num, len(pdf.pages), texts)
#                         case 1: parse_stats(tc, language, texts)
#                         case 2: cl.letterhead = "\n".join(texts)
#                         case 3: verify_header(tc, language, texts)
#                         case _:
#                             for batch in itertools.batched(texts, 6):
#                                 cl.charges.append(parse_charge(tc, language, batch))
#                 else:
#                     match next_text_stream:
#                         case 0: verify_page_num(tc, language, page_num, len(pdf.pages), texts)
#                         case 1: verify_header(tc, language, texts)
#                         case _:
#                             for batch in itertools.batched(texts, 6):
#                                 cl.charges.append(parse_charge(tc, language, batch))
#
#                 next_text_stream += 1
#
#         return cl


def fetch_charge_log(tc: TestContext):
    log = tc.http_request('GET', '/charge_tracker/charge_log', timeout=1)
    return [ChargeLogEntry.unpack(bytes(entry)) for entry in itertools.batched(log, 16)]

def generate_test_data(tc: TestContext):
    CHARGER_COUNT = 64

    tc.create_directory('/charge-records')

    charge_counter = 0

    charger_names = bytes()

    for uid_num in range(1, CHARGER_COUNT + 1):
        uid_str = base58encode(uid_num)
        directory = f"/charge-records/{uid_str}"

        tc.create_directory(directory)

        entries = [ChargeLogEntry(
                        timestamp=(i * 360 + uid_num * 99000) // 60,
                        meter_start=charge_counter + i,
                        user_id=i,
                        duration=i,
                        meter_end=charge_counter + i + 0.1,
                        ).pack() for i in range(256)]
        b"".join(entries)

        tc.upload_file(f"{directory}/charge-record-1.bin", b"".join(entries))

        charge_counter += len(entries)

        entries = [ChargeLogEntry(
                        timestamp=(300000000 + i * 360 + uid_num * 99000) // 60,
                        meter_start=charge_counter + i,
                        user_id=i,
                        duration=i,
                        meter_end=charge_counter + i + 0.1,
                        ).pack() for i in range(256)]
        b"".join(entries)

        charge_counter += len(entries)

        tc.upload_file(f"{directory}/charge-record-2.bin", b"".join(entries))

        charger_names += uid_num.to_bytes(4, 'little')
        charger_names += f'warp-{uid_str}'.encode('utf-8').ljust(32, b'\0')
        print(uid_num, f'warp-{uid_str}');

    tc.upload_file(f"/charge_manager/all_charger_names", charger_names.ljust(256*36, b'\0'))

# ---------------------------------------------------------------------------
# Tests for the current PDF layout
#
# The tests upload charge records via the debug file system (requires DEBUG_FS_ENABLE),
# request PDFs and compare their content to the uploaded records.
# All tracked charges of the device under test are removed!
# ---------------------------------------------------------------------------

Language = typing.Literal['de', 'en']

RECORDS_PER_FILE = 256
UNCONFIGURED_USERS = [200, 201]  # Not configured: Shown as deleted users.
EMPTY_CHARGE_THRESHOLD_KWH = 0.0015  # See CHARGE_TRACKER_EMPTY_CHARGE_THRESHOLD_KWH
CHARGER_NAMES_FILE = "/charge_manager/all_charger_names"

CHARGE_RECORDS_DIR = "/charge-records"

PDF_TIMEOUT_S = 120
MAX_CHARGES_PDF_TIMEOUT_S = 15 * 60
MAX_CHARGES_TEST_TIMEOUT_S = 30 * 60

DEFAULT_LETTERHEAD = "Max Mustermann\nMusterstraße 12\n12345 Musterstadt"

# Layout constants of pdf_charge_log.cpp that are needed to interpret the text positions.
NUMBER_COLUMN_MAX_X = 72         # Right edge of the number column (MARGIN_L + 28 pt)
LINE_OFFSET = 12.75 - 8.5        # Distance between the center line and the top/bottom line of a charge

# Fonts as referenced in the content streams (see PDF_FONT_REGULAR/PDF_FONT_BOLD in pdfgen.h)
FONT_REGULAR = '/F1'
FONT_BOLD = '/F2'

# Font sizes (see SIZE_* in pdf_charge_log.cpp)
SIZE_TILE_VALUE = 14
SIZE_SECTION = 10
SIZE_TEXT = 9
SIZE_SMALL = 7.5

COLOR_TEXT = (0x22 / 255, 0x22 / 255, 0x22 / 255)
COLOR_WHITE = (1.0, 1.0, 1.0)

# Placeholder for missing values (PDF_NO_VALUE)
NO_VALUE = '-'

# Texts of the PDF that are checked in multiple places
TEXTS: dict[str, dict[str, str]] = {
    'de': {
        'title': 'Ladelog',
        'unknown': 'unbekannt',
        'unknown_user': 'Unbekannter Benutzer',
        'deleted_user': 'Gelöschter Benutzer',
        'total': 'Summe',
        'tile_count': 'Ladevorgänge',
        'tile_energy': 'Gesamtenergie',
        'page_label': 'Seite {} von {}',
        'subtotals_users': 'Übersicht nach Benutzer',
        'subtotals_chargers': 'Übersicht nach Wallbox',
        'note_title': 'Hinweis',
        'note_empty_filtered': 'höchstens 1 Wh',
        'info_period': 'Zeitraum',
        'info_users': 'Benutzer',
    },
    'en': {
        'title': 'Charge Log',
        'unknown': 'unknown',
        'unknown_user': 'Unknown User',
        'deleted_user': 'Deleted User',
        'total': 'Total',
        'tile_count': 'Charging sessions',
        'tile_energy': 'Total energy',
        'page_label': 'Page {} of {}',
        'subtotals_users': 'Summary by user',
        'subtotals_chargers': 'Summary by charger',
        'note_title': 'Note',
        'note_empty_filtered': '1 Wh or less',
        'info_period': 'Period',
        'info_users': 'Users',
    },
}

PAGE_LABEL_RE = re.compile(r"(Seite|Page) \d+ (von|of) \d+")
SUBTOTAL_TITLES = tuple(t[k] for t in TEXTS.values() for k in ('subtotals_users', 'subtotals_chargers'))
TOTAL_LABELS = tuple(t['total'] for t in TEXTS.values())
NOTE_TITLES = tuple(t['note_title'] for t in TEXTS.values())


def f32(x: float) -> float:
    return struct.unpack("<f", struct.pack("<f", x))[0]


def entry_charged_invalid(e: ChargeLogEntry) -> bool:
    return math.isnan(e.meter_start) or math.isnan(e.meter_end) or e.meter_end < e.meter_start


def entry_charged(e: ChargeLogEntry) -> float | None:
    # The firmware subtracts the float meter values in single precision.
    return None if entry_charged_invalid(e) else f32(e.meter_end - e.meter_start)


def entry_is_empty(e: ChargeLogEntry) -> bool:
    return not entry_charged_invalid(e) and (e.meter_end - e.meter_start) < EMPTY_CHARGE_THRESHOLD_KWH


@dataclass
class ExpectedCharge:
    entry: ChargeLogEntry
    charger: str  # Display name of the charger


# --- Formatting as done by the firmware ---

def fmt_number(value: float, decimals: int, lang: Language) -> str:
    s = f"{value:.{decimals}f}"
    neg = s.startswith('-')
    if neg:
        s = s[1:]
    integer, _, frac = s.partition('.')
    groups = []
    while len(integer) > 3:
        groups.insert(0, integer[-3:])
        integer = integer[:-3]
    groups.insert(0, integer)
    thousands, decimal = (',', '.') if lang == 'en' else ('.', ',')
    result = thousands.join(groups) + (decimal + frac if frac else '')
    return ('-' if neg else '') + result


def fmt_duration(seconds: int) -> str:
    return f"{seconds // 3600}:{(seconds // 60) % 60:02}:{seconds % 60:02}"


def fmt_date_time(timestamp_min: int, tz: ZoneInfo, lang: Language) -> str:
    if timestamp_min == 0:
        return TEXTS[lang]['unknown']
    dt = datetime.fromtimestamp(timestamp_min * 60, tz)
    return dt.strftime("%Y-%m-%d %H:%M" if lang == 'en' else "%d.%m.%Y %H:%M")


def fmt_date(timestamp_min: int, tz: ZoneInfo, lang: Language) -> str:
    dt = datetime.fromtimestamp(timestamp_min * 60, tz)
    return dt.strftime("%Y-%m-%d" if lang == 'en' else "%d.%m.%Y")


def cost_cents(charged: float, price: int) -> int:
    # C round(): half away from zero
    return math.floor(charged * price / 100.0 + 0.5)


# --- PDF parsing ---

@dataclass
class Run:
    x: float
    y: float
    font: str
    size: float
    color: tuple[float, ...]
    text: str


@dataclass
class ChargeRow:
    number: int
    start: str
    end: str
    user: str
    charger: str
    duration: str
    meter_start: str
    meter_end: str
    energy: str
    cost: str | None


@dataclass
class ParsedPDF:
    docinfo: dict[str, str]
    page_count: int
    page_labels: list[str] = field(default_factory=list)
    footers: list[str] = field(default_factory=list)
    info: dict[str, str] = field(default_factory=dict)
    tiles: dict[str, str] = field(default_factory=dict)
    table_header: list[str] = field(default_factory=list)
    charges: list[ChargeRow] = field(default_factory=list)
    totals: list[str] = field(default_factory=list)
    totals_page: int = -1
    subtotal_titles: list[str] = field(default_factory=list)
    subtotal_rows: list[list[str]] = field(default_factory=list)
    note: str = ""
    frame_texts: list[str] = field(default_factory=list)


def stream_runs(stream) -> list[Run]:
    runs = []
    font, size, color, x, y = "", 0.0, (0.0, 0.0, 0.0), 0.0, 0.0

    for operands, operator in pikepdf.parse_content_stream(stream):
        op = str(operator)
        if op == 'Tf':
            font, size = str(operands[0]), float(operands[1])
        elif op == 'rg':
            color = tuple(float(o) for o in operands)
        elif op == 'BT':
            x, y = 0.0, 0.0
        elif op == 'Tm':
            x, y = float(operands[4]), float(operands[5])
        elif op == 'Td':
            # Relative to the start of the previous line
            x, y = x + float(operands[0]), y + float(operands[1])
        elif op == 'Tj':
            runs.append(Run(x, y, font, size, color, bytes(operands[0]).decode('cp1252')))

    return runs


def same_y(a: float, b: float) -> bool:
    return abs(a - b) < 0.05


def is_number_run(run: Run) -> bool:
    return run.x < NUMBER_COLUMN_MAX_X and run.text.isdigit() and run.font == FONT_REGULAR


def parse_charge_runs(tc: TestContext, runs: list[Run], show_cost: bool) -> list[ChargeRow]:
    # The cells are not drawn row by row (all texts of one color are drawn first).
    # Assign them to the charge number in the same row.
    numbers = [r for r in runs if is_number_run(r)]
    others = [r for r in runs if not is_number_run(r)]
    rows = []
    assigned = 0

    for number in numbers:
        group = [r for r in others if abs(r.y - number.y) <= LINE_OFFSET + 0.05]
        by_x = lambda line: [r.text for r in sorted(line, key=lambda r: r.x)]
        top = by_x([r for r in group if same_y(r.y, number.y + LINE_OFFSET)])
        bottom = by_x([r for r in group if same_y(r.y, number.y - LINE_OFFSET)])
        center = by_x([r for r in group if same_y(r.y, number.y)])
        tc.assert_eq(len(group), len(top) + len(bottom) + len(center))
        assigned += len(group)

        tc.assert_eq(3, len(top))
        if len(bottom) == 2:  # Empty charger names are not drawn.
            bottom.insert(1, "")
        tc.assert_eq(3, len(bottom))
        tc.assert_eq(3 if show_cost else 2, len(center))

        rows.append(ChargeRow(int(number.text), top[0], bottom[0], top[1], bottom[1], center[0],
                              top[2], bottom[2], center[1], center[2] if show_cost else None))

    # Every cell belongs to exactly one charge.
    tc.assert_eq(len(others), assigned)
    return rows


def parse_frame(result: ParsedPDF, page_num: int, runs: list[Run]):
    result.frame_texts.extend(r.text for r in runs)

    for i, run in enumerate(runs):
        if PAGE_LABEL_RE.fullmatch(run.text):
            result.page_labels.append(run.text)
            # The footer text is drawn directly before the page label.
            result.footers.append(runs[i - 1].text)

        if page_num == 0 and run.size == SIZE_TEXT and i + 1 < len(runs):
            nxt = runs[i + 1]
            if nxt.size == SIZE_TEXT and same_y(run.y, nxt.y) and run.x < nxt.x:
                result.info[run.text] = nxt.text
            elif nxt.size == SIZE_TILE_VALUE:
                result.tiles[run.text] = nxt.text

        # The table header is white bold text. The first table header (of the charges) is drawn on the first page.
        if page_num == 0 and not result.table_header and run.font == FONT_BOLD and run.color == COLOR_WHITE:
            result.table_header = [r.text for r in runs[i:] if r.font == FONT_BOLD and r.color == COLOR_WHITE and r.y >= run.y - 10]

        if run.text in TOTAL_LABELS and run.font == FONT_BOLD:
            result.totals = [r.text for r in runs if r.font == FONT_BOLD and same_y(r.y, run.y)]
            result.totals_page = page_num

        if run.size == SIZE_SECTION and run.font == FONT_BOLD and run.text in SUBTOTAL_TITLES:
            result.subtotal_titles.append(run.text)

        if run.text in NOTE_TITLES and run.font == FONT_BOLD:
            note_runs = [r for r in runs[i + 1:] if r.size == SIZE_SMALL and r.font == FONT_REGULAR and all(abs(a - b) < 0.01 for a, b in zip(r.color, COLOR_TEXT))]
            result.note = " ".join(r.text for r in note_runs)


def parse_pdf_v2(tc: TestContext, data: bytes, show_cost: bool) -> ParsedPDF:
    with pikepdf.Pdf.open(io.BytesIO(data)) as pdf:
        tc.assert_eq([], pdf.check_pdf_syntax())

        docinfo = {str(k): str(v) for k, v in pdf.docinfo.items()}
        result = ParsedPDF(docinfo, len(pdf.pages))

        for page_num, page in enumerate(pdf.pages):
            contents = page.obj["/Contents"]
            streams = list(contents) if isinstance(contents, pikepdf.Array) else [contents]

            parse_frame(result, page_num, stream_runs(streams[0]))

            for stream in streams[1:]:
                runs = stream_runs(stream)
                if len(runs) == 0:
                    continue  # Logo

                if any(is_number_run(r) for r in runs):
                    result.charges.extend(parse_charge_runs(tc, runs, show_cost))
                else:
                    # Subtotal rows: All cells of a row have the same baseline.
                    row: list[Run] = []
                    for run in runs:
                        if row and not same_y(row[0].y, run.y):
                            result.subtotal_rows.append([r.text for r in row])
                            row = []
                        row.append(run)
                    if row:
                        result.subtotal_rows.append([r.text for r in row])

        return result


# --- Device helpers ---

def wait_for_device(tc: TestContext):
    def ping():
        try:
            tc.api('info/version', timeout=1)
        except (URLError, OSError, HTTPError) as e:
            raise AssertionError(str(e))

    tc.wait_for(ping, timeout=30, poll_delay=1)


def delete_path(tc: TestContext, path: str):
    try:
        tc.http_request('DELETE', f"/debug/fs{path}", timeout=10)
    except HTTPError as e:
        if e.code != 404:
            raise


def remove_test_data(tc: TestContext):
    """Removes all charge records (also of other chargers) and the charger names file."""
    delete_path(tc, CHARGE_RECORDS_DIR)
    delete_path(tc, CHARGER_NAMES_FILE)


def upload_records(tc: TestContext, entries: list[ChargeLogEntry], directory: str = CHARGE_RECORDS_DIR):
    tc.assert_le(2 * RECORDS_PER_FILE, len(entries))

    tc.create_directory(CHARGE_RECORDS_DIR)
    if directory != CHARGE_RECORDS_DIR:
        tc.create_directory(directory)

    for i, chunk in enumerate([entries[:RECORDS_PER_FILE], entries[RECORDS_PER_FILE:]], start=1):
        if i == 1 or chunk:
            tc.upload_file(f"{directory}/charge-record-{i}.bin", b"".join(e.pack() for e in chunk))


def upload_charger_names(tc: TestContext, names: dict[int, str]):
    data = b"".join(uid.to_bytes(4, 'little') + name.encode('utf-8').ljust(32, b'\0') for uid, name in names.items())
    tc.create_directory('/charge_manager')
    tc.upload_file(CHARGER_NAMES_FILE, data.ljust(256 * 36, b'\0'))


def request_pdf(tc: TestContext, lang: Language, *, start_min=0, end_min=0, filter_empty=True, user_filter=-2, device_filter=-2,
                letterhead=DEFAULT_LETTERHEAD, timeout=PDF_TIMEOUT_S) -> bytes:
    payload = {
        "api_not_final_acked": True,
        "language": 1 if lang == 'en' else 0,
        "start_timestamp_min": start_min,
        "end_timestamp_min": end_min,
        "user_filter": user_filter,
        "device_filter": device_filter,
        "letterhead": letterhead,
        "persist_letterhead": False,
        "filter_empty_charges": filter_empty,
        "current_timestamp_min": int(time.time() // 60),
    }
    return tc.http_request('PUT', '/charge_tracker/pdf', json.dumps(payload), headers={"Content-Type": "application/json"}, timeout=timeout)


def set_electricity_price(tc: TestContext, price: int):
    config = tc.api('charge_tracker/config')
    config['electricity_price'] = price
    tc.api('charge_tracker/config_update', config)


def device_tz(tc: TestContext) -> ZoneInfo:
    return ZoneInfo(tc.api('ntp/config')['timezone'])


def local_min(tz: ZoneInfo, *args) -> int:
    return int(datetime(*args, tzinfo=tz).timestamp() // 60)


def device_display_name(tc: TestContext) -> str:
    return tc.api('info/display_name')['display_name']


def configured_users(tc: TestContext) -> dict[int, str]:
    return {u['id']: u['display_name'] for u in tc.api('users/config')['users']}


def user_name(users: dict[int, str], user_id: int, lang: Language) -> str:
    if user_id == 0 and users.get(0) == 'Anonymous':
        return TEXTS[lang]['unknown_user']
    if user_id in users:
        return users[user_id]
    return TEXTS[lang]['deleted_user']


def make_entries(tz: ZoneInfo, count: int, *, unknown_start=True, empty=True, invalid=True, meter=1208.901,
                 base=None, step=97, users=None) -> list[ChargeLogEntry]:
    """Charges in August 2026 with unique start times, chronologically ordered."""
    entries = []
    meter = f32(meter)
    users = users if users is not None else [0, *UNCONFIGURED_USERS]

    # Charges with an unknown start time are sorted first, if they are the first in the file.
    if unknown_start:
        entries.append(ChargeLogEntry(0, meter, 0, 3600, f32(meter + 5.5)))
        meter = entries[-1].meter_end

    base = base if base is not None else local_min(tz, 2026, 8, 1, 6, 0)
    for i in range(count - len(entries)):
        start = base + i * step
        duration = 600 + (i * 977) % 40000
        if empty and i % 17 == 5:
            energy = 0.0 if i % 2 else 0.001  # Empty charges
        else:
            energy = ((i * 1.731) % 23) + 0.2
        end = f32(meter + energy)
        if invalid and i == 7:
            end = float('nan')
        entries.append(ChargeLogEntry(start, meter, users[i % len(users)], duration, end))
        if not math.isnan(end):
            meter = end

    return entries


def expected(entries: list[ChargeLogEntry], charger: str, filter_empty: bool = True) -> list[ExpectedCharge]:
    return [ExpectedCharge(e, charger) for e in entries if not (filter_empty and entry_is_empty(e))]


# --- Verification ---

def verify_charges(tc: TestContext, parsed: ParsedPDF, charges: list[ExpectedCharge], tz: ZoneInfo, lang: Language,
                   price: int, users: dict[int, str], check_times: bool = True):
    tc.assert_eq(len(charges), len(parsed.charges))

    for number, (row, c) in enumerate(zip(parsed.charges, charges), start=1):
        e = c.entry
        tc.assert_eq(number, row.number)
        # The meter values identify the charge: Check them first to get a meaningful error if the order is wrong.
        tc.assert_eq(fmt_number(e.meter_start, 3, lang), row.meter_start)
        tc.assert_eq(NO_VALUE if math.isnan(e.meter_end) else fmt_number(e.meter_end, 3, lang), row.meter_end)

        if check_times:
            tc.assert_eq(fmt_date_time(e.timestamp, tz, lang), row.start)
            end_min = 0 if e.timestamp == 0 else e.timestamp + (e.duration + 30) // 60
            tc.assert_eq(fmt_date_time(end_min, tz, lang), row.end)

        tc.assert_eq(user_name(users, e.user_id, lang), row.user)
        tc.assert_eq(c.charger, row.charger)
        tc.assert_eq(fmt_duration(e.duration), row.duration)

        charged = entry_charged(e)
        tc.assert_eq(NO_VALUE if charged is None else fmt_number(charged, 3, lang), row.energy)

        if price:
            tc.assert_eq(NO_VALUE if charged is None else fmt_number(cost_cents(charged, price) / 100, 2, lang), row.cost)


def subtotal_cells(charges: list[ExpectedCharge], name: str, lang: Language, price: int) -> list[str]:
    charged = [entry_charged(c.entry) for c in charges]
    cells = [
        name,
        fmt_number(len(charges), 0, lang),
        fmt_duration(sum(c.entry.duration for c in charges)),
        fmt_number(sum(x for x in charged if x is not None), 3, lang),
    ]
    if price:
        cells.append(fmt_number(sum(cost_cents(x, price) for x in charged if x is not None) / 100, 2, lang))
    return cells


def expected_user_subtotals(charges: list[ExpectedCharge], users: dict[int, str], lang: Language, price: int) -> list[list[str]]:
    # Sorted by name, users with the same name are sorted by ID.
    ids = sorted({c.entry.user_id for c in charges}, key=lambda u: (user_name(users, u, lang).lower(), u))
    return [subtotal_cells([c for c in charges if c.entry.user_id == u], user_name(users, u, lang), lang, price) for u in ids]


def expected_charger_subtotals(charges: list[ExpectedCharge], charger_uids: dict[str, int], lang: Language, price: int) -> list[list[str]]:
    # Sorted by name, chargers with the same name are sorted by UID.
    names = sorted({c.charger for c in charges}, key=lambda n: (n.lower(), charger_uids[n]))
    return [subtotal_cells([c for c in charges if c.charger == n], n, lang, price) for n in names]


def verify_totals(tc: TestContext, parsed: ParsedPDF, charges: list[ExpectedCharge], lang: Language, price: int):
    cells = subtotal_cells(charges, TEXTS[lang]['total'], lang, price)
    # The totals row has no count.
    tc.assert_eq([cells[0]] + cells[2:], parsed.totals)
    tc.assert_ge(0, parsed.totals_page)

    tc.assert_eq(cells[1], parsed.tiles[TEXTS[lang]['tile_count']])
    tc.assert_eq(cells[3] + " kWh", parsed.tiles[TEXTS[lang]['tile_energy']])


def verify_pages(tc: TestContext, parsed: ParsedPDF, lang: Language):
    tc.assert_eq(parsed.page_count, len(parsed.page_labels))
    for i, label in enumerate(parsed.page_labels):
        tc.assert_eq(TEXTS[lang]['page_label'].format(i + 1, parsed.page_count), label)


# --- Suite ---

def suite_setup(tc: TestContext):
    if not tc.debug_fs_enabled():
        tc.skip("Firmware was built without DEBUG_FS_ENABLE")

    tc.restore_before_suite_teardown('charge_tracker/config')

    try:
        tc.api('charge_tracker/remove_all_charges', {"do_i_know_what_i_am_doing": True})
    except (URLError, OSError):
        pass  # The device reboots.

    time.sleep(5)
    wait_for_device(tc)


def suite_teardown(tc: TestContext):
    remove_test_data(tc)


def setup(tc: TestContext):
    remove_test_data(tc)
    set_electricity_price(tc, 0)


def test_content_de(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    entries = make_entries(tz, 60)
    upload_records(tc, entries)

    # The records can be read back unchanged.
    tc.assert_eq(b"".join(e.pack() for e in entries), b"".join(e.pack() for e in fetch_charge_log(tc)))

    letterhead = ["Max Mustermann", "Musterstraße 12", "12345 Musterstadt", "", "Personalnummer 4711", "Kostenstelle (0815)", "Zeile 7 wird abgeschnitten"]
    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de', letterhead="\n".join(letterhead)), show_cost=False)
    charges = expected(entries, device_display_name(tc))

    verify_pages(tc, parsed, 'de')
    verify_charges(tc, parsed, charges, tz, 'de', 0, users)
    verify_totals(tc, parsed, charges, 'de', 0)

    tc.assert_eq(['Nr.', 'Start', 'Ende', 'Benutzer', 'Wallbox', 'Ladedauer', '(h:mm:ss)', 'Start-Zählerstand', 'End-Zählerstand', 'Energie', '(kWh)'],
                 parsed.table_header)

    # Info block. Without a date filter the period starts at the first charge with a known start time.
    first_known = min(c.entry.timestamp for c in charges if c.entry.timestamp != 0)
    tc.assert_(parsed.info[TEXTS['de']['info_period']].startswith(fmt_date(first_known, tz, 'de') + " - "))
    name = tc.api('info/name')['name']
    display_name = device_display_name(tc)
    tc.assert_eq(display_name if display_name == name else f"{display_name} ({name})", parsed.info['Gerät'])
    tc.assert_eq('Alle Benutzer', parsed.info[TEXTS['de']['info_users']])
    tc.assert_(re.fullmatch(r"\d{2}\.\d{2}\.\d{4} \d{2}:\d{2}", parsed.info['Erstellt am']))
    tc.assert_eq(False, 'Strompreis' in parsed.info)
    tc.assert_('Gesamtladedauer' in parsed.tiles)

    # The letterhead is limited to 6 lines. Empty lines are not drawn.
    for line in letterhead[:6]:
        if line:
            tc.assert_in(parsed.frame_texts, line)
    tc.assert_eq(False, letterhead[6] in parsed.frame_texts)

    # Note
    tc.assert_in(parsed.note, "Enthalten sind alle Ladevorgänge, die im angegebenen Zeitraum begonnen haben.")
    tc.assert_in(parsed.note, "Bei einem Ladevorgang ist die Startzeit unbekannt")
    tc.assert_in(parsed.note, "Die Summen sind daher unvollständig.")
    tc.assert_in(parsed.note, TEXTS['de']['note_empty_filtered'])
    tc.assert_in(parsed.note, "MID-konform")

    # Subtotals per user. Only one charger: No subtotals per charger.
    tc.assert_eq([TEXTS['de']['subtotals_users']], parsed.subtotal_titles)
    tc.assert_eq(expected_user_subtotals(charges, users, 'de', 0), parsed.subtotal_rows)

    # Document properties and footer
    tc.assert_in(('WARP Ladelog', 'ELTAKO Ladelog'), parsed.docinfo.get('/Title'))
    tc.assert_in(parsed.docinfo.get('/Author', ''), name)
    tc.assert_(parsed.docinfo.get('/Subject', '').startswith(TEXTS['de']['title'] + ' '))
    tc.assert_in(parsed.docinfo.get('/Creator', ''), 'Firmware')
    for footer in parsed.footers:
        tc.assert_(footer.startswith(TEXTS['de']['title']))
        tc.assert_in(footer, 'Firmware')


def test_filter_empty_charges_off(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    entries = make_entries(tz, 60)
    upload_records(tc, entries)

    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de', filter_empty=False), show_cost=False)
    charges = expected(entries, device_display_name(tc), filter_empty=False)

    tc.assert_gt(len(expected(entries, '', filter_empty=True)), len(charges))
    verify_charges(tc, parsed, charges, tz, 'de', 0, users)
    verify_totals(tc, parsed, charges, 'de', 0)
    tc.assert_eq(False, TEXTS['de']['note_empty_filtered'] in parsed.note)


def test_english_with_cost(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    price = 3249  # 32.49 ct/kWh
    set_electricity_price(tc, price)

    entries = make_entries(tz, 40)
    upload_records(tc, entries)

    parsed = parse_pdf_v2(tc, request_pdf(tc, 'en'), show_cost=True)
    charges = expected(entries, device_display_name(tc))

    verify_pages(tc, parsed, 'en')
    verify_charges(tc, parsed, charges, tz, 'en', price, users)
    verify_totals(tc, parsed, charges, 'en', price)
    tc.assert_eq(['No.', 'Start', 'End', 'User', 'Charger', 'Duration', '(h:mm:ss)', 'Start reading', 'End reading', 'Energy', '(kWh)', 'Cost', '(€)'],
                 parsed.table_header)
    tc.assert_eq("32.49 ct/kWh", parsed.info['Electricity price'])
    tc.assert_eq('All users', parsed.info[TEXTS['en']['info_users']])
    tc.assert_('Total cost' in parsed.tiles)
    tc.assert_eq([TEXTS['en']['subtotals_users']], parsed.subtotal_titles)
    tc.assert_eq(expected_user_subtotals(charges, users, 'en', price), parsed.subtotal_rows)
    tc.assert_in(parsed.note, "The start time of one charging session is unknown")


def test_end_date_is_inclusive(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    meter = f32(100.0)
    entries = []
    for start in [local_min(tz, 2026, 8, 14, 23, 59),  # Before the period
                  local_min(tz, 2026, 8, 15, 0, 0),    # First minute of the period
                  local_min(tz, 2026, 8, 15, 12, 0),
                  local_min(tz, 2026, 8, 15, 23, 59),  # Last minute of the period
                  local_min(tz, 2026, 8, 16, 0, 0)]:   # After the period
        entries.append(ChargeLogEntry(start, meter, 0, 60, f32(meter + 1)))
        meter = entries[-1].meter_end
    upload_records(tc, entries)

    # The web interface requests the start of the next day as (exclusive) end.
    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de', start_min=local_min(tz, 2026, 8, 15), end_min=local_min(tz, 2026, 8, 16)), show_cost=False)

    verify_charges(tc, parsed, expected(entries[1:4], device_display_name(tc)), tz, 'de', 0, users)
    tc.assert_eq("15.08.2026 - 15.08.2026", parsed.info[TEXTS['de']['info_period']])
    # Only one user and one charger: No subtotals
    tc.assert_eq([], parsed.subtotal_titles)


def test_user_filter(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    tc.assert_eq(False, any(u in users for u in UNCONFIGURED_USERS))

    entries = make_entries(tz, 60)
    upload_records(tc, entries)
    charger = device_display_name(tc)

    def check(user_filter: int, include: typing.Callable[[int], bool], users_info: str):
        parsed = parse_pdf_v2(tc, request_pdf(tc, 'de', user_filter=user_filter), show_cost=False)
        charges = [c for c in expected(entries, charger) if include(c.entry.user_id)]
        tc.assert_gt(0, len(charges))
        verify_charges(tc, parsed, charges, tz, 'de', 0, users)
        verify_totals(tc, parsed, charges, 'de', 0)
        tc.assert_eq(users_info, parsed.info[TEXTS['de']['info_users']])

        user_count = len({c.entry.user_id for c in charges})
        tc.assert_eq([TEXTS['de']['subtotals_users']] if user_count > 1 else [], parsed.subtotal_titles)

    # A single (deleted) user
    check(UNCONFIGURED_USERS[0], lambda u: u == UNCONFIGURED_USERS[0], TEXTS['de']['deleted_user'])
    # Configured users
    check(-3, lambda u: u in users, 'Konfigurierte Benutzer')
    # Deleted users
    check(-1, lambda u: u not in users, 'Gelöschte Benutzer')


def test_multiple_chargers(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    local = device_display_name(tc)

    # Charges of this device and three other chargers. The start times are interleaved.
    others = {1001: "Garage links", 1002: "garage rechts", 1003: "Carport"}
    upload_charger_names(tc, others)

    base = local_min(tz, 2026, 8, 1, 6, 0)
    local_entries = make_entries(tz, 20, unknown_start=False, invalid=False, base=base)
    upload_records(tc, local_entries)

    charges = expected(local_entries, local)
    charger_uids = {local: 0}
    for k, (uid, name) in enumerate(others.items(), start=1):
        entries = make_entries(tz, 10 + k, unknown_start=False, invalid=False, meter=1000.0 * k, base=base + k * 13, step=131)
        upload_records(tc, entries, f"{CHARGE_RECORDS_DIR}/{base58encode(uid)}")
        charges += expected(entries, name)
        charger_uids[name] = uid

    charges.sort(key=lambda c: c.entry.timestamp)

    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de'), show_cost=False)
    verify_pages(tc, parsed, 'de')
    verify_charges(tc, parsed, charges, tz, 'de', 0, users)
    verify_totals(tc, parsed, charges, 'de', 0)

    tc.assert_eq([TEXTS['de']['subtotals_users'], TEXTS['de']['subtotals_chargers']], parsed.subtotal_titles)
    tc.assert_eq(expected_user_subtotals(charges, users, 'de', 0) + expected_charger_subtotals(charges, charger_uids, 'de', 0),
                 parsed.subtotal_rows)

    # The meter type is not known for other chargers.
    tc.assert_eq(False, 'Stromzähler' in parsed.info)
    tc.assert_in(parsed.note, "Die Energiemengen wurden mit MID-konformen Stromzählern gemessen.")

    # Device filter: Only the charges of one charger
    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de', device_filter=1002), show_cost=False)
    filtered = [c for c in charges if c.charger == others[1002]]
    verify_charges(tc, parsed, filtered, tz, 'de', 0, users)
    verify_totals(tc, parsed, filtered, 'de', 0)
    tc.assert_eq([TEXTS['de']['subtotals_users']], parsed.subtotal_titles)


def test_pagination(tc: TestContext):
    tz = device_tz(tc)
    users = configured_users(tc)
    entries = make_entries(tz, 300, unknown_start=False, empty=False, invalid=False)
    upload_records(tc, entries)

    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de'), show_cost=False)
    charges = expected(entries, device_display_name(tc))

    verify_pages(tc, parsed, 'de')
    verify_charges(tc, parsed, charges, tz, 'de', 0, users)
    verify_totals(tc, parsed, charges, 'de', 0)
    tc.assert_ge(9, parsed.page_count)
    tc.assert_eq(False, TEXTS['de']['unknown'] in parsed.note)


def test_no_charges(tc: TestContext):
    tz = device_tz(tc)
    upload_records(tc, make_entries(tz, 10))

    parsed = parse_pdf_v2(tc, request_pdf(tc, 'de', start_min=local_min(tz, 2030, 1, 1), end_min=local_min(tz, 2030, 2, 1)), show_cost=False)

    tc.assert_eq(1, parsed.page_count)
    tc.assert_eq([], parsed.charges)
    tc.assert_in(parsed.frame_texts, "Im ausgewählten Zeitraum wurden keine Ladevorgänge erfasst.")
    tc.assert_eq([TEXTS['de']['total'], '0:00:00', '0,000'], parsed.totals)
    tc.assert_eq("01.01.2030 - 31.01.2030", parsed.info[TEXTS['de']['info_period']])


def request_csv(tc: TestContext, lang: Language) -> list[list[str]]:
    payload = {
        "api_not_final_acked": True,
        "language": 1 if lang == 'en' else 0,
        "user_filter": -2,
        "device_filter": -2,
        "csv_delimiter": 1,  # RFC 4180: UTF-8, comma separated
        "filter_empty_charges": False,
    }
    data = tc.http_request('PUT', '/charge_tracker/csv', json.dumps(payload), headers={"Content-Type": "application/json"}, timeout=PDF_TIMEOUT_S)
    return list(csv.reader(io.StringIO(data.decode('utf-8-sig'))))


def test_csv_user_names(tc: TestContext):
    """The CSV export uses the same translated names for unknown and deleted users as the PDF."""
    tz = device_tz(tc)
    users = configured_users(tc)
    entries = make_entries(tz, 6, unknown_start=False, empty=False, invalid=False)
    upload_records(tc, entries)

    for lang in typing.get_args(Language):
        rows = request_csv(tc, lang)
        tc.assert_eq(len(entries) + 1, len(rows))  # Header + one row per charge
        tc.assert_eq([user_name(users, e.user_id, lang) for e in entries], [row[1] for row in rows[1:]])


def expected_generated_test_data() -> tuple[list[ExpectedCharge], dict[str, int]]:
    """The charges written by generate_test_data, sorted by start time."""
    charges = []
    charger_uids = {}
    charge_counter = 0

    for uid_num in range(1, 64 + 1):
        name = f'warp-{base58encode(uid_num)}'
        charger_uids[name] = uid_num

        for offset in (0, 300000000):
            for i in range(256):
                entry = ChargeLogEntry(timestamp=(offset + i * 360 + uid_num * 99000) // 60,
                                       meter_start=f32(charge_counter + i),
                                       user_id=i,
                                       duration=i,
                                       meter_end=f32(charge_counter + i + 0.1))
                charges.append(ExpectedCharge(entry, name))
            charge_counter += 256

    charges.sort(key=lambda c: c.entry.timestamp)
    return charges, charger_uids


def test_max_charges(tc: TestContext):
    if tc.device_type().is_warp(1):
        tc.skip("WARP1 only exports its own charges")

    tc.set_test_timeout(MAX_CHARGES_TEST_TIMEOUT_S)

    tz = device_tz(tc)
    users = configured_users(tc)

    # 64 chargers with 512 charges each: The maximum number of tracked charges
    generate_test_data(tc)
    charges, charger_uids = expected_generated_test_data()
    tc.assert_eq(32768, len(charges))

    start = time.monotonic()
    data = request_pdf(tc, 'de', timeout=MAX_CHARGES_PDF_TIMEOUT_S)
    print(f"Generated PDF with {len(charges)} charges ({len(data)} bytes) in {time.monotonic() - start:.1f} s")

    parsed = parse_pdf_v2(tc, data, show_cost=False)

    verify_pages(tc, parsed, 'de')
    # The test data starts in 1970. The time zone database and the device's POSIX time zone rules don't agree on that time,
    # so don't check the start and end times. The order is checked by the meter values.
    verify_charges(tc, parsed, charges, tz, 'de', 0, users, check_times=False)
    verify_totals(tc, parsed, charges, 'de', 0)
    tc.assert_ge(len(charges) // 32, parsed.page_count)

    tc.assert_eq([TEXTS['de']['subtotals_users'], TEXTS['de']['subtotals_chargers']], parsed.subtotal_titles)
    user_rows = expected_user_subtotals(charges, users, 'de', 0)
    charger_rows = expected_charger_subtotals(charges, charger_uids, 'de', 0)
    tc.assert_eq(256, len(user_rows))
    tc.assert_eq(64, len(charger_rows))
    tc.assert_eq(user_rows + charger_rows, parsed.subtotal_rows)


if __name__ == '__main__':
    run_testsuite(locals())
