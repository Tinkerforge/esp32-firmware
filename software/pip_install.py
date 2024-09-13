Import("env")

version = '1.2.1'
install = False

try:
    import tinkerforge_util

    if tinkerforge_util.__version__ != version:
        install = True
except ImportError:
    install = True

if install:
    env.Execute(f"$PYTHONEXE -m pip install tinkerforge_util=={version}")
