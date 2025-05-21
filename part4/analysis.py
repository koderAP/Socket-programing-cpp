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
        "server_ip": "127.0.0.5",
        "server_port": 8089,
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
    with open("log.txt", "r") as f:
        # read the log file as elements of a list with each element as a line
        log_content = f.read().split("\n")
    
    with open("log.txt", "w") as f:
        f.truncate(0)

    server.kill()
    wait_time = 1
    time.sleep(wait_time)

    client_id = []
    completion_times = []
    
    for i in log_content:
        print(i)
        print(len(i.split(",")))
        a = list(i.split(","))
        print(a)
        if len(a) == 2:
            client_id.append(a[0])
            completion_times.append(float(a[1]))
        
    return  client_id, completion_times

def plot_results(n_values, fifo_times, rr_times):
    """Plots the comparison of average completion times for FIFO and Round Robin."""
    plt.figure()
    plt.plot(n_values, fifo_times, label="FIFO")
    plt.plot(n_values, rr_times, label="Round Robin")
    plt.xlabel("Number of Clients (n)")
    plt.ylabel("Average Completion Time (seconds)")
    plt.title("Comparison of Scheduling Policies: FIFO vs Round Robin")
    plt.legend()
    plt.savefig("plot.png")
    plt.show()

def main():
    n_values = [5, 10, 15, 20, 25, 30, 35, 40, 45, 50]
    fifo_times = []
    rr_times = []
    
    for n in n_values:
        client_id, completion_times = run_experiment(n, "FIFO")
        fifo_times.append(sum(completion_times) / len(completion_times))
        
        client_id, completion_times = run_experiment(n, "ROUND_ROBIN")
        rr_times.append(sum(completion_times) / len(completion_times))
        print(f"Client ID: {client_id}")
        print(f"Completion Times: {completion_times}")
        print()

    plot_results(n_values, fifo_times, rr_times)

if __name__ == "__main__":
    main()