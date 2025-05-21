#pragma once
#include "My_Json.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <random>
#include <chrono>
#include <mutex>

using namespace std;

#define MAX_BUFFER_SIZE 1024

// Mutex for thread-safe access to log.txt
std::mutex time_file_mutex;

class Client {
private:
    string server_ip;
    int server_port;
    int offset;
    int client_id;
    int k;
    int request_no;

public:
    Client(string ip, int port, int offset_, int id, int k_, int request_no_)
        : server_ip(std::move(ip)), server_port(port), offset(offset_), client_id(id), k(k_), request_no(request_no_) {}

    void run() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            cerr << "Could not create socket" << endl;
            exit(1);
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

        if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            cerr << "Connect failed" << endl;
            close(sock);
            exit(1);
        }

        // Record start time
        auto start_time = std::chrono::high_resolution_clock::now();

        string offs = to_string(offset) + "\n";
        send(sock, offs.c_str(), offs.size(), 0);

        map<string, int> word_count;
        char buffer[MAX_BUFFER_SIZE];

        int words_received = 0;
        bool is_read = false;

            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int read_size = recv(sock, buffer, sizeof(buffer), 0);
                if (read_size <= 0) break;

                string packet(buffer);
                if (packet == "EOF\n" || packet == "EOF") {
                    break;
                } else if (packet == "$$\n") {
                    break;
                }

                stringstream ss(packet);
                string word, sentence;
                while (getline(ss, sentence, '\n')) {
                    stringstream senten(sentence);
                    while (getline(senten, word, ',')) {
                        if (word == "EOF") {
                            is_read = true;
                            // cout << "EOF received." << endl;
                            break;
                        }
                        word_count[word]++;
                        words_received++;
                    }
                    if (is_read) break;
                }
                if (is_read) break;
                if (words_received % k == 0) {
                    offset += k;
                    string offs = to_string(offset) + "\n";
                    send(sock, offs.c_str(), offs.size(), 0);
                }
            }

            auto end_time = std::chrono::high_resolution_clock::now();

            // Calculate duration
            std::chrono::duration<double> duration = end_time - start_time;

            // Log to log.txt in a thread-safe manner
            {
                std::lock_guard<std::mutex> lock(time_file_mutex);
                ofstream time_file("log.txt", ios::app);
                if (!time_file.is_open()) {
                    cerr << "Failed to open log.txt" << endl;
                    exit(1);
                }
                // Log client_socket (sock) and duration
                time_file << client_id << "," << duration.count() << endl;
                time_file.close();
            }

            if (is_read) {
            string output_file = "output_" + to_string(client_id) + "_request_" + to_string(request_no) + ".txt";
            ofstream outputFile(output_file);
            if (!outputFile.is_open()) {
                cerr << "Failed to open output file" << endl;
                exit(1);
            }

            for (const auto &entry: word_count) {
                outputFile << entry.first << ", " << entry.second << endl;
            }

            outputFile.close();
        }

        close(sock);
    }
};

void* client_thread(void* arg) {
    auto [config, offset, id, request_no] = *static_cast<std::tuple<Config, int, int, int>*>(arg);
    Client client(config.server_ip, config.server_port, offset, id, config.k, request_no);
    client.run();
    delete static_cast<std::tuple<Config, int, int, int>*>(arg);
    return nullptr;
}

int main() {
    Config config;
    config.readConfig();

    int num_client = config.n;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, num_client);
    int rogue_client_id = dis(gen);

    vector<pthread_t> threads;
    int request_no = 1; // To assign request numbers

    for (int i = 1; i <= num_client; ++i) {
        if (i == rogue_client_id) {
            // cout << "Rogue client " << i << " sending 5 concurrent requests." << endl;
            // Create 5 threads for the rogue client
            for (int j = 1; j <= 5; ++j) {
                pthread_t thread;
                pthread_create(&thread, NULL, client_thread, new std::tuple<Config, int, int, int>(config, 0, i, j));
                threads.push_back(thread);
            }
        } else {
            pthread_t thread;
            pthread_create(&thread, NULL, client_thread, new std::tuple<Config, int, int, int>(config, 0, i, 1));
            threads.push_back(thread);
        }
    }

    for (auto &thread : threads) {
        pthread_join(thread, NULL);
    }

    return 0;
}