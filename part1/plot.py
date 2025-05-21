import matplotlib.pyplot as plt
import subprocess
import time
import numpy as np

def run_experiment(p):
    config_content = f"""
    {{
        "server_ip": "127.0.0.1",
        "server_port": 8080,
        "k": 10,
        "p": {p},
        "file": "input_543.txt",
        "n": 1
    }}
    """
    with open("config.json", "w") as f:
        f.write(config_content)
    
    
    server = subprocess.Popen(["./server"])
    time.sleep(1) 
    start_time = time.time()
    client = subprocess.Popen(["./client"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    client.wait()
    end_time = time.time()
    server.terminate()
    server.wait()
    

    return end_time - start_time

def plot_results(results):
    ps = list(range(1, 10))
    means = [np.mean(results[p]) for p in ps]
    std_devs = [np.std(results[p]) for p in ps]

    plt.errorbar(ps, means, yerr=std_devs, fmt='-o')
    plt.xlabel('p (Number of words per packet)')
    plt.ylabel('Completion Time (seconds)')
    plt.title('Completion Time vs. p')
    plt.savefig('plot.png')

if __name__ == "__main__":
    results = {p: [] for p in range(1, 10)}
    print(results)
    for p in range(1, 10):
        for _ in range(3):
            print(f"Running experiment for p={p}")
            results[p].append(run_experiment(p))
            print(results)
    print(results)
    plot_results(results)