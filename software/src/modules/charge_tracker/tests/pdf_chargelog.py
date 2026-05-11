#!/usr/bin/env -S uv run --group tests --script

from dataclasses import dataclass
from pathlib import Path
import re
import typing
import itertools
from datetime import datetime, timedelta
import struct

from pikepdf import Pdf

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
        return struct.pack("LfB", self.timestamp, self.meter_start, self.user_id) + self.duration.to_bytes(3) + struct.pack("f", self.meter_end)

    @classmethod
    def unpack(cls, b):
        return ChargeLogEntry(*struct.unpack("<IfBIf", b[:12] + bytes([0]) + b[12:]))

@dataclass
class Charge:
    start: datetime | None
    user: str | None
    charged_kwh: float | None
    duration: int
    meter_start: float | None
    cost: float | None

@dataclass
class Stats:
    charger: str
    exported_at: datetime
    users: int
    start: datetime | None
    end: datetime | None
    energy_sum_kwh: float

@dataclass
class ChargeLog:
    stats: Stats | None
    letterhead: str | None
    charges: list[Charge]

Language = typing.Literal['de', 'en']

def verify_page_num(tc: TestContext, language: Language, page_num: int, page_count: int, texts: list[str]):
    tc.assert_eq(1, len(texts))

    if language == 'de':
        m = re.fullmatch(r"Seite (\d+) von (\d+)", texts[0])
    elif language == 'en':
        m = re.fullmatch(r"Page (\d+) of (\d+)", texts[0])

    tc.assert_eq(page_num + 1, int(m.group(1)))
    tc.assert_eq(page_count, int(m.group(2)))

def parse_date(tc: TestContext, language: Language, x: str, unknowns: list[str] = []):
    if x in unknowns:
        return None

    if language == 'de':
        date_exp = r"(?P<day>\d{2})\.(?P<month>\d{2})\.(?P<year>\d{4}) (?P<hour>\d{2}):(?P<minute>\d{2})"
    elif language == 'en':
        date_exp = r"(?P<year>\d{4})-(?P<month>\d{2})-(?P<day>\d{2}) (?P<hour>\d{2}):(?P<minute>\d{2})"

    m = re.fullmatch(date_exp, x)
    tc.assert_(m)
    return datetime(int(m.group('year')), int(m.group('month')), int(m.group('day')), int(m.group('hour')), int(m.group('minute')))


def parse_stats(tc: TestContext, language: Language, texts: list[str]):
    if language == 'de':
        exps = [
            r"Wallbox: (.*)",
            rf"Exportiert am: (.*)",
            r"Exportierte Benutzer: (.*)",
            rf"Exportierter Zeitraum: (.*) bis (.*)",
            r"Gesamtenergie exportierter Ladevorgänge:\s+(\d+,\d{3}) kWh",
        ]
    elif language == 'en':
        exps = [
            r"Charger: (.*)",
            rf"Exported on: (.*)",
            r"Exported users: (.*)",
            rf"Exported period: (.*) to (.*)",
            r"Total energy of exported charges: (\d+.\d{3}) kWh",
        ]

    tc.assert_eq(5, len(texts))

    m = [re.fullmatch(e, t) for e, t in zip(exps, texts)]

    s = Stats(
        m[0].group(1),
        parse_date(tc, language, m[1].group(1), ['unknown'] if language == 'en' else ['unbekannt']),
        m[2].group(1),
        parse_date(tc, language, m[3].group(1), ['record start'] if language == 'en' else ['Aufzeichnungsbeginn']),
        parse_date(tc, language, m[3].group(2), ['record end'] if language == 'en' else ['Aufzeichnungsende', '-ende']),
        float(m[4].group(1).replace(",", ".")),
    )

    return s

def verify_header(tc: TestContext, language: Language, texts: list[str]):
    if language == 'de':
        tc.assert_eq([
                'Startzeit',
                'Benutzer',
                'geladen (kWh)',
                'Ladedauer',
                'Zählerstand Start',
                'Kosten (€)'
            ],
            texts)
    elif language == 'en':
        tc.assert_eq([
                'Start time',
                'User',
                'Charged (kWh)',
                'Duration',
                'Meter start',
                'Cost (€)'
            ],
            texts)
    else:
        tc.fail("Unknown language")

def parse_charge(tc: TestContext, language: Language, texts: list[str]):
    if language == 'de':
        exps = [
            r"(.*)",
            r"(.*)",
            r"(?:(\d+,\d{3})|(N/A))",
            r"(\d+):(\d{2}):(\d{2})",
            r"(?:(\d+,\d{3})|(N/A))",
            r"(?:(\d+,\d{2})|(---))",
        ]
    elif language == 'en':
        exps = [
            r"(.*)",
            r"(.*)",
            r"(?:(\d+\.\d{3})|(N/A))",
            r"(\d+):(\d{2}):(\d{2})",
            r"(?:(\d+\.\d{3})|(N/A))",
            r"(?:(\d+\.\d{2})|(---))",
        ]

    tc.assert_eq(6, len(texts))

    m = [re.fullmatch(e, t) for e, t in zip(exps, texts)]

    c = Charge(
        parse_date(tc, language, texts[0], ['unknown'] if language == 'en' else ['unbekannt']),
        texts[1],
        float(texts[2].replace(",", ".")) if texts[2] != "N/A" else None,
        timedelta(hours=int(m[3].group(1)),minutes=int(m[3].group(2)),seconds=int(m[3].group(3))),
        float(texts[4].replace(",", ".")) if texts[4] != "N/A" else None,
        float(texts[5].replace(",", ".")) if texts[5] != "---" else None,)

    return c

def read_pdf(tc: TestContext, language: Language, path: Path):
    with Pdf.open(path) as pdf:
        tc.assert_eq([], pdf.check_pdf_syntax())

        cl = ChargeLog(None, None, [])

        for page_num, page in enumerate(pdf.pages):
            next_text_stream = 0
            for stream in page["/Contents"]:
                s = stream.read_bytes().decode('cp1252')
                if not s.startswith('BT'):
                    continue

                # Replace escaped parenthesis with characters that are not in cp1252
                s = s.replace("\\(", "💩").replace("\\)", "🪿")
                texts = [x.replace("💩", "(").replace("🪿", ")") for x in re.findall(r"\(([^\)]+)\) Tj", s)]

                if page_num == 0:
                    match next_text_stream:
                        case 0: verify_page_num(tc, language, page_num, len(pdf.pages), texts)
                        case 1: parse_stats(tc, language, texts)
                        case 2: cl.letterhead = "\n".join(texts)
                        case 3: verify_header(tc, language, texts)
                        case _:
                            for batch in itertools.batched(texts, 6):
                                cl.charges.append(parse_charge(tc, language, batch))
                else:
                    match next_text_stream:
                        case 0: verify_page_num(tc, language, page_num, len(pdf.pages), texts)
                        case 1: verify_header(tc, language, texts)
                        case _:
                            for batch in itertools.batched(texts, 6):
                                cl.charges.append(parse_charge(tc, language, batch))

                next_text_stream += 1

        return cl

def fetch_charge_log(tc: TestContext):
    log = tc.http_request('GET', '/charge_tracker/charge_log', timeout=1)
    return [ChargeLogEntry.unpack(bytes(entry)) for entry in itertools.batched(log, 16)]

if __name__ == '__main__':
    run_testsuite(locals())
