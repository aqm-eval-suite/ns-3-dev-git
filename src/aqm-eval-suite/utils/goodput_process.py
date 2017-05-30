import sys
import os

scenario_name = sys.argv[1]
queuedisc_name = sys.argv[2]
file_name = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-goodput.dat'
new_file_name = 'aqm-eval-output/'+scenario_name+"/data/new-"+queuedisc_name+'-goodput.dat'
File = open (file_name ,"r")
lines_read = File.readlines ()
lines_read.sort ()
File.close ()
i = 0
data = []
File = []
inst_data = []
for file_iterator in lines_read:
  if i == len(lines_read)-1:
    break
  i+=1
  if file_iterator.split(' ')[0] == lines_read[i].split(' ')[0]:
    string_uno = float(file_iterator.split(' ')[1])
    string_duo = int((file_iterator.split(' ')[2]).replace('\n', ''))
    data.append([string_uno,string_duo])
  else:
    data.sort()
    goodput_val = 0
    goodput_count = 0
    last_recorded = 0
    for a in data:
      if a[0] - last_recorded < 0.1:
        goodput_val += a[1]
        goodput_count+=1
      else:
        last_recorded = a[0]
        if goodput_count>0:
          inst_data.append([a[0], (goodput_val*1.0/a[0])])
    last_recorded = a[0]
    if goodput_count>0:
      inst_data.append([a[0], (goodput_val*1.0/a[0])])
    data = []
    File.append(inst_data)
    inst_data = []

data.sort()
goodput_val = 0
goodput_count = 0
last_recorded = 0
for a in data:
  if a[0] - last_recorded < 0.1:
    goodput_val += a[1]
    goodput_count+=1
  else:
    last_recorded = a[0]
    if goodput_count>0:
      inst_data.append([float("{0:.2f}".format(a[0])), (goodput_val*1.0/a[0])])
last_recorded = a[0]
if goodput_count>0:
  inst_data.append([float("{0:.2f}".format(a[0])), (goodput_val*1.0/a[0])])
File.append(inst_data)

st = ""

gnufile = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-gnu-goodput'
gnu = open(gnufile, "w")

gnu.write("set terminal png\n")
gnu.write("set output \"aqm-eval-output/"+scenario_name+"/graph/"+queuedisc_name+"-goodput.png\"\n set xlabel \"Time (Seconds)\" font \"Verdana,12\"\nset ylabel \"Goodput (Mbps)\" font \"Verdana,12\"\nset grid\nshow grid\nset key font \"Verdana,12\"\n")


wfile = open (new_file_name ,"w")

flow = 1
for x in File:
  wfile.write("\n\n#\"flow"+str(flow)+"\"\n")
  flow+=1
  for y in x:
    wfile.write (str(y[0])+" "+str(y[1]/(1024*128))+"\n")
wfile.close()

if "RttFairness" in scenario_name:
  for j in range(len(File)):
    if j == 0:
      gnu.write("plot \""+new_file_name+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Fixed Rtt Flow\"")
    else:
      gnu.write(", \""+new_file_name+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Variable Rtt Flow\"")
else:
  for j in range(len(File)):
    if j == 0:
      gnu.write("plot \""+new_file_name+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Flow "+str(j+1)+"\"")
    else:
      gnu.write(", \""+new_file_name+"\" i "+str(j)+" using 1:2 with lines smooth csplines title \"Flow "+str(j+1)+"\"")

gnu.close()
os.system("gnuplot "+gnufile)



