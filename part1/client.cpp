#pragma once
#include "My_Json.h"

using namespace std;

#define MAX_BUFFER_SIZE 1024

class Client {
private:
    string server_ip;
    int server_port;
    int offset;
    int k;

public:
    Client(string ip, int port, int offset, int k_) : server_ip(std::move(ip)), server_port(port), offset(offset), k(k_) {}

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
    int total_words = 0;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int read_size = recv(sock, buffer, sizeof(buffer), 0);
        if (read_size <= 0) break;

        string packet(buffer);
        if (packet[0] == EOF)  {
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
            stringstream senten(sentence);
            while (getline(senten, word, ',')) {
                if (word[0] == EOF) {
                    is_read = true;
                    break;
                }
                if (word.empty()) continue;
                total_words++;
                word_count[word]++;
            }
            if (is_read) break;
        }
        if (is_read) break;
        if (total_words % k == 0) {
            offset += k;
            string offs = to_string(offset) + "\n";
            send(sock, offs.c_str(), offs.size(), 0);
            total_words = 0;
        }
    }

    ofstream outputFile("output.txt");
    if (!outputFile.is_open()) {
        cerr << "Failed to open output file" << endl;
        exit(1);
    }
    total_words = 0;
    for (const auto &entry: word_count) {
        outputFile << entry.first << ", " << entry.second << endl;
        total_words += entry.second;
    }

    outputFile.close();
    
    close(sock);
    }


};

int main(int argc, char *argv[]) {

    int offse = 0;
    Config config;
    config.readConfig();
    Client client(config.server_ip, config.server_port, offse, config.k);
    client.run();
    return 0;
}