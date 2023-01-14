Import("env")

env.AddCustomTarget(
    "uploadnobuild",
    None,
    'pio run -e %s -t nobuild -t upload' %
        env["PIOENV"],
    title="Uploads without building"
)

env.AddCustomTarget(
    "ota",
    "$BUILD_DIR/${PROGNAME}.elf",
    "ota_script --firmware-path $SOURCE"
)
