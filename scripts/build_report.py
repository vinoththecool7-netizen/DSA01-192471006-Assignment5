#!/usr/bin/env python3
"""Build the PDF and SVG diagrams from the task Markdown and captured evidence."""
from pathlib import Path
import csv,io,statistics,html,re,textwrap
from reportlab.pdfgen import canvas
from reportlab.platypus import SimpleDocTemplate,Paragraph,Spacer,PageBreak,Table,TableStyle,Preformatted,KeepTogether
from reportlab.lib.styles import getSampleStyleSheet,ParagraphStyle
from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.graphics.shapes import Drawing,Rect,String,Line,Polygon
from reportlab.graphics import renderSVG
R=Path(__file__).resolve().parents[1]
TEAL=colors.HexColor('#086a70');INK=colors.HexColor('#18303d')
def box(d,x,y,w,h,title,lines):
 d.add(Rect(x,y,w,h,fillColor=colors.HexColor('#f2f8fa'),strokeColor=TEAL,strokeWidth=1))
 d.add(String(x+w/2,y+h-17,title,fontName='Helvetica-Bold',fontSize=11,textAnchor='middle',fillColor=INK))
 for i,s in enumerate(lines):d.add(String(x+8,y+h-37-i*16,s,fontName='Helvetica',fontSize=11,fillColor=INK))
def arrow(d,x,y,xx,yy):
 d.add(Line(x,y,xx,yy,strokeColor=TEAL))
 d.add(Polygon([xx,yy,xx-5,yy-9,xx+5,yy-9],fillColor=colors.white,strokeColor=TEAL))
def diagrams():
 ds={}
 d=Drawing(500,390);box(d,70,280,360,106,'Bill <<abstract>>',['- patient : string','+ total(), type() : pure virtual, noexcept','+ ~Bill() : virtual, noexcept; print() : shared'])
 # Shared inheritance bus routes around the lower boxes, never through labels.
 d.add(Line(5,250,495,250,strokeColor=TEAL));arrow(d,250,250,250,280)
 for x,y,name,formula in [(20,150,'OutPatientBill','consultation + tests'),(260,150,'InPatientBill','days * rate + procedures'),(20,20,'DayCareBill','package price'),(260,20,'EmergencyBill','base + hours * rate')]:
  if y==150:d.add(Line(x+110,y+80,x+110,250,strokeColor=TEAL))
  else:
   edge=5 if x==20 else 495;start=x if x==20 else x+220
   d.add(Line(start,y+40,edge,y+40,strokeColor=TEAL));d.add(Line(edge,y+40,edge,250,strokeColor=TEAL))
  box(d,x,y,220,80,name,['total(), type() : override',formula]+(['+ medicines'] if name=='InPatientBill' else []))
 ds['A1/bill_hierarchy.svg']=d
 d=Drawing(500,250);box(d,95,150,310,94,'Payer <<abstract>>',['amountCovered(Bill) : pure virtual','~Payer() : virtual, noexcept'])
 for x,name,ls in [(0,'SelfPay',['full billed amount']),(170,'Insurance',['limit, copay state']),(340,'GovernmentScheme',['fixed package rates'])]:
  box(d,x,22,160,78,name,['amountCovered : override']+ls);arrow(d,x+80,100,170 if x==0 else 250 if x==170 else 330,150)
 ds['A3/payer_hierarchy.svg']=d
 d=Drawing(500,250)
 for x,name,total in [(0,'Bill table','total -> pure slot'),(170,'OutPatientBill table','total -> OP.total'),(340,'InPatientBill table','total -> IP.total')]:box(d,x,135,160,100,name,[total,'type -> virtual slot','destructor -> virtual'])
 box(d,20,5,460,85,'Conceptual dispatch of b->total()',['Bill* b -> concrete object -> vptr -> concrete table','entry for total -> derived implementation','Tables are shared; exact layout is ABI-dependent.'])
 ds['A2/vtables.svg']=d
 d=Drawing(500,400);box(d,130,325,240,64,'std::exception',['base interface; what() noexcept'])
 box(d,130,238,240,64,'std::runtime_error',['message storage']);arrow(d,250,302,250,325)
 box(d,130,151,240,64,'BillingError',['application error base']);arrow(d,250,215,250,238)
 for x,name in [(0,'InvalidCharge'),(170,'MissingPatient'),(340,'ClaimError')]:box(d,x,64,160,64,name,['specific reason']);arrow(d,x+80,128,200 if x==0 else 250 if x==170 else 300,151)
 box(d,340,0,160,47,'UnknownPackage',[]);arrow(d,420,47,420,64)
 ds['A4/exceptions.svg']=d
 for p,d in ds.items():renderSVG.drawToFile(d,str(R/p))
 return ds
