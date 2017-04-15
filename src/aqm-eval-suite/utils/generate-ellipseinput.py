import sys

scenario_name = sys.argv[1]
queuedisc_name = sys.argv[2]
read_qdelay_file = open("aqm-eval-output/"+scenario_name+"/data/"+queuedisc_name+"-qdel.dat", "r")
read_throughput_file = open("aqm-eval-output/"+scenario_name+"/data/"+queuedisc_name+"-throughput.dat", "r") 
write_file = open("aqm-eval-output/"+scenario_name+"/data/"+queuedisc_name+"-result.dat", "w")
qdelay_lines = read_qdelay_file.readlines()
throughput_lines = read_throughput_file.readlines()
last_time_interval = 0.0
qdelay_ans = 0.0
qdelay_count = 0
throughput_index = 0
throughput_val = 0
throughput_count = 0
for l in qdelay_lines:
  s = l.split(' ')
  qdelay_time = float(s[0])
  qdelay_val = float(s[1])
  curr_time_interval = ((int)(qdelay_time*10))/10.0
  if last_time_interval == curr_time_interval:
    qdelay_ans += qdelay_val
    qdelay_count += 1
  else:
    if not qdelay_count == 0:
        qdelay_ans /= qdelay_count
        while throughput_index<len(throughput_lines):
            throughput_time = float((throughput_lines[throughput_index].split(' '))[0])
            if throughput_time < curr_time_interval:
                throughput_val += float((throughput_lines[throughput_index].split(' '))[1])
                throughput_count +=1
                throughput_index +=1
            else:
                break
        if not throughput_count == 0:
          throughput_ans = throughput_val/throughput_count
        else:
          throughput_ans = 0
        write_file.write(str(qdelay_ans)+' '+str(throughput_ans)+'\n')
        throughput_val = 0
        throughput_count = 0
        qdelay_ans = 0
        qdelay_count = 0
    last_time_interval = curr_time_interval

if not qdelay_count == 0: 
    qdelay_ans /=  qdelay_count
    while throughput_index<len(throughput_lines):
        throughput_val += float((throughput_lines[throughput_index].split(' '))[1])
        throughput_count +=1
        throughput_index +=1
    if not throughput_count == 0:
      throughput_ans = throughput_val/throughput_count
    else:
      throughput_ans = 0
    write_file.write(str(qdelay_ans)+' '+str(throughput_ans)+'\n')
    throughput_val = 0
    throughput_count = 0
    qdelay_ans = 0
    qdelay_count = 0
    last_time_interval = curr_time_interval

read_qdelay_file.close()
read_throughput_file.close()
write_file.close()
