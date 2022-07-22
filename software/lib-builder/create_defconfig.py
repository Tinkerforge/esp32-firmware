import re
import sys

def line_filter(line):
    if line.strip() == "":
        return False
    if "#" in line and not " is not set" in line:
        return False
    return True

def parse_line(line):
    m = re.search(r"CONFIG_[A-Z0-9_]*", line)
    if m is None:
        print("Failed to parse line '{}'".format(line))
        sys.exit(1)

    config = m.group(0)
    value = None if " is not set" in line else line.split("=", 1)[1]
    return config, value

def main(argv):
    if len(argv) != 3:
        print("Usage: {} sdkconfig.vanilla sdkconfig.modified".format(argv[0]))
        sys.exit(1)

    with open(argv[1]) as f:
        vanilla = [x.strip() for x in f.read().splitlines()]

    with open(argv[2]) as f:
        modified = [x.strip() for x in f.read().splitlines()]

    # Get all lines that are in left but not in right and vice-versa.
    # Yes we could use a set one-liner here, but that would lose the ordering.

    left = []
    right = []

    for line in vanilla:
        try:
            modified.remove(line)
        except:
            if line_filter(line):
                left.append(line)

    for line in modified:
        if line_filter(line) and line not in vanilla:
            right.append(line)

    left_vals = {}
    right_vals = {}

    for line in left:
        config, value = parse_line(line)
        left_vals[config] = value

    for line in right:
        config, value = parse_line(line)
        right_vals[config] = value


    diff = []

    for k, v in left_vals.items():
        if k in right_vals:
            new_v = right_vals[k]
            #print("{} {} -> {}".format(k, v, new_v))
            if new_v is None:
                diff.append("# {} is not set".format(k))
            else:
                diff.append("{}={}".format(k, new_v))
        else:
            #print("{}={} removed.".format(k, v))
            #diff.append("# {} is not set".format(k))
            pass

    for k, v in right_vals.items():
        if k in left_vals:
            continue
        else:
            #print("{}={} added".format(k, v))
            if v is None:
                diff.append("# {} is not set".format(k))
            else:
                diff.append("{}={}".format(k, v))



    #print("Removed")
    #print("\n".join(left))
    #print("Added")
    #print("\n".join(right))
    return "\n".join(diff)

if __name__ == "__main__":
    output = main(sys.argv)
    print(output)