def timing():
 path=R/'B3/evidence/b3_benchmark.stdout.txt'
 if not path.exists():return
 (R/'B3/timings.csv').write_text(path.read_text())
 rows=list(csv.DictReader(io.StringIO(path.read_text())));groups={}
 for row in rows:groups.setdefault((row['method'],int(row['n'])),[]).append(float(row['ms']))
 text='### Median traversal time (milliseconds; 3 repetitions)\n\n| Bills | Virtual | Switch | Variant |\n| --- | --- | --- | --- |\n'
 for n in [1000,1000000,10000000]:text+=f'| {n:,} | '+ ' | '.join(f'{statistics.median(groups[m,n]):.6f}' for m in ['virtual','switch','variant'])+' |\n'
 text+='\n### Median validation time (1,000,000 inputs; milliseconds)\n\n| Invalid | Status | Exceptions | Optional |\n| --- | --- | --- | --- |\n'
 for rate in [0,1,50]:text+=f'| {rate}% | '+' | '.join(f'{statistics.median(groups[m+"_"+str(rate),1000000]):.6f}' for m in ['status','exception','optional'])+' |\n'
 text+=f'\nFor 10,000,000 bills, the observed medians were {statistics.median(groups["virtual",10000000]):.3f} ms virtual, {statistics.median(groups["switch",10000000]):.3f} ms switch and {statistics.median(groups["variant",10000000]):.3f} ms variant. Here the contiguous variant was fastest, with differences that include locality and allocation layout as well as dispatch. At 1% invalid inputs, exceptions took {statistics.median(groups["exception_1",1000000]):.3f} ms compared with {statistics.median(groups["status_1",1000000]):.3f} ms for status. At 50%, repeated unwinding dominates. At 0%, the measured exception success path is close to the explicit-result paths.\n'
 text+='\nObserved sizes in bytes: Base 8, OP 24, IP 32, DC 16, ER 32; owning pointer 8; switch 32; variant 32; optional<double> 16; status int 4. A virtual bill in the measured container therefore requests object bytes plus an 8-byte list slot; allocation overhead is extra. These values are platform-specific.\n'
 p=R/'B3/README.md';s=p.read_text();
 if 'MEASUREMENTS_PLACEHOLDER' in s:s=s.replace('MEASUREMENTS_PLACEHOLDER',text)
 else:s=re.sub(r'### Median traversal time.*?(?=## Interpretation)',text+'\n',s,flags=re.S)
 p.write_text(s)
 (R/'B3/timing_summary.md').write_text(text)
styles=getSampleStyleSheet()
styles.add(ParagraphStyle(name='Body11',fontName='Helvetica',fontSize=11,leading=15,spaceAfter=9,textColor=INK))
styles.add(ParagraphStyle(name='H1x',keepWithNext=True,fontName='Helvetica-Bold',fontSize=21,leading=26,spaceAfter=18,textColor=TEAL))
styles.add(ParagraphStyle(name='H2x',keepWithNext=True,fontName='Helvetica-Bold',fontSize=13,leading=18,spaceBefore=10,spaceAfter=9,textColor=TEAL))
styles.add(ParagraphStyle(name='Cell11',fontName='Helvetica',fontSize=11,leading=14,textColor=INK))
def para(s,style='Body11'):return Paragraph(html.escape(s).replace('`',''),styles[style])
def md(text,ds):
 lines=text.splitlines();story=[];i=0
 while i<len(lines):
  line=lines[i]
  if not line.strip():i+=1;continue
  if line.startswith('```'):
   buf=[];i+=1
   while i<len(lines) and not lines[i].startswith('```'):
    buf+=textwrap.wrap(lines[i],width=70,subsequent_indent='        ',replace_whitespace=False,drop_whitespace=False) or [''];i+=1
   block=[Preformatted('\n'.join(buf),ParagraphStyle('code',fontName='Courier',fontSize=11,leading=14,spaceAfter=12))]
   if story and isinstance(story[-1],Paragraph) and story[-1].style.name=='H2x':block.insert(0,story.pop())
   story.append(KeepTogether(block));i+=1;continue
  if line.startswith('!['):
   path=re.search(r'\]\((.*?)\)',line).group(1);key=next(k for k in ds if k.endswith('/'+path));story.append(ds[key]);story.append(Spacer(1,12));i+=1;continue
  if line.startswith('|'):
   rows=[]
   while i<len(lines) and lines[i].startswith('|'):
    cells=[x.strip() for x in lines[i].strip('|').split('|')]
    if not all(re.match(r'^[-: ]+$',x) for x in cells):rows.append([para(x,'Cell11') for x in cells])
    i+=1
   cols=len(rows[0]);widths=([190]+[77.5]*4) if cols==5 else [500/cols]*cols
   t=Table(rows,colWidths=widths,repeatRows=1,hAlign='LEFT');t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#e7f2f4')),('VALIGN',(0,0),(-1,-1),'TOP'),('GRID',(0,0),(-1,-1),.4,colors.HexColor('#bdd5db')),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7)]));story+=[t,Spacer(1,12)];continue
  if line.startswith('# '):story.append(para(line[2:],'H1x'));i+=1;continue
  if line.startswith('##'):story.append(para(line.lstrip('# '),'H2x'));i+=1;continue
  buf=[line];i+=1
  while i<len(lines) and lines[i].strip() and not lines[i].startswith(('#','```','|','![')):buf.append(lines[i]);i+=1
  story.append(para(' '.join(buf)))
 return story
