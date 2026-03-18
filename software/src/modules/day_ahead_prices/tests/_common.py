import math
import time

SLOTS_PER_DAY = 96
SLOTS_TOTAL = 7 * SLOTS_PER_DAY  # 672


def minutes_since_epoch(ts=None):
    if ts is None:
        ts = time.time()
    return int(ts) // 60

def make_prices(count, base_price=5000, amplitude=3000):
    """Generates a sine-wave price curve"""
    prices = []
    for i in range(count):
        price = int(base_price + amplitude * math.sin(2 * math.pi * i / count))
        prices.append(price)
    return prices

def midnight_today_minutes():
    """Returns the start of today (midnight UTC) in minutes since epoch"""
    now = time.time()
    midnight = int(now) - int(now) % 86400
    return midnight // 60
