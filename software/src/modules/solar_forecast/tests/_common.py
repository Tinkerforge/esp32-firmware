import math
import time

FORECAST_COUNT = 48     # 48 hourly forecast, two days
MAX_FORECAST_SLOTS = 49 # 48 + 1 for DST switch
PLANES = 6              # maximum number of planes


def minutes_since_epoch(ts=None):
    if ts is None:
        ts = time.time()
    return int(ts) // 60


def make_forecast(count=FORECAST_COUNT, base_wh=500, amplitude=400):
    """Generate a bell-curve solar forecast: Low in morning, peak at midday, low in evening."""
    forecast = []
    for i in range(count):
        hour_of_day = i % 24
        solar_curve = math.exp(-0.5 * ((hour_of_day - 12) / 3.5) ** 2)
        wh = int(base_wh * solar_curve + amplitude * solar_curve * math.sin(2 * math.pi * i / 12))
        wh = max(0, wh)
        forecast.append(wh)
    return forecast


def midnight_today_minutes():
    """Returns the start of today (midnight UTC) in minutes since epoch."""
    now = time.time()
    midnight = int(now) - int(now) % 86400
    return midnight // 60
