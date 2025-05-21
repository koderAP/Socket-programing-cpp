#pragma once
#include "My_Json.h"
#include <cstdlib>  
#include <pthread.h>

using namespace std;

#define MAX_BUFFER_SIZE 1024

vector<int> time_served;

class Client {
private:
    string server_ip;
    int server_port;
    int offset;
    int client_id;
    int k;

public:
    Client(string ip, int port, int offset_, int id, int k_) : server_ip(std::move(ip)), server_port(port), offset(offset_), client_id(id), k(k_) {}

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

        string offs = to_string(offset) + "\n";
        send(sock, offs.c_str(), offs.size(), 0);

        map<string, int> word_count;
        char buffer[MAX_BUFFER_SIZE];
        int toatal_words = 0;
        // cout << "Waiting for response..." << endl;
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int read_size = recv(sock, buffer, sizeof(buffer), 0);
            if (read_size <= 0) break;

            string packet(buffer);
            if (packet[0] == EOF) {
                cout << "EOF received." << endl;
                break;
            }

            else if (packet == "$$\n") {
                cout << "Offset is greater than the number of words in text file." << endl;
                break;
            }
            stringstream ss(packet);
            string word, sentence;
            bool is_read = false;

            while (getline(ss, sentence, '\n')) {
                // cout << "Received: " << sentence << endl;
                stringstream senten(sentence);
                while (getline(senten, word, ',')) {
                    // cout << "Word: " << word << endl;
                    if (word[0] == EOF){
                        is_read = true;
                        break;
                    }
                    if (word.empty()) continue;
                    toatal_words++;
                    word_count[word]++;
                }
                if (is_read) break;
            }
            if (is_read) break;
            
            if (toatal_words % k == 0) {
                offset += toatal_words;
                toatal_words = 0;
                string new_offset = to_string(offset) + "\n";
                send(sock, new_offset.c_str(), new_offset.size(), 0);
            }
        }

        string output_file = "output_" + to_string(client_id) + ".txt";
        ofstream outputFile(output_file);
        if (!outputFile.is_open()) {
            cerr << "Failed to open output file" << endl;
            exit(1);
        }
        int total = 0;
        for (const auto &entry: word_count) {
            outputFile << entry.first << ", " << entry.second << endl;
            total += entry.second;
        }

        // cout << "Total words at client "<<client_id<<" : " << total << endl;
        outputFile.close();
        close(sock);
    }

};

void* clientThread(void* args) {
    auto start = chrono::high_resolution_clock::now();
    int client_id = *(int*)args;
    delete (int*)args;  

    int offset = 0;

    Config config;
    config.readConfig();

    Client client(config.server_ip, config.server_port, offset, client_id, config.k);
    client.run();

    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    // cout << "Client " << client_id << " served in " << duration.count() << " ms" << endl;
    time_served[client_id - 1] = duration.count();
    pthread_exit(nullptr);
}

int main(int argc, char *argv[]) {


    Config config;
    config.readConfig();
    int num_clients = config.n;
    time_served.resize(num_clients);
    fill(time_served.begin(), time_served.end(), 0);
    
    vector<pthread_t> threads(num_clients);

    for (int i = 0; i < num_clients; ++i) {
        int* client_id = new int(i + 1);  
        if (pthread_create(&threads[i], nullptr, clientThread, (void*)client_id) != 0) {
            cerr << "Failed to create thread for client " << i + 1 << endl;
            return 1;
        }
    }

    for (int i = 0; i < num_clients; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // cout << "All clients have finished execution." << endl;

    ofstream logFile("log.txt");
    if (!logFile.is_open()) {
        cerr << "Failed to open log file" << endl;
        return 1;
    }
    int total_time = 0;
    for (int i = 0; i < num_clients; ++i) {
        total_time += time_served[i];
    }
    logFile <<total_time / num_clients << endl;

    logFile.close();



    return 0;
}