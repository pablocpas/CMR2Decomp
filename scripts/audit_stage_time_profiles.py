#!/usr/bin/env python3
"""Record sizes and hashes of TM resources in installed high-detail track BFLs.

Read-only audit. No game assets or coefficient/time contents are copied to the
report. The archive directory follows GenericFileLoader::FindFileInArchive.
"""
import argparse
from collections import Counter
import gzip
import hashlib
import json
from pathlib import Path
import struct


def audit(root):
    profiles=[]
    archives=0
    for path in sorted(root.rglob('*')):
        if path.suffix.lower()!='.bfl' or not path.stem.lower().endswith('hi'):
            continue
        data=gzip.decompress(path.read_bytes())
        assert data[:4]==b'CMPR',path
        size=struct.unpack_from('<I',data,4)[0]
        data=data[8:]
        assert len(data)==size and size>=4,path
        end=size-4
        pos=struct.unpack_from('<I',data,end)[0]
        assert pos<=end,path
        archives+=1
        while pos<end:
            assert pos+12<=end,path
            length,offset,name_length=struct.unpack_from('<III',data,pos)
            pos+=12
            assert name_length<=260 and pos+name_length<=end,path
            name=data[pos:pos+name_length].decode('latin1').rstrip('\0')
            pos+=(name_length+3)&~3
            assert offset+length<=end,path
            if name.lower().endswith(('.tm0','.tm1','.tm2')):
                payload=data[offset:offset+length]
                profiles.append(dict(archive=path.relative_to(root).as_posix(),name=name,size=length,
                                     sha256=hashlib.sha256(payload).hexdigest()))
        assert pos==end,path
    return dict(schema_version=1,scope='High-detail track archives (*hi.bfl, case-insensitive)',
                archive_count=archives,profile_count=len(profiles),
                size_counts=dict(sorted(Counter(p['size'] for p in profiles).items())),profiles=profiles)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('tracks',type=Path,help='Installed Data/GAME/Tracks directory')
    parser.add_argument('--json',required=True,type=Path)
    args=parser.parse_args()
    if not args.tracks.is_dir():parser.error('Track directory does not exist')
    report=audit(args.tracks)
    args.json.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k!='profiles'},indent=2))


if __name__=='__main__':
    main()
