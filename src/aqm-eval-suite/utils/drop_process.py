import sys
import os
import numpy as np

scenario_name = sys.argv[1]
queuedisc_name = sys.argv[2]
file_name = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-drop.dat'
new_file_name = 'aqm-eval-output/'+scenario_name+"/data/new-"+queuedisc_name+'-drop.dat'
File = open (file_name ,"r")
lines_read = File.readlines ()
lines_read.sort ()
File.close ()
i=0
data=[]
inst_data=[]
bucket = []
for file_iterator in lines_read:
  if i == len(lines_read)-1:
    break
  i+=1
  if file_iterator.split(' ')[0] == lines_read[i].split(' ')[0]:
    temp_string = float(file_iterator.split(' ')[1])
    data.append(temp_string)
  else:
   data.sort ()
   data_iterator=1

   while data_iterator < len(data):
     inst_data.append(data[data_iterator]-data[data_iterator-1])
     data_iterator+=1
   hist, bin_edges = np.histogram(inst_data, normed=True, bins=10000, density=True)
   dx = bin_edges[1] - bin_edges[0]
   Function_uno = np.cumsum(hist)*dx
   Function_duo = [[0,0], [bin_edges[0], 0]]
   for k in range(len(Function_uno)):
     Function_duo.append([bin_edges[k+1], Function_uno[k]])
   bucket.append(Function_duo)
   data=[]

data.sort ()
data_iterator = 1
while data_iterator < len(data):
  inst_data.append(data[data_iterator]-data[data_iterator-1])
  data_iterator+=1
hist, bin_edges = np.histogram(inst_data, normed=True, bins=10000, density=True)
dx = bin_edges[1] - bin_edges[0]
Function_uno = np.cumsum(hist)*dx
Function_duo = [[0,0], [bin_edges[0], 0]]
for k in range(len(Function_uno)):
  Function_duo.append([bin_edges[k+1], Function_uno[k]])
bucket.append(Function_duo)
data=[]

gnufile = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-gnu-drop'
gnu = open(gnufile, "w")

gnu.write("set terminal png size 1260, 800\n")
gnu.write("set output \"aqm-eval-output/"+scenario_name+"/graph/"+queuedisc_name+"-drop.png\"\n set xlabel \"Time difference between two drops\"\nset ylabel \"CDF\"\nset grid\nshow grid\n")


wfile = open (new_file_name ,"w")

flow = 1
for x in bucket:
  wfile.write("\n\n#\"flow"+str(flow)+"\"\n")
  flow+=1
  for y in x:
    wfile.write (str(y[0])+" "+str(y[1])+"\n")
wfile.close()

for data_iterator in range(len(bucket)):
  if data_iterator == 0:
    gnu.write("plot \""+new_file_name+"\" i "+str(data_iterator)+" using 1:2 with lines smooth csplines  title \"Flow "+str(data_iterator+1)+"\"")
  else:
    gnu.write(", \""+new_file_name+"\" i "+str(data_iterator)+" using 1:2 with lines smooth csplines title \"Flow "+str(data_iterator+1)+"\"")

gnu.close()
os.system("gnuplot "+gnufile)

