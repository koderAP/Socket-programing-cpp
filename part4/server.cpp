#pragma once
#include "My_Json.h"
#include <pthread.h>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>
#include <atomic>
#include <set>
#include <unistd.h>

using namespace std;

#define MAX_BUFFER_SIZE 10240

std::atomic<int> MAX(10);

enum SchedulingPolicy {
    FIFO,
    ROUND_ROBIN
};

class Server {
public:
    string server_ip;
    int server_port;
    int p;
    int k;
    int n;
    string filename;
    vector<string> words;
    queue<int> fifo_queue;
    unordered_map<int, queue<int>> rr_queue;
    int server_socket;
    int client_unserved;
    
    std::set<int> client_set;
    SchedulingPolicy policy;
    pthread_mutex_t queue_mutex;
    vector<pthread_t> threads;

    Server(SchedulingPolicy sched_policy) : policy(sched_policy) {
        Config config;
        config.readConfig();
        server_ip = config.server_ip;
        server_port = config.server_port;
        p = config.p;
        k = config.k;
        n = config.n;
        client_unserved = n+4;
        filename = config.file;
        pthread_mutex_init(&queue_mutex, nullptr);
        loadFile();
    }

    struct ClientArgs {
        Server* server_instance;
        int client_socket;
    };

    void loadFile() {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filename << endl;
            exit(1);
        }
        string word;
        while (getline(file, word, ',')) {
            words.push_back(word);
        }
        file.close();
        cout << "Loaded words..." << endl;
    }

    void addToQueue(int client_socket) {
        pthread_mutex_lock(&queue_mutex);

        if (client_set.size() < n+4 || client_set.find(client_socket) != client_set.end()) {
            if (client_set.find(client_socket) == client_set.end()) {
                client_set.insert(client_socket);
                // cout << "Client " << client_socket << " added to set." << endl;
            } else {
                // cout << "Client " << client_socket << " already in set." << endl;
            }

            if (policy == FIFO) {
                fifo_queue.push(client_socket);
            } else if (policy == ROUND_ROBIN) {
                if (rr_queue[client_socket].empty()) {
                    rr_queue[client_socket] = queue<int>();
                }
                rr_queue[client_socket].push(client_socket);
            }
        } else {
            cerr << "Client " << client_socket << " not added to queue. Set is full." << endl;
            close(client_socket);
        }

        pthread_mutex_unlock(&queue_mutex);
    }

    int getNextRequest() {
        pthread_mutex_lock(&queue_mutex);
        int client_socket = -1;

        if (policy == FIFO && !fifo_queue.empty()) {
            client_socket = fifo_queue.front();
            fifo_queue.pop();
        } else if (policy == ROUND_ROBIN) {
            for (auto& [client, q] : rr_queue) {
                if (!q.empty()) {
                    client_socket = q.front();
                    q.pop();
                    if (q.empty()) {
                        rr_queue.erase(client);
                    }
                    break;
                }
            }
        }

        pthread_mutex_unlock(&queue_mutex);
        return client_socket;
    }

    void* scheduler(void* arg) {
        Server* server = (Server*) arg;

        while (true) {
            int client_socket = server->getNextRequest();
            
            if (client_socket != -1) {
                server->processRequest(client_socket);
            }
        }
        return nullptr;
    }

    void processRequest(int client_socket) {
        char buffer[MAX_BUFFER_SIZE];

        // while (true) {
            memset(buffer, 0, sizeof(buffer));
            int read_size = recv(client_socket, buffer, sizeof(buffer), 0);
            if (read_size <= 0) {
                close(client_socket);
                return;
            }

            string request(buffer);
            // cout << "Received: " << request << endl;

            if (request == "DONE") {
                // cout << "Client " << client_socket << " finished session." << endl;
                close(client_socket);
                return;
            }

            int offset = stoi(request);
            // cout << "Offset: " << offset << endl;

            if (offset >= words.size()) {
                send(client_socket, "$$\n", 3, 0);
                close(client_socket);
                return;
            }

            int z = min(k + offset, (int) words.size());
            string response;
            bool is_going = true;
            for (int i = offset; i < z;) {
                response = "";
                int j = 0;
                while (i + j < z && j < p) {
                    response += words[i + j] + ",";
                    j++;
                }
                i += j;
                if (i >= words.size()) {
                    response += "EOF";
                    is_going = false;
                }
                response += "\n";
                send(client_socket, response.c_str(), response.size(), 0);
                // cout << response << endl;
            }
            
            // cout << "Client: " << client_socket << " has been served for offset: " << offset << endl;
            if (!is_going) {
                client_unserved--;
                // cout << "Client " << client_socket << " has been served." << endl;
                // cout << "Remaining clients: " << client_unserved << endl;
                if (client_unserved == 0) {
                    // cout << "All clients have been served." << endl;
                    for (auto &thre : threads) {
                        pthread_join(thre, nullptr);
                    }
                    close(server_socket);
                    return;
                }
                return;
            }
        // }
        addToQueue(client_socket);
    }

    static void* handleClient(void* args) {
        auto* clientArgs = (ClientArgs*) args;
        Server* server = clientArgs->server_instance;
        int client_socket = clientArgs->client_socket;
        delete clientArgs;

        server->addToQueue(client_socket);

        pthread_exit(nullptr);
    }

    void start() {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            cerr << "Failed to create socket" << endl;
            exit(1);
        }

        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server_port);
        server_address.sin_addr.s_addr = inet_addr(server_ip.c_str());

        if (::bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
            cerr << "Failed to bind" << endl;
            exit(1);
        }

        if (listen(server_socket, n+4) < 0) {
            cerr << "Failed to listen" << endl;
            exit(1);
        }

        cout << "Server started at " << server_ip << ":" << server_port << endl;

        pthread_t scheduler_thread;
        pthread_create(&scheduler_thread, nullptr, [](void* arg) -> void* {
            Server* server = (Server*) arg;
            server->scheduler(server);  // Call the non-static scheduler
            return nullptr;
        }, this);

        int clientCount = 0;
        // vector<pthread_t> threads;

        while (client_unserved > 1) {
            int client_socket = accept(server_socket, nullptr, nullptr);
            if (client_socket < 0) {
                // cout << "Client unserved: " << client_unserved << endl;
                for (auto &thre : threads) {
                    pthread_join(thre, nullptr);
                }
                close(server_socket);
                exit(1);
            }

            pthread_t thread;
            ClientArgs* clientArgs = new ClientArgs;
            clientArgs->server_instance = this;
            clientArgs->client_socket = client_socket;
            pthread_create(&thread, nullptr, handleClient, (void*) clientArgs);
            threads.push_back(thread);
            // clientCount++;
            if (client_unserved == 0) {
                break;
            }
        }

        for (auto thre : threads) {
            pthread_join(thre, nullptr);
        }

        close(server_socket);
    }
};

// Usage: ./server <policy> <max_requests>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./server <policy>" << endl;
        exit(1);
    }

    SchedulingPolicy policy;
    string policy_str = argv[1];
    if (policy_str == "FIFO") {
        policy = FIFO;
    } else if (policy_str == "ROUND_ROBIN") {
        policy = ROUND_ROBIN;
    } else {
        cerr << "Invalid policy" << endl;
        exit(1);
    }

    Config config;
    config.readConfig();

    // Set the global MAX value based on the command-line argument
    MAX = config.n * 3;

    Server server(policy);
    server.start();
    return 0;
}