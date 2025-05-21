import os
import subprocess
import threading
import time
import json
import matplotlib.pyplot as plt

def run_experiment(num_clients, policy):
    """Run server and multiple clients, returning total time and average time per client."""
    config_content = f"""
    {{
        "server_ip": "127.0.0.1",
        "server_port": 8080,
        "k": 100,
        "p": 5,
        "input_file": "words.txt",
        "num_clients": {num_clients}
    }}
    """
    with open("config.json", "w") as f:
        f.write(config_content)

    time.sleep(1)

    server = subprocess.Popen(["./server", policy])
    time.sleep(1)  
    
    client = subprocess.Popen(["./client"])
    server.wait()
    client.wait()
    time.sleep(1)
    # read from log.txt
    # copy log.txt to policy.txt
    os.system(f"cp log.txt {policy}.txt")
    with open("log.txt", "r") as f:
        # read the log file as elements of a list with each element as a line
        log_content = f.read().split("\n")
    
    with open("log.txt", "w") as f:
        f.truncate(0)

    server.kill()
    wait_time = 1
    time.sleep(wait_time)

    d = {}
    
    for i in log_content:
        a = list(i.split(","))
        if len(a) == 2:
            if a[0] in d:
                d[a[0]].append(float(a[1]))
            else:
                d[a[0]] = [float(a[1])]

        
    return d

def jains_fairness_index(completion_times):
    """Calculates Jain's fairness index."""
    n = len(completion_times)
    total_time = sum(completion_times)
    squared_sum = sum([x**2 for x in completion_times])
    print(total_time**2, n, squared_sum)
    return (total_time**2) / (n * squared_sum)

n = 10

fifo = run_experiment(n, "FIFO")

rr = run_experiment(n, "ROUND_ROBIN")

fifo_time = []
rr_time = []

for i in fifo:
    fifo_time.append(sum(fifo[i]))

for i in rr:
    rr_time.append(sum(rr[i]))

print(fifo_time)
print(rr_time)

f = open("jains_fairness_index.txt", "w")

print(fifo_time)

for x in fifo_time:
    f.write(str(x) + ", ")

f.write(str(jains_fairness_index(fifo_time)) + "\n")

for x in rr_time:
    f.write(str(x) + ", ")

f.write(str(jains_fairness_index(rr_time)))

print("FIFO: ", jains_fairness_index(fifo_time))
print("RR: ", jains_fairness_index(rr_time))

f.close()