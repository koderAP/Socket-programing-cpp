#pragma once
#include <iostream>
#include <string>
#include <pthread.h>
#include <chrono>
#include <random>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "My_Json.h"

using namespace std;
using namespace std::chrono;

#define MAX_BUFFER_SIZE 10240
#define OFFSET 0

int total_clients = 0;
int total_time  = 0;

class Client {
private:
    string server_ip;
    int server_port;
    int offset;
    int client_id;
    double prob;
    int k;
    mt19937 rng; 
    uniform_real_distribution<double> dist;
    map<string, int> wordcount;
    int max_attempts;
    int T;
    std::chrono::high_resolution_clock::time_point tim;

public:
    Client(string ip, int port, int offset_, int id, double prob_, int k_, int t_) : server_ip(std::move(ip)), server_port(port), offset(offset_), client_id(id), prob(prob_),
          rng(static_cast<unsigned>(time(nullptr))), dist(0.0, 1.0) {
                k = k_;
                max_attempts = 16;
                T = t_;
                tim = chrono::high_resolution_clock::now();
          }


    void wait_aloha(){
        cout << "Getting into wait_aloha() : "<< client_id << endl;
        while (true) {
            auto now = system_clock::now();
            auto now_ms = time_point_cast<milliseconds>(now);
            auto epoch = now_ms.time_since_epoch();
            auto value = duration_cast<milliseconds>(epoch).count();
            auto next_slot = ((value / T) + 1) * T;
            auto current_time = system_clock::now();
            auto time_to_wait = milliseconds(next_slot - duration_cast<milliseconds>(current_time.time_since_epoch()).count());
            cout << "Time to wait: " << time_to_wait.count() << "ms" << "for " << client_id << endl;
            this_thread::sleep_for(time_to_wait);
            double random_num = dist(rng);
            if (random_num < prob) {
                cout << "Client " << client_id << " is sending request..." << endl;
                return;
            }
        }
    }

    void wait_beb(int max_attempts){
        int k = 1;
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist_pow(1, static_cast<int>(pow(2, (16 - max_attempts))));
        int wait_time = dist_pow(rng) * T;
        // cout << "Client " << client_id << " is waiting for " << wait_time << "ms" << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
        max_attempts--;
        return;
    }

    void wait_sensing_beb(){
        // cout << "Client " << client_id << " is waiting for " << T << "ms" << endl;
        this_thread::sleep_for(chrono::milliseconds(T));
    }

    bool send_request(int sock, int type) {
        char buffer[MAX_BUFFER_SIZE];
        bool huh = false;
        bool received = false;        
        while (true){ 

            if (type == 0){
                wait_aloha();
            }
            else if (type == 1){
                if (max_attempts == 0) {
                    cout << "Max attempts reached for client " << client_id << endl;
                    return false;
                }
                wait_beb(max_attempts);
            }
            else if (type == 2){
                wait_sensing_beb();
            }
            

            huh = false;
            vector<string> words = {};
            string offs = to_string(offset) + "\n";
            // cout << "Sending offset: " << offs << " for client " << client_id << endl;
            send(sock, offs.c_str(), offs.size(), 0);


            while (true)
            {
                memset(buffer, 0, sizeof(buffer));
                int read_size = recv(sock, buffer, sizeof(buffer), 0);
                if (read_size <= 0) break;

                string response(buffer);
                stringstream ss(response);
                string packet;
                

                while (getline(ss, packet, '\n')) {
                    // cout << "Packet received: " << packet << " for client " << client_id << endl;
                    if (packet[0] == EOF || packet == "$$"){
                        received = true;
                        cout << "EOF received by : " << client_id << endl;
                        break;
                    }
                    if (packet == "HUH!"){
                        huh = true;
                        cout << "HUH! received : " << client_id << endl;
                        break;
                    }
                    stringstream ss1(packet);
                    string word;
                    size_t len = packet.length();
                    if (packet[len -1] == EOF){
                        while (getline(ss1, word, ','))
                            {
                            if (packet[0] == EOF || packet == "$$"){
                                received = true;
                                cout << "EOF received : " << client_id << endl;
                                break;
                            }
                            if (packet == "HUH!"){
                                huh = true;
                                cout << "HUH! received : " << client_id << endl;
                                break;
                            }
                            
                            words.push_back(word);
                            
                        }
                        words.pop_back();
                        received = true;
                        
                        
                    }else{

                    while (getline(ss1, word, ',')) {
                        // cout << "Word received: " << word << " for client " << client_id << endl;
                        if (packet[0] == EOF || packet == "$$"){
                            received = true;
                            break;
                        }
                        if (packet == "HUH!"){
                            huh = true;
                            // cout << "HUH! received : " << client_id << endl;
                            break;
                        }
                        words.push_back(word);
                    }

                    }
                    if (huh) break;
                    if (received) break;
                    if (words.size() == k){
                        offset += k;
                        break;
                    }
                    

                }

                if (received) break;
                if (huh) break;
                if (words.size() == k){
                    break;
                }
                
            }

            if (!huh){
                max_attempts = 16;
                for (const auto &word: words) {
                    wordcount[word]++;
                }
            }
            if (received) break;
            

        }
        

        string output_file = "output_" + to_string(client_id) + ".txt";
        ofstream outputFile(output_file);
        if (!outputFile.is_open()) {
            cerr << "Failed to open output file" << endl;
            exit(1);
        }

        for (const auto &entry: wordcount) {
            outputFile << entry.first << ", " << entry.second << endl;
        }

        outputFile.close();
        // cout << "Client " << client_id << " has finished..." << endl;


        auto now = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(now - tim);
        total_time += duration.count();

        return true;
    }

  
  void run(string protocol) {
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

        if (protocol == "slotted-aloha"){
            send_request(sock, 0);
        } else if (protocol == "beb") {
            send_request(sock, 1);
        } else if (protocol == "sensing-beb") {
            send_request(sock, 2);
        } else {
            cerr << "Invalid protocol..." << endl;
            close(sock);
            exit(1);
            return;
        }
        total_clients--;
    }




};

struct ThreadArgs {
    int client_id;
    string protocol;
    string server_ip;
    int server_port;
    int num;
    int k;
    int t;
};

void* client_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    string protocol = args->protocol;
    int n = args->num;  
    int client_id = *(int*)arg;
    int port = args->server_port;
    int k = args->k;
    int t = args->t;
    string server_ip = args->server_ip;
    Client client(server_ip, port, OFFSET, client_id, 1.0 / n, k, t);
    client.run(protocol);
    // cout << "Total clients remaining: " << total_clients << endl;
    pthread_exit(nullptr);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << "<protocol>" << endl;
        return 1;
    }

    string protocol = argv[1];
    Config config;
    config.readConfig();
    int num = config.n;
    total_clients = num;
    string serverip = config.server_ip;
    int serverport = config.server_port;
    int k = config.k;
    int t = config.T;

    pthread_t threads[num];
    int client_ids[num];

    for (int i = 0; i < num; i++) {
        ThreadArgs* args = new ThreadArgs;
        args->client_id = i + 1;
        args->protocol = protocol;
        args->server_ip = serverip;
        args->server_port = serverport;
        args->num = num;
        args->k = k;
        args->t = t;

        int rc = pthread_create(&threads[i], NULL, client_thread, (void*)args);
        if (rc != 0) {
            cerr << "Error creating thread: " << rc << endl;
            return 1;
        }
    }

    for (int i = 0; i < num; i++) {
        pthread_join(threads[i], NULL);
    }


    ofstream logFile("log.txt");
    if (!logFile.is_open()) {
        cerr << "Failed to open log file" << endl;
        return 1;
    }

    logFile << total_time / num << endl;



    return 0;
}