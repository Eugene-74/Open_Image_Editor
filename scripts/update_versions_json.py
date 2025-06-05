import os
import re
import json

base_dir = os.path.dirname(os.path.abspath(__file__))

if os.path.basename(base_dir) == "scripts":
    base_dir = os.path.dirname(base_dir)

version_pattern = re.compile(r"^\d+\.\d+\.\d+$")

folders = [name for name in os.listdir(base_dir) if os.path.isdir(os.path.join(base_dir, name))]
versions = sorted([f for f in folders if version_pattern.match(f)], reverse=True)

with open(os.path.join(base_dir, "versions.json"), "w", encoding="utf-8") as f:
    json.dump(versions, f, indent=2, ensure_ascii=False)

