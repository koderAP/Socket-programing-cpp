import matplotlib.pyplot as plt
import subprocess
import time


def run_experiment(num_clients):
    """Configure the experiment, set config.json, run 'make run', and extract timing from log.txt."""

    config_content = f"""
    {{
        "server_ip": "127.0.0.1",
        "server_port": 8080,
        "k": 40,
        "p": 20,
        "file": "words.txt",
        "T": 20,
        "n": {num_clients}
    }}
    """
    with open("config.json", "w") as f:
        f.write(config_content)
    
    iterations = 2
    avg_time_per_client_aloha = 0
    for _ in range(iterations):
        subprocess.run(["make", "run-aloha"])
        time.sleep(2)
        try:
            with open("log.txt", "r") as f:
                log_content = f.read()
            avg_time_per_client_aloha += float(log_content.split("\n")[-2].split(":")[-1])
        except (FileNotFoundError, IndexError, ValueError):
            print(f"Error reading log.txt for {num_clients} clients.")
            avg_time_per_client_aloha = None
        subprocess.run(["make", "clear_txt"])
    if avg_time_per_client_aloha is not None:
        avg_time_per_client_aloha /= iterations

    avg_time_per_client_beb = 0
    for _ in range(iterations):
        subprocess.run(["make", "run-beb"])
        time.sleep(2)
        try:
            with open("log.txt", "r") as f:
                log_content = f.read()
            avg_time_per_client_beb += float(log_content.split("\n")[-2].split(":")[-1])
        except (FileNotFoundError, IndexError, ValueError):
            print(f"Error reading log.txt for {num_clients} clients.")
            avg_time_per_client_beb = None
        subprocess.run(["make", "clear_txt"])

    if avg_time_per_client_beb is not None:
        avg_time_per_client_beb /= iterations

    avg_time_per_client_csma = 0
    for _ in range(iterations):
        subprocess.run(["make", "run-cscd"])
        time.sleep(2)
        try:
            with open("log.txt", "r") as f:
                log_content = f.read()
            avg_time_per_client_csma += float(log_content.split("\n")[-2].split(":")[-1])
        except (FileNotFoundError, IndexError, ValueError):
            print(f"Error reading log.txt for {num_clients} clients.")
            avg_time_per_client_csma = None
        subprocess.run(["make", "clear_txt"])

    if avg_time_per_client_csma is not None:
        avg_time_per_client_csma /= iterations

    
    return avg_time_per_client_aloha, avg_time_per_client_beb, avg_time_per_client_csma

def plot_results(avg_times, client_count_list):
    """Plot average completion time against the number of clients."""
    plt.plot(client_count_list, avg_times[0], '-s', label='ALOHA')
    plt.plot(client_count_list, avg_times[1], '-s', label='BEB')
    plt.plot(client_count_list, avg_times[2], '-s', label='CSMA/CD')

    plt.xlabel('Number of Clients')
    plt.ylabel('Time (ms)')
    plt.title('Average Completion Time vs. Number of Clients')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot.png')
    plt.show()


if __name__ == "__main__":
    client_count_list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    avg_times = [[],[],[]]

    for num_clients in client_count_list:
        print(f"Running experiment for {num_clients} clients...")
        avg_time_per_client_aloha, avg_time_per_client_beb, avg_time_per_client_cscd  = run_experiment(num_clients)
        avg_times[0].append(avg_time_per_client_aloha)
        avg_times[1].append(avg_time_per_client_beb)
        avg_times[2].append(avg_time_per_client_cscd)

    plot_results(avg_times, client_count_list)
