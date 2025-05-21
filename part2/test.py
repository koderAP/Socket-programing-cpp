import matplotlib.pyplot as plt
import subprocess
import time


def run_experiment(num_clients):
    """Configure the experiment, set config.json, run 'make run', and extract timing from log.txt."""

    config_content = f"""
    {{
        "server_ip": "127.0.0.1",
        "server_port": 8080,
        "k": 100,
        "p": 1,
        "file": "words.txt",
        "n": {num_clients}
    }}
    """
    with open("config.json", "w") as f:
        f.write(config_content)
    
    avg_time_per_client = 0
    for _ in range(1):
        subprocess.run(["make", "run"])
        time.sleep(2)
        try:
            with open("log.txt", "r") as f:
                log_content = f.read()
            avg_time_per_client += float(log_content.split("\n")[-2].split(":")[-1])
        except (FileNotFoundError, IndexError, ValueError):
            print(f"Error reading log.txt for {num_clients} clients.")
            avg_time_per_client = None
        subprocess.run(["make", "clear_txt"])
    if avg_time_per_client is not None:
        avg_time_per_client /= 5
    return avg_time_per_client


def plot_results(avg_times, client_count_list):
    """Plot average completion time against the number of clients."""
    plt.plot(client_count_list, avg_times, '-s', label='Average Time per Client')

    plt.xlabel('Number of Clients')
    plt.ylabel('Time (seconds)')
    plt.title('Average Completion Time vs. Number of Clients')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot.png')
    plt.show()


if __name__ == "__main__":
    client_count_list = [1, 4, 8, 12, 16, 20, 24, 28, 32]
    avg_times = []

    for num_clients in client_count_list:
        print(f"Running experiment for {num_clients} clients...")
        avg_time_per_client = run_experiment(num_clients)
        if avg_time_per_client is not None:
            avg_times.append(avg_time_per_client)

    plot_results(avg_times, client_count_list)
