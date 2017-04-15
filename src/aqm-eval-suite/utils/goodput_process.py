import sys
import os

scenario_name = sys.argv[1]
queuedisc_name = sys.argv[2]
fname = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-goodput.dat'
nfname = 'aqm-eval-output/'+scenario_name+"/data/new-"+queuedisc_name+'-goodput.dat'
f = open (fname ,"r")
l = f.readlines ()
l.sort ()
f.close ()
i = 0
d = []
f = []
g = []
for m in l:
  if i == len(l)-1:
    break
  i+=1
  if m.split(' ')[0] == l[i].split(' ')[0]:
    s1 = float(m.split(' ')[1])
    s2 = int((m.split(' ')[2]).replace('\n', ''))
    d.append([s1,s2])
  else:
    d.sort()
    goodput_val = 0
    goodput_count = 0
    last_recorded = 0
    for a in d:
      if a[0] - last_recorded < 0.1:
        goodput_val += a[1]
        goodput_count+=1
      else:
        last_recorded = a[0]
        if goodput_count>0:
          g.append([a[0], (goodput_val*1.0/a[0])])
    last_recorded = a[0]
    if goodput_count>0:
      g.append([a[0], (goodput_val*1.0/a[0])])
    d = []
    f.append(g)
    g = []

d.sort()
goodput_val = 0
goodput_count = 0
last_recorded = 0
for a in d:
  if a[0] - last_recorded < 0.1:
    goodput_val += a[1]
    goodput_count+=1
  else:
    last_recorded = a[0]
    if goodput_count>0:
      g.append([float("{0:.2f}".format(a[0])), (goodput_val*1.0/a[0])])
last_recorded = a[0]
if goodput_count>0:
  g.append([float("{0:.2f}".format(a[0])), (goodput_val*1.0/a[0])])
f.append(g)

st = ""

gnufile = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-gnu-goodput'
gnu = open(gnufile, "w")

gnu.write("set terminal png\n")
gnu.write("set output \"aqm-eval-output/"+scenario_name+"/graph/"+queuedisc_name+"-goodput.png\"\n set xlabel \"Time (Seconds)\" font \"Verdana\"\nset ylabel \"Goodput (Mbps)\" font \"Verdana,24\"\nset grid\nshow grid\nset key font \",18\"\n")


wfile = open (nfname ,"w")

flow = 1
for x in f:
  wfile.write("\n\n#\"flow"+str(flow)+"\"\n")
  flow+=1
  for y in x:
    wfile.write (str(y[0])+" "+str(y[1]/(1024*128))+"\n")
wfile.close()

if "RttFairness" in scenario_name:
  for j in range(len(f)):
    if j == 0:
      gnu.write("plot \""+nfname+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Fixed Rtt Flow\"")
    else:
      gnu.write(", \""+nfname+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Variable Rtt Flow\"")
else:
  for j in range(len(f)):
    if j == 0:
      gnu.write("plot \""+nfname+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Flow "+str(j+1)+"\"")
    else:
      gnu.write(", \""+nfname+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Flow "+str(j+1)+"\"")

gnu.close()
os.system("gnuplot "+gnufile)



