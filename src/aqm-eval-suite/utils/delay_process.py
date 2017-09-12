import sys
import os

scenario_name = sys.argv[1]
queuedisc_name = sys.argv[2]
file_name = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-qdel.dat'
File = open (file_name ,"r")
lines_read = File.readlines ()
lines_read.sort ()
File.close ()

gnufile = 'aqm-eval-output/'+scenario_name+"/data/"+queuedisc_name+'-gnu-delay'
gnu = open(gnufile, "w")

gnu.write("set terminal png\n")
gnu.write("set output \"aqm-eval-output/"+scenario_name+"/graph/"+queuedisc_name+"-delay.png\"\n set xlabel \"Time (Seconds)\" font \"Verdana,12\"\nset ylabel \"Delay (ms)\" font \"Verdana,12\"\nset grid\nshow grid\nset key font \"Verdana,12\"\nset yrange [0:]\n")


if "RttFairness" in scenario_name:
  gnu.write("set xrange [:600]\n")
else:
  gnu.write("set xrange [:300]\n")

gnu.write("plot \""+file_name+"\" using 1:2 with lines title \""+queuedisc_name+"\"")

gnu.close()
os.system("gnuplot "+gnufile)



