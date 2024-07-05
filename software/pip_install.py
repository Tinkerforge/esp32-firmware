Import("env")

try:
    import tinkerforge_util
except ImportError:
    env.Execute("$PYTHONEXE -m pip install tinkerforge_util")
