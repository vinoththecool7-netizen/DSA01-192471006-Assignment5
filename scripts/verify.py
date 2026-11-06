#!/usr/bin/env python3
"""Compile, execute, compare; publish relative logs with source digests."""
import subprocess,pathlib,hashlib,json,datetime,os,sys
R=pathlib.Path(__file__).resolve().parents[1];os.chdir(R)
(R/'build').mkdir(exist_ok=True)
compiler=subprocess.check_output(['g++','--version'],text=True)
flags=['-std=c++17','-Wall','-Wextra','-fsanitize=address','-g','-fno-omit-frame-pointer']
manifest=[]
def clean(text):return text.replace(str(R),'<project>').replace(str(pathlib.Path.home()),'<home>')
def run(name,src,expected=None,invalid=False,unsafe=False,opt=False,args=None):
 task=src.split('/')[0];out=R/task/'evidence';out.mkdir(exist_ok=True)
 binary='build/'+name
 cmd=['g++']+(['-std=c++17','-Wall','-Wextra','-O2'] if opt else flags)
 if src.endswith('.txt'):cmd+=['-x','c++']
 cmd += [src,'-o',binary]
 c=subprocess.run(cmd,capture_output=True,text=True)
 entry={'name':name,'source':src,'source_sha256':hashlib.sha256(pathlib.Path(src).read_bytes()).hexdigest(),'command':cmd,'compile_exit':c.returncode,'compiler':compiler,'input':'embedded fixtures; no stdin','utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'path_export':'project and home prefixes only'}
 (out/(name+'.compile.txt')).write_text('$ '+' '.join(cmd)+'\n'+clean(c.stdout+c.stderr)+'exit='+str(c.returncode)+'\n')
 if invalid:
  assert c.returncode!=0
  assert 'override' in c.stderr or 'virtual' in c.stderr,c.stderr
 else:
  assert c.returncode==0,c.stderr
  p=subprocess.run([binary]+(args or []),capture_output=True,text=True)
  entry.update(run_exit=p.returncode,stdout=p.stdout,stderr=clean(p.stderr))
  (out/(name+'.stdout.txt')).write_text(p.stdout)
  (out/(name+'.stderr.txt')).write_text(clean(p.stderr))
  (out/(name+'.run.txt')).write_text('$ '+binary+'\n'+p.stdout+clean(p.stderr)+'exit='+str(p.returncode)+'\n')
  if not unsafe:assert p.returncode==0,(name,p.stderr)
  if expected is not None:assert p.stdout==expected,(name,repr(p.stdout),repr(expected))
 (out/(name+'.json')).write_text(json.dumps(entry,indent=2)+'\n');manifest.append(entry)
 print(name,'PASS' if not unsafe else 'OBSERVED (undefined behaviour)')
 return entry
which=sys.argv[1] if len(sys.argv)>1 else 'all'
def want(t):return which in ('all',t)
if want('A2'):
 run('a2_predict','A2/predict.cpp',json.loads((R/'A2/prediction.json').read_text())['prediction'])
 run('a2_invalid_destructor','A2/nonvirtual_destructor.cpp.txt',invalid=True)
 run('a2_unsafe','A2/unsafe_delete.cpp',unsafe=True)
if want('A5'):run('a5_predict','A5/predict.cpp',json.loads((R/'A5/prediction.json').read_text())['prediction'])
if want('B1'):
 run('b1_complete','B1/complete.cpp',(R/'B1/expected.txt').read_text())
 run('b1_tests','B1/main.cpp')
 run('b1_nonvirtual_pure','B1/nonvirtual_pure.cpp.txt',invalid=True)
 run('b1_early','B1/early_binding.cpp','0 0 Total 0\n')
if want('B2'):
 run('b2_predict','B2/predict.cpp',json.loads((R/'B2/prediction.json').read_text())['prediction'])
 run('b2_complete','B2/complete.cpp',(R/'B2/expected.txt').read_text())
 for name in ['errors','allocation','slicing']:run('b2_'+name,'B2/'+name+'.cpp')
if want('B3'):
 run('b3_benchmark','B3/benchmark.cpp',opt=True)
 run('b3_small_asan','B3/benchmark.cpp',args=['--check'])
if want('B4'):run('b4_extension','B4/main.cpp')
