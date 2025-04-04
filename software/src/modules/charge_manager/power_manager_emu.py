import argparse
import json
import os
import sys
import urllib.request
import traceback
import datetime

from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox

os.system('pyuic5 -o ui_power_manager_emu.py power_manager_emu.ui')  # sudo apt install pyqt5-dev-tools

from ui_power_manager_emu import Ui_PowerManagerEmu

class SliderSpinSyncer:
    def __init__(self, slider, spin, changed_callback, commit_callback, spin_signal='editingFinished'):
        self.slider = slider
        self.spin = spin
        self.changed_callback = changed_callback
        self.commit_callback = commit_callback

        self.slider.valueChanged.connect(self.set_spinbox_from_slider_value)
        self.slider.sliderMoved.connect(self.set_spinbox_from_slider_position)
        self.slider.sliderReleased.connect(self.report_commit)
        getattr(self.spin, spin_signal).connect(self.set_slider_from_spinbox)
        getattr(self.spin, spin_signal).connect(self.report_commit)

    def set_value(self, value):
        old_state = self.slider.blockSignals(True)
        self.slider.setValue(value)
        self.slider.blockSignals(old_state)

        old_state = self.spin.blockSignals(True)
        self.spin.setValue(value)
        self.spin.blockSignals(old_state)

    def set_spinbox_from_slider_value(self):
        value = self.slider.value()
        self.spin.setValue(value)
        self.report_change(value)

    def set_spinbox_from_slider_position(self):
        self.spin.setValue(self.slider.sliderPosition())

    def set_slider_from_spinbox(self):
        value = self.spin.value()
        self.slider.setValue(value)
        self.report_change(value)

    def report_change(self, value):
        changed_callback = self.changed_callback

        if changed_callback != None:
            changed_callback(value)

    def report_commit(self):
        commit_callback = self.commit_callback

        if commit_callback != None:
            commit_callback()


class PowerManagerEmu(QMainWindow, Ui_PowerManagerEmu):
    def __init__(self, host):
        super().__init__()

        self.setupUi(self)

        if host:
            self.edit_host.setText(host)

        self.limits = [
            [0, 0, 0, 0],
            [0, 0, 0, 0],
            [0, 0, 0, 0],
            [0, 0, 0, 0],
        ]

        self.syncers = [
            [
                SliderSpinSyncer(self.slider_max_pv, self.spin_max_pv, lambda v: self.limit_changed(0, 0, v), self.commit),
                None,
                None,
                None,
            ],
            [
                SliderSpinSyncer(self.slider_raw_pv, self.spin_raw_pv, lambda v: self.limit_changed(1, 0, v), self.commit),
                SliderSpinSyncer(self.slider_raw_l1, self.spin_raw_l1, lambda v: self.limit_changed(1, 1, v), self.commit),
                SliderSpinSyncer(self.slider_raw_l2, self.spin_raw_l2, lambda v: self.limit_changed(1, 2, v), self.commit),
                SliderSpinSyncer(self.slider_raw_l3, self.spin_raw_l3, lambda v: self.limit_changed(1, 3, v), self.commit),
            ],
            [
                SliderSpinSyncer(self.slider_min_pv, self.spin_min_pv, lambda v: self.limit_changed(2, 0, v), self.commit),
                SliderSpinSyncer(self.slider_min_l1, self.spin_min_l1, lambda v: self.limit_changed(2, 1, v), self.commit),
                SliderSpinSyncer(self.slider_min_l2, self.spin_min_l2, lambda v: self.limit_changed(2, 2, v), self.commit),
                SliderSpinSyncer(self.slider_min_l3, self.spin_min_l3, lambda v: self.limit_changed(2, 3, v), self.commit),
            ],
            [
                SliderSpinSyncer(self.slider_spread_pv, self.spin_spread_pv, lambda v: self.limit_changed(3, 0, v), self.commit),
                SliderSpinSyncer(self.slider_spread_l1, self.spin_spread_l1, lambda v: self.limit_changed(3, 1, v), self.commit),
                SliderSpinSyncer(self.slider_spread_l2, self.spin_spread_l2, lambda v: self.limit_changed(3, 2, v), self.commit),
                SliderSpinSyncer(self.slider_spread_l3, self.spin_spread_l3, lambda v: self.limit_changed(3, 3, v), self.commit),
            ]
        ]

    def limit_changed(self, kind, phase, v):
        self.limits[kind][phase] = v

        for l, s in list(zip(self.limits, self.syncers))[:kind]:
            l[phase] = max(v, l[phase])
            if s[phase] is not None:
                s[phase].set_value(l[phase])

        for l, s in list(zip(self.limits, self.syncers))[kind + 1:]:
            l[phase] = min(v, l[phase])
            if s[phase] is not None:
                s[phase].set_value(l[phase])

    def commit(self):
        if len(self.edit_host.text()) == 0:
            QMessageBox.critical(self, "Host missing", "Host missing")
            return

        limits = {
            'max_pv': self.limits[0][0] * 1000,
            'raw':    [x * 1000 for x in self.limits[1]],
            'min':    [x * 1000 for x in self.limits[2]],
            'spread': [x * 1000 for x in self.limits[3]],
        }

        data = json.dumps(limits).encode("utf-8")
        req = urllib.request.Request(f"http://{self.edit_host.text()}/charge_manager/debug_limits_update",
                                        data=data,
                                        method='PUT',
                                        headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=1) as f:
                f.read()
            print(datetime.datetime.now().isoformat(), "Sent", data)
        except Exception as e:
            traceback.print_exc()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', nargs='?', default=None)

    args = parser.parse_args()

    app = QApplication(sys.argv)
    window = PowerManagerEmu(args.host)

    window.show()

    return app.exec_()


if __name__ == '__main__':
    sys.exit(main())
