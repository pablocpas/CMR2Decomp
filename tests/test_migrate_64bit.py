#!/usr/bin/env python3
"""Migration isolation, shared-file edits, dependencies and rollback contracts."""
import importlib.util,json,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('migrations',ROOT/'scripts/migrate_64bit.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

class MigrationTests(unittest.TestCase):
 def family(self,name,old,new,requires=()):
  return {'id':name,'classification':'pointer','evidence':'synthetic verified pointer transport','requires':list(requires),'rules':[{'paths':['source.cpp'],'find':old,'replace':new,'count':1}]}
 def gate(self,root,code):
  path=root/'gate.py';path.write_text(code);return [sys.executable,str(path)]
 def test_large_batch_isolates_failure_in_shared_file(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);(root/'source.cpp').write_text('alpha beta gamma delta\n')
   recipe={'families':[self.family('a','alpha','ALPHA'),self.family('bad','beta','BROKEN'),self.family('c','gamma','GAMMA'),self.family('d','delta','DELTA')]}
   plan=m.generate(root,recipe);gate=self.gate(root,"from pathlib import Path\nimport sys\nsys.exit('BROKEN' in Path('source.cpp').read_text())\n")
   report=m.apply_plan(root,plan,gate,root/'logs')
   self.assertEqual(report['accepted'],['a','c','d']);self.assertEqual(report['parked_dependency_groups'],[['bad']]);self.assertEqual((root/'source.cpp').read_text(),'ALPHA beta GAMMA DELTA\n')
   self.assertFalse(report['probes'][1]['passed']);self.assertTrue(any(p['families']==['a','c','d'] and p['passed'] for p in report['probes']))
 def test_dependency_group_is_parked_together(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);(root/'source.cpp').write_text('alpha beta gamma\n')
   recipe={'families':[self.family('header','alpha','HEADER'),self.family('consumer','beta','BAD_USE',('header',)),self.family('other','gamma','GOOD')]}
   report=m.apply_plan(root,m.generate(root,recipe),self.gate(root,"from pathlib import Path\nimport sys\ns=Path('source.cpp').read_text()\nsys.exit(('HEADER' in s)!=('BAD_USE' in s) or 'BAD_USE' in s)\n"),root/'logs')
   self.assertEqual(report['accepted'],['other']);self.assertEqual(report['parked_dependency_groups'],[['consumer','header']]);self.assertEqual((root/'source.cpp').read_text(),'alpha beta GOOD\n')
 def test_stale_plan_never_overwrites_user_edit(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);p=root/'source.cpp';p.write_text('alpha\n');plan=m.generate(root,{'families':[self.family('a','alpha','ALPHA')]});p.write_text('user edit\n')
   with self.assertRaisesRegex(ValueError,'Input changed'):m.apply_plan(root,plan,[],root/'logs')
   self.assertEqual(p.read_text(),'user edit\n')
 def test_loaded_plan_rejects_unknown_dependencies_before_writing(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);p=root/'source.cpp';p.write_text('alpha\n');plan=m.generate(root,{'families':[self.family('a','alpha','ALPHA')]})
   plan['families'][0]['requires']=['missing']
   with self.assertRaisesRegex(ValueError,'Dependencies must precede'):m.apply_plan(root,plan,[],root/'logs')
   self.assertEqual(p.read_text(),'alpha\n')
 def test_baseline_failure_rolls_back_without_applying_family(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);p=root/'source.cpp';p.write_text('alpha\n');plan=m.generate(root,{'families':[self.family('a','alpha','ALPHA')]})
   with self.assertRaisesRegex(RuntimeError,'Baseline gate fails'):m.apply_plan(root,plan,self.gate(root,'raise SystemExit(1)\n'),root/'logs')
   self.assertEqual(p.read_text(),'alpha\n')
 def test_concurrent_edit_is_preserved(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);p=root/'source.cpp';p.write_text('alpha\n');plan=m.generate(root,{'families':[self.family('a','alpha','ALPHA')]})
   gate=self.gate(root,"from pathlib import Path\np=Path('source.cpp')\nif 'ALPHA' in p.read_text():p.write_text('concurrent edit\\n')\n")
   with self.assertRaisesRegex(RuntimeError,'Concurrent source edit'):m.apply_plan(root,plan,gate,root/'logs')
   self.assertEqual(p.read_text(),'concurrent edit\n')
 def test_metric_regressions_include_each_score_and_missing_functions(self):
  self.assertEqual(len(m.score_regressions({'a':{'s':1,'fz':1,'x':True},'b':{'s':1}}, {'a':{'s':.9,'fz':.95,'x':False}})),4)
 def test_probe_cannot_delete_baseline_metrics(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);p=root/'source.cpp';p.write_text('alpha\n');metrics=root/'CMR2PROGRESS/bytes.json';metrics.parent.mkdir();metrics.write_text('{"a":{"s":1,"fz":1,"x":true}}\n');before=metrics.read_bytes()
   plan=m.generate(root,{'families':[self.family('a','alpha','ALPHA')]})
   gate=self.gate(root,"from pathlib import Path\nif 'ALPHA' in Path('source.cpp').read_text():Path('CMR2PROGRESS/bytes.json').unlink()\n")
   with self.assertRaisesRegex(RuntimeError,'rewrote baseline metrics'):m.apply_plan(root,plan,gate,root/'logs')
   self.assertEqual(metrics.read_bytes(),before);self.assertEqual(p.read_text(),'alpha\n')
 def test_exact_match_counts_and_path_boundary(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);(root/'source.cpp').write_text('alpha alpha\n')
   with self.assertRaisesRegex(ValueError,'expected 1 matches'):m.generate(root,{'families':[self.family('a','alpha','ALPHA')]})
   with self.assertRaisesRegex(ValueError,'outside source'):m.safe_path(root,'../outside.cpp')
 def test_new_header_is_removed_if_its_family_is_parked(self):
  with tempfile.TemporaryDirectory() as directory:
   root=Path(directory);(root/'source.cpp').write_text('alpha\n');family=self.family('a','alpha','ALPHA');family['rules'].insert(0,{'create':'new.h','content':'native pointer header\n','count':1})
   report=m.apply_plan(root,m.generate(root,{'families':[family]}),self.gate(root,"from pathlib import Path\nraise SystemExit(Path('new.h').exists())\n"),root/'logs')
   self.assertEqual(report['accepted'],[]);self.assertFalse((root/'new.h').exists());self.assertEqual((root/'source.cpp').read_text(),'alpha\n')

if __name__=='__main__':unittest.main()
