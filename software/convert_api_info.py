#!/usr/bin/env python3
import json
import re

with open("api_info.json") as f:
    content = f.readlines()

content = "".join(x for x in content if "| api_info" in x)

content = re.sub(r"^[^\|]*\|[^\|]*\| ", "", content, flags=re.MULTILINE)

content = re.sub(r",\s*\}", "}", content, flags=re.MULTILINE)
content = re.sub(r",\s*\]", "]", content, flags=re.MULTILINE)
content = content.replace(',"val":nan,', ',"val":null,')

with open("api_info.json", "w+") as f:
    f.write(content)
