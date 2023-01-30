Import("env")

env.AddCustomTarget(
    "uploadnobuild",
    None,
    'pio run -e %s -t nobuild -t upload' %
        env["PIOENV"],
    title="Uploads without building"
)

env.AddCustomTarget(
    name="ota",
    dependencies="$BUILD_DIR/${PROGNAME}.elf",
    actions=["$PROJECT_DIR/ota-flasher.sh $PROJECT_DIR $PIOENV $PROGRAM_ARGS"],
    title="OTA upload",
    description="over the air upload / reflash (set OTA-IP-ADDRESS via pio option [-a 10.0.0.1])"
)

