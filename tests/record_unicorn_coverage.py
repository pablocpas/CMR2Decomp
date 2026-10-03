#!/usr/bin/env python3
"""Run a harness and record original-machine branch edges (Unicorn only).

Invoked by run_differential_suite.py --coverage-disassembly PATH. Native/Wine
execution is explicitly unmeasured. Coverage is diagnostic, not equivalence.
"""
import atexit
import hashlib
import json
from pathlib import Path
import runpy
import sys

import pefile
from unicorn import Uc, UC_HOOK_CODE


def main():
    specpath, output, harness, *arguments = sys.argv[1:]
    spec = json.loads(Path(specpath).read_text())
    original = pefile.PE(spec['original'])
    image = original.get_memory_mapped_image()
    image_sha = hashlib.sha256(image).digest()
    base = original.OPTIONAL_HEADER.ImageBase
    states = []
    init, write, start = Uc.__init__, Uc.mem_write, Uc.emu_start

    def initialise(self, *args, **kwargs):
        init(self, *args, **kwargs)
        state = {'original': None, 'pending': None, 'instructions': {}, 'edges': {}}
        self._cmr2_branch_record = state
        states.append(state)

        def record(u, address, size, data):
            if not state['original']:
                return
            if state['pending'] is not None:
                target, branch = state['pending']
                edges = state['edges'].setdefault(target, set())
                edges.add((branch, address))
                state['pending'] = None
            for target, function in spec['functions'].items():
                if function['begin'] <= address < function['end']:
                    state['instructions'].setdefault(target, set()).add(address)
                    if str(address) in function['branches']:
                        state['pending'] = target, address
                    break

        # Include callees and tail destinations so the instruction after a
        # branch is recorded even when control leaves the target function.
        if spec['functions']:
            self.hook_add(UC_HOOK_CODE, record, begin=base, end=base + len(image) - 1)

    def memory_write(self, address, data):
        write(self, address, data)
        state = self._cmr2_branch_record
        if state['original'] is None and address == base and len(data) > 4096:
            state['original'] = len(data) == len(image) and hashlib.sha256(data).digest() == image_sha

    def emulate(self, *args, **kwargs):
        self._cmr2_branch_record['pending'] = None
        return start(self, *args, **kwargs)

    Uc.__init__, Uc.mem_write, Uc.emu_start = initialise, memory_write, emulate

    def save():
        instructions, edges = {}, {}
        for state in states:
            if state['original']:
                for target, values in state['instructions'].items():
                    instructions.setdefault(target, set()).update(values)
                for target, values in state['edges'].items():
                    edges.setdefault(target, set()).update(values)
        result = {
            'scope': 'original Unicorn instruction addresses and branch destinations',
            'original_instances': sum(bool(s['original']) for s in states),
            'spec_sha256': hashlib.sha256(Path(specpath).read_bytes()).hexdigest(),
            'functions': {target: {
                'instructions': sorted(instructions.get(target, set())),
                'branch_edges': sorted(edges.get(target, set())),
            } for target in spec['functions']},
        }
        Path(output).write_text(json.dumps(result, indent=2) + '\n')

    atexit.register(save)
    sys.argv = [harness, *arguments]
    runpy.run_path(harness, run_name='__main__')


if __name__ == '__main__':
    main()
