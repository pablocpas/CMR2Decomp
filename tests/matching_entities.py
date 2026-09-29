"""Resolve original/rebuilt symbol addresses for the differential harnesses.

Optional cached JSON files must describe the same build as the reccmp report.
"""

import argparse
import json
import os
from pathlib import Path


def load_entities(path=None):
    if path is not None:
        return json.loads(Path(path).read_text())
    from reccmp.compare import Compare
    from reccmp.project.detect import argparse_parse_project_target

    root = Path(__file__).resolve().parents[1]
    previous = Path.cwd()
    try:
        os.chdir(root)
        comparison = Compare.from_target(argparse_parse_project_target(
            argparse.Namespace(target="CMR2")))
        return {hex(e.orig_addr): [e.recomp_addr, e.name]
                for e in comparison.get_all()
                if e.orig_addr is not None and e.recomp_addr is not None}
    finally:
        os.chdir(previous)
