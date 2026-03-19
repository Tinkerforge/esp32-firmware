import math
import time

TEMP_COUNT = 48  # 48 hourly temperatures


def minutes_since_epoch(ts=None):
    if ts is None:
        ts = time.time()
    return int(ts) // 60

def make_temperatures(count=TEMP_COUNT, base_temp=100, amplitude=150):
    """Generates a sine-wave temperature curve"""
    temps = []
    for i in range(count):
        temp = int(base_temp + amplitude * math.sin(2 * math.pi * i / count))
        temps.append(temp)
    return temps

def midnight_today_minutes():
    """Returns the start of today (midnight UTC) in minutes since epoch"""
    now = time.time()
    midnight = int(now) - int(now) % 86400
    return midnight // 60
