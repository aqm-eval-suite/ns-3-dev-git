import sys
import os
import numpy as np

scenario_name = sys.argv[1]
queuedisc_name = sys.argv[2]
fname = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-drop.dat'
nfname = 'aqm-eval-output/'+scenario_name+"/data/new-"+queuedisc_name+'-drop.dat'
f = open (fname ,"r")
l = f.readlines ()
l.sort ()
f.close ()
i=0
d=[]
a=[]
F3 = []
for m in l:
  if i == len(l)-1:
    break
  i+=1
  if m.split(' ')[0] == l[i].split(' ')[0]:
    s1 = float(m.split(' ')[1])
    d.append(s1)
  else:
   d.sort ()
   j=1

   while j < len(d):
     a.append(d[j]-d[j-1])
     j+=1
   hist, bin_edges = np.histogram(a, normed=True, bins=10000, density=True)
   dx = bin_edges[1] - bin_edges[0]
   F1 = np.cumsum(hist)*dx
   F2 = [[0,0], [bin_edges[0], 0]]
   for k in range(len(F1)):
     F2.append([bin_edges[k+1], F1[k]])
   F3.append(F2)
   d=[]

d.sort ()
j = 1
while j < len(d):
  a.append(d[j]-d[j-1])
  j+=1
hist, bin_edges = np.histogram(a, normed=True, bins=10000, density=True)
dx = bin_edges[1] - bin_edges[0]
F1 = np.cumsum(hist)*dx
F2 = [[0,0], [bin_edges[0], 0]]
for k in range(len(F1)):
  F2.append([bin_edges[k+1], F1[k]])
F3.append(F2)
d=[]

gnufile = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-gnu-drop'
gnu = open(gnufile, "w")

gnu.write("set terminal png size 1260, 800\n")
gnu.write("set output \"aqm-eval-output/"+scenario_name+"/graph/"+queuedisc_name+"-drop.png\"\n set xlabel \"Time difference between two drops\"\nset ylabel \"CDF\"\nset grid\nshow grid\n")


wfile = open (nfname ,"w")

flow = 1
for x in F3:
  wfile.write("\n\n#\"flow"+str(flow)+"\"\n")
  flow+=1
  for y in x:
    wfile.write (str(y[0])+" "+str(y[1])+"\n")
wfile.close()

for j in range(len(F3)):
  if j == 0:
    gnu.write("plot \""+nfname+"\" i "+str(j)+" using 1:2 with lines smooth csplines  title \"Flow "+str(j+1)+"\"")
  else:
    gnu.write(", \""+nfname+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Flow "+str(j+1)+"\"")

gnu.close()
os.system("gnuplot "+gnufile)

