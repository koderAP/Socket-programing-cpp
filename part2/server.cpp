#pragma once
#include "My_Json.h"
#include <pthread.h>

using namespace std;

#define MAX_BUFFER_SIZE 1024

class Server {
private:
    string server_ip;
    int server_port;
    int p;
    int k;
    int n;
    string filename;
    vector<string> words;

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

        cout << "Loaded words...";
        cout << endl;
    }

    static void* handleClient(void* args) {
        auto* clientArgs = (ClientArgs*) args;
        Server* server = clientArgs->server_instance;
        int client_socket = clientArgs->client_socket;
        delete clientArgs;

        char buffer[MAX_BUFFER_SIZE];
        bool is_sent = false;
        
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int read_size = recv(client_socket, buffer, sizeof(buffer), 0);
            if (read_size <= 0) break;

            string request(buffer);
            int offset = stoi(request);
            // cout << "Offset: " << offset << endl;
            if (offset > server->words.size()) {
                send(client_socket, "$$\n", 3, 0);
                close(client_socket);
                pthread_exit(nullptr);
            }
            // cout << server->words.size() << endl;
            int z = min(server->k + offset, (int)server->words.size());
            // cout << "Z: " << z << endl;
    
            while (offset < z) {
                string response = "";
                // cout << response << endl;
                int temp = 0;
                for (int i = offset; i < offset + server->p && i < z; i++) {
                    response += server->words[i] + ",";
                    temp++;
                }
                offset += temp;
                response += "\n";
                if (offset >= server->words.size()) {
                    response += EOF;
                    response += "\n";
                    is_sent = true;
                }
                send(client_socket, response.c_str(), response.size(), 0);
                
            }
            if (is_sent){
                sleep(1);
                break;
            }
            
        }

        pthread_exit(nullptr);
    }

public:
    Server(string ip, int port, int p_val, int k_val, int n_val, string file) :
            server_ip(std::move(ip)), server_port(port), p(p_val), k(k_val), n(n_val), filename(std::move(file)) {
        loadFile();
    }

    void start() {
        
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1) {
            cout << "Could not create socket" << endl;
            exit(1);
        }
        int opt = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
        server_addr.sin_port = htons(server_port);

        if (::bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            cout << "Bind failed" << endl;
            close(server_socket);
            exit(1);
        }

        listen(server_socket, n);
        cout << "Server listening on " << server_ip << ":" << server_port << endl;


        int clientCount = 0;
        vector<pthread_t> threads;

        while (clientCount < n) {
            int client_socket = accept(server_socket, nullptr, nullptr);
            if (client_socket < 0) {
                cout << "Accept failed" << endl;
                close(server_socket);
                exit(1);
            }

            cout << "Client connected!" << endl;

            pthread_t thread_id;
            ClientArgs* clientArgs = new ClientArgs{this, client_socket};
            if (pthread_create(&thread_id, nullptr, handleClient, clientArgs) != 0) {
                cout << "Failed to create thread" << endl;
                close(client_socket);
                close(server_socket);
                exit(1);
            }

            threads.push_back(thread_id);
            clientCount++;
        }

        for (pthread_t thread : threads) {
            pthread_join(thread, nullptr);
        }
        
        close(server_socket);
    }
};

int main() {
    Config config;
    config.readConfig();
    Server server(config.server_ip, config.server_port, config.p, config.k, config.n, config.file);
    server.start();
    return 0;
}