#pragma once
#include "My_Json.h"
#include <cstdlib>  // For std::atoi
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>  // For close()
#include <pthread.h>
#include <random>    // For random number generation

using namespace std;

#define MAX_BUFFER_SIZE 1024

class Client {
private:
    string server_ip;
    int server_port;
    int offset;
    int client_id;

public:
    Client(string ip, int port, int offset_, int id) : server_ip(std::move(ip)), server_port(port), offset(offset_), client_id(id) {}

    void run(int request_count = 1) {
        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            cerr << "Could not create socket" << endl;
            exit(1);
        }

        // Define server address
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

        // Connect to server
        if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            cerr << "Connect failed" << endl;
            close(sock);
            exit(1);
        }

        // Loop to send multiple requests if necessary
        for (int req = 0; req < request_count; ++req) {
            // Send offset to server
            string offs = to_string(offset) + "\n";
            send(sock, offs.c_str(), offs.size(), 0);

            map<string, int> word_count;
            char buffer[MAX_BUFFER_SIZE];

            // Receive data from server
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

                // Process received words
                stringstream ss(packet);
                string word, sentence;
                bool is_read = false;
                while (getline(ss, sentence, '\n')) {
                    stringstream senten(sentence);
                    while (getline(senten, word, ',')) {
                        if (word == "EOF") {
                            is_read = true;
                            break;
                        }
                        word_count[word]++;
                    }
                    if (is_read) break;
                }
                if (is_read) break;
            }

            // Write results to output file
            string output_file = "output_" + to_string(client_id) + "_request_" + to_string(req + 1) + ".txt";
            ofstream outputFile(output_file);
            if (!outputFile.is_open()) {
                cerr << "Failed to open output file" << endl;
                exit(1);
            }

            // Write word counts
            for (const auto &entry: word_count) {
                outputFile << entry.first << ", " << entry.second << endl;
            }

            outputFile.close();
        }

        close(sock);  // Close the socket after all requests
    }
};

void* client_thread(void* arg) {
    auto [config, offset, id, request_count] = *static_cast<std::tuple<Config, int, int, int>*>(arg);
    Client client(config.server_ip, config.server_port, offset, id);
    client.run(request_count);
    delete static_cast<std::tuple<Config, int, int, int>*>(arg);
    return nullptr;
}

int main(int argc, char* argv[]) {
    Config config;
    config.readConfig();

    if (argc != 3) {
        cerr << "Usage: ./client_single <client_id> <rogue>" << endl;
        exit(1);
    }

    int client_id = atoi(argv[1]);
    string rogue = argv[2];

    int num_client = config.n;

    pthread_t* threads = new pthread_t[num_client];

    if (rogue == "1") {
        tuple<Config, int, int, int>* args = new tuple<Config, int, int, int>{config, 1, client_id, 5};
        client_thread((void*) args);
    }
    else {
        tuple<Config, int, int, int>* args = new tuple<Config, int, int, int>{config, 1, client_id, 1};
        client_thread((void*) args);
    }

    for (int i=0; i<num_client; i++) {
        pthread_join(threads[i], nullptr);
    }

    return 0;
}