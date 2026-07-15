#
# Tinkerforge Util
# Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#

import functools
import shlex
import subprocess
import queue
import threading
import collections

AppLauncherAction = collections.namedtuple('AppLauncherAction', 'name cwd cmd run_as_root shell detach')


def app_launcher(title, actions, root_pw='', big_btns=False):
    # import PyQt5 only if the function is actually use to avoid
    # importing it for nothing on system that might not have PyQt5
    from PyQt5.QtCore import QTimer
    from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton

    unblock_btn_queue = queue.Queue()

    def work(action: AppLauncherAction, btn: QPushButton):
        cmd = action.cmd if isinstance(action.cmd, list) else [action.cmd]

        for c in cmd:
            sub_cmd = shlex.split(c)

            if action.shell != 'no_shell':
                #sub_cmd = ['konsole', '-e', *sub_cmd]
                sub_cmd = ['konsole', '--hold' if action.shell == 'hold_shell' else '', '-e', *sub_cmd]

            if action.run_as_root:
                sub_cmd = ['sudo', '-S', *sub_cmd]

            if not action.detach:
                subprocess.check_output(sub_cmd, cwd=action.cwd, input=root_pw.encode('utf-8') if action.run_as_root else '')
            else:
                subprocess.Popen(sub_cmd, cwd=action.cwd)

        unblock_btn_queue.put(btn)

    def run(action: AppLauncherAction, btn: QPushButton):
        btn.setDisabled(True)
        threading.Thread(target=work, args=[action, btn], daemon=True).start()

    def unblock_btn_check():
        try:
            btn = unblock_btn_queue.get_nowait()
        except queue.Empty:
            return

        btn.setDisabled(False)

    class AppLauncher(QApplication):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)

            self.unblock_btn_timer = QTimer()
            self.unblock_btn_timer.timeout.connect(unblock_btn_check)
            self.unblock_btn_timer.setInterval(100)
            self.unblock_btn_timer.start()

    app = AppLauncher([])
    window = QWidget()
    window.setWindowTitle(title)
    window.setMinimumWidth(450)
    layout = QVBoxLayout()

    for action in actions:
        btn = QPushButton(action.name)

        if big_btns:
            btn.setMinimumHeight(btn.sizeHint().height() * 2)
            font = btn.font()
            font.setPointSizeF(font.pointSizeF() * 1.5)
            btn.setFont(font)

        btn.clicked.connect(functools.partial(run, action, btn))
        layout.addWidget(btn)

    window.setLayout(layout)
    window.show()
    app.exec_()
