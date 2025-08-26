Import("env")

version = '1.2.2'
install = False

try:
    import tinkerforge_util

    if tinkerforge_util.__version__ != version:
        install = True
except ImportError:
    install = True

if install:
    has_pip = True

    try:
        import pip
    except ImportError:
        has_pip = False

    if has_pip:
        env.Execute(f"$PYTHONEXE -m pip install tinkerforge_util=={version}")
    else:
        env.Execute(f"uv pip install tinkerforge_util=={version}")
