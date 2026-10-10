#!/usr/bin/env python3
"""Keep an improved byte score from hiding a worse fuzzy score in the gate."""
import argparse
import contextlib
import io
from pathlib import Path
import sys
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
import match


class Gate(unittest.TestCase):
    def report(self,old,new):
        output=io.StringIO()
        with contextlib.redirect_stdout(output):
            failures=match.report(Path('example.cpp'),{0x401000:new},
                                  {'0x401000':dict(n='Example',**old)},[],
                                  argparse.Namespace(all=False))
        return failures,output.getvalue()

    def test_normal_improvement_cannot_hide_fuzzy_regression(self):
        failures,output=self.report(dict(s=.25,fz=.74,x=False),dict(s=.28,fz=.69,x=False))
        self.assertGreater(failures,0)
        self.assertIn('worse fz',output)

    def test_unchanged_and_improved_scores_pass(self):
        for score,fuzzy in ((.25,.74),(.28,.75),(1.,1.)):
            failures,_=self.report(dict(s=.25,fz=.74,x=False),
                                   dict(s=score,fz=fuzzy,x=score==1.))
            self.assertEqual(failures,0)

    def test_loss_of_exactness_and_normal_score_still_fail(self):
        for old,new in ((dict(s=1.,fz=1.,x=True),dict(s=.9,fz=1.,x=False)),
                        (dict(s=.25,fz=.74,x=False),dict(s=.24,fz=.75,x=False))):
            failures,_=self.report(old,new)
            self.assertGreater(failures,0)


if __name__=='__main__':unittest.main()
