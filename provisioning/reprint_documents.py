#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = "==3.12.*"
# dependencies = [
#     "tinkerforge_util",
#     "pyqt5",
#     "tzlocal",
#     "pytz",
#     "setuptools",
# ]
# ///

import os
from app_launcher import app_launcher, AppLauncherAction

TEST_REPORTS_DIRECTORY = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', 'test-reports'))
MAX_LATEST_FILES = 10

def main():
    latest_files = []
    actions = []

    for root, dirs, files in os.walk(TEST_REPORTS_DIRECTORY):
        for name in files:
            if not name.endswith('.pdf'):
                continue

            pdf_path = os.path.join(root, name)

            if name.endswith('_report_stage_2.pdf'):
                json_path = pdf_path[:-len('.pdf')] + '.json'
                file_type = 'Metrel Report'
            elif name.endswith('_report_stage_2_packing_slip.pdf'):
                json_path = pdf_path[:-len('_packing_slip.pdf')] + '.json'
                file_type = 'Packing Slip'
            elif name.endswith('_report_stage_2_shipping_label.pdf'):
                json_path = pdf_path[:-len('_shipping_label.pdf')] + '.json'
                file_type = 'Shipping Label'
            else:
                json_path = None

            if json_path != None and os.path.exists(json_path):
                parts = name.split('_')
                ssid = parts[0]
                timestamp = parts[1]

                timestamp_chars = list(timestamp)
                timestamp_chars[13] = ':'
                timestamp_chars[16] = ':'
                timestamp_chars[29] = ':'

                timestamp = ''.join(timestamp_chars)

                latest_files.append((timestamp, f'{ssid} - {file_type}\n{timestamp}', pdf_path))

                if len(latest_files) > MAX_LATEST_FILES:
                    latest_files.sort(reverse=True)
                    latest_files = latest_files[:MAX_LATEST_FILES]

    for latest_file in latest_files:
        if latest_file[2].endswith('_report_stage_2_shipping_label.pdf'):
            cmd = f'sh -c "pdftops {latest_file[2]} - | lpr -P Wallbox-DHL"'
        else:
            cmd = f'sh -c "pdftops {latest_file[2]} - | lpr"'

        actions.append(AppLauncherAction(latest_file[1], '.', cmd, False, 'hold_shell', False))

    app_launcher('Reprint Documents', actions, big_btns=True)


if __name__ == '__main__':
    main()
