Import("env")

version = '1.1.0'
needs_install = False

try:
    import tinkerforge_util
except ImportError:
    needs_install = True

if tinkerforge_util.__version__ != version:
    needs_install = True

if needs_install:
    env.Execute(f"$PYTHONEXE -m pip install tinkerforge_util=={version}")
