Import("env")

env.AddCustomTarget(
    name="uploadnobuild",
    dependencies=None,
    actions=[
        "pio run -e $PIOENV -t nobuild -t upload",
    ],
    title="Uploads without building via serial"
)

env.AddCustomTarget(
    name="ota",
    dependencies="$BUILD_DIR/${PROGNAME}.elf",
    actions=[
        "pio run -e $PIOENV",
        "$PROJECT_DIR/ota-flasher.sh $PROJECT_DIR $BUILD_DIR $PROGRAM_ARGS",
    ],
    title="OTA upload",
    description="over the air upload / reflash (set OTA-IP-ADDRESS via pio option [-a 10.0.0.1])"
)

