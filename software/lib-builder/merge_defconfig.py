import re
import sys

from create_defconfig import line_filter, parse_line

def create_config_content(config):
    result = []
    for k, v in config.items():
        if k.startswith("###COMMENT"):
            result.append(v)
        elif v is None:
            result.append("# {} is not set".format(k))
        else:
            result.append("{}={}".format(k, v))
    return "\n".join(result)

def main(argv):
    if len(argv) != 4:
        print("Usage: {} defconfig.esp32vanilla defconfig.modified defconfig.to.be.written".format(argv[0]))
        sys.exit(1)

    with open(argv[1]) as f:
        vanilla = [x.strip() for x in f.read().splitlines()]

    idx = len(vanilla)
    try:
        idx = vanilla.index("# Deprecated options for backward compatibility")
    except:
        pass
    vanilla = vanilla[:idx]

    with open(argv[2]) as f:
        modified = [x.strip() for x in f.read().splitlines()]

    left_vals = {}
    right_vals = {}

    for line in vanilla:
        if not line_filter(line):
            continue

        config, value = parse_line(line)
        left_vals[config] = value

    comment_counter = 0
    for line in modified:
        if not line_filter(line):
            right_vals["###COMMENT{}".format(comment_counter)] = line
            comment_counter += 1
            continue

        config, value = parse_line(line)
        right_vals[config] = value

    for k, v in left_vals.items():
        if k in right_vals:
            continue

        right_vals[k] = v

    with open(argv[3], "w") as f:
        f.write(create_config_content(right_vals))

if __name__ == "__main__":
    output = main(sys.argv)
    print(output)
