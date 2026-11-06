// Screenshots of a browser evidence viewer; never a recreated terminal.
const {chromium}=require('playwright');
const fs=require('fs');const path=require('path');
const root=path.resolve(__dirname,'..');
const escape=s=>s.replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;');
(async()=>{const b=await chromium.launch({channel:'chrome',headless:true});
for(const task of ['A1','A2','A3','A4','A5','B1','B2','B3','B4']){
 const dir=path.join(root,task,'evidence');fs.mkdirSync(dir,{recursive:true});
 const diagrams={A1:'bill_hierarchy.svg',A2:'vtables.svg',A3:'payer_hierarchy.svg',A4:'exceptions.svg'};
 let body='';if(diagrams[task])body+=fs.readFileSync(path.join(root,task,diagrams[task]),'utf8');
 const runs=fs.readdirSync(dir).filter(x=>x.endsWith('.run.txt'));
 for(const f of runs){let s=fs.readFileSync(path.join(dir,f),'utf8');if(task==='B3'&&s.length>1800)s=s.slice(0,1800)+'\n[Further repetitions retained in raw log]';body+=`<h2>${escape(f)}</h2><pre>${escape(s)}</pre>`;}
 if(!runs.length)body+='<p>Design task. No executable is required; see the task README for algorithms and reasoning.</p>';
 let doc=`<!doctype html><html><head><meta charset="utf-8"><style>body{font:17px Arial;color:#18303d;background:#edf3f5;padding:32px}main{background:white;padding:32px;border-top:6px solid #086a70;max-width:1080px;margin:auto}h1{color:#086a70}h2{font-size:18px;margin-top:28px}p{line-height:1.5}pre{font:15px monospace;white-space:pre-wrap;overflow-wrap:anywhere;background:#f4f7f8;padding:18px}svg{display:block;width:100%;height:auto;max-height:500px}small{color:#46616e}</style></head><body><main><h1>${task} - Recorded evidence viewer</h1><p>S Vinoth Kumar | 192471006</p><small>Genuine captured execution logs and conceptual design figures. This viewer is not an original terminal capture.</small>${body}</main></body></html>`;
 fs.writeFileSync(path.join(dir,'viewer.html'),doc);const page=await b.newPage({viewport:{width:1200,height:900},deviceScaleFactor:1});await page.goto('file://'+path.join(dir,'viewer.html'));await page.screenshot({path:path.join(dir,'output_screenshot.png'),fullPage:true});await page.close();console.log(task+' captured');}
 await b.close();})().catch(e=>{console.error(e);process.exit(1)});