def footer(c,doc):
 c.setStrokeColor(TEAL);c.line(47,45,548,45);c.setFont('Helvetica',11);c.setFillColor(INK);c.drawString(47,28,'S Vinoth Kumar | 192471006 | Assignment 5');c.drawRightString(548,28,str(doc.page))
def main():
 ds=diagrams();timing();story=[Spacer(1,65),para('ASSIGNMENT 5 - CO5','H1x'),para('Reliable Software with Runtime Polymorphism and Exceptions','H1x'),Spacer(1,22),para('S Vinoth Kumar'),para('Register number: 192471006'),para('Hospital Billing and Insurance Claims'),Spacer(1,20),para('Repository: vinoththecool7-netizen / DSA01-192471006-Assignment5'),Paragraph('<link href="https://github.com/vinoththecool7-netizen/DSA01-192471006-Assignment5" color="#086a70">Open the GitHub repository</link>',styles['Body11']),Spacer(1,20),para('Part A - Design: 25 marks'),para('Part B - Implementation and Experimental Investigation: 25 marks'),para('6 November 2026'),PageBreak()]
 story+=md('''# Execution and report guide
The report follows A1-A5 and B1-B4. Task folders contain the actual programs, predictions and evidence. The original assignment text is retained in reference/specification.txt. All inputs are embedded fixtures, except expected.txt files which the verifier reads for exact output comparisons. There are no dummy input files.

The g++ command on this machine is Apple clang version 21.0.0 (clang-2100.4.1.1), targeting arm64-apple-darwin27.2.0, rather than GNU GCC. Correctness tests use -std=c++17 -Wall -Wextra -fsanitize=address -g -fno-omit-frame-pointer. B3 timed runs use -std=c++17 -Wall -Wextra -O2, and the 10-item B3 check also runs with AddressSanitizer. Exact commands, source hashes, return codes and stdout/stderr are retained in evidence JSON and text logs.

Predictions were written in this work session before the corresponding programs were first run. The pure-virtual and destructor compile failures are intentional and isolated from the successful builds. The non-virtual deletion experiment is undefined behaviour and is not part of the clean sanitizer result.

The pseudocode uses explicit INPUT/OUTPUT, assignment, indentation and paired BEGIN/END blocks. No external pseudocode handout is assumed. The bill rates and payer rules are declared test assumptions. Figures are conceptual UML and dispatch diagrams, not raw ABI memory dumps.

Output images are screenshots of the recorded-evidence viewer. They present genuine captured output and are labelled accordingly; raw machine-readable logs remain the primary execution evidence. Timing tables show medians of three local repetitions with all raw values available.
''',ds)
 for task in ['A1','A2','A3','A4','A5','B1','B2','B3','B4']:
  story.append(PageBreak());story+=md((R/task/'README.md').read_text(),ds)
  # Short excerpt preserves the exact actual output in report, while raw diagnostics stay in evidence files.
  name={'A2':'a2_predict','A5':'a5_predict','B1':'b1_complete','B2':'b2_complete','B4':'b4_extension'}.get(task)
  if name:
   story.append(para('Observed output excerpt','H2x'));story.append(Preformatted((R/task/'evidence'/(name+'.stdout.txt')).read_text(),ParagraphStyle('out',fontName='Courier',fontSize=11,leading=14)))
 report=R/'Assignment5_Report.pdf';doc=SimpleDocTemplate(str(report),pagesize=(595.28,841.89),rightMargin=47,leftMargin=47,topMargin=48,bottomMargin=60,title='Assignment 5 - S Vinoth Kumar - 192471006',author='S Vinoth Kumar');doc.build(story,onFirstPage=footer,onLaterPages=footer)
 (R/'Assignment5_Report.md').write_text('# Assignment 5 - CO5\n\nS Vinoth Kumar | 192471006\n\nhttps://github.com/vinoththecool7-netizen/DSA01-192471006-Assignment5\n\n'+'\n\n'.join(re.sub(r'(!\[[^\]]*\]\()([^)]*)(\))',lambda m:m.group(1)+t+'/'+m.group(2)+m.group(3),(R/t/'README.md').read_text()) for t in ['A1','A2','A3','A4','A5','B1','B2','B3','B4']))
 print(report.name)
if __name__=='__main__':main()
