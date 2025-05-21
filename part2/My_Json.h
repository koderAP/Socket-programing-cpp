#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cctype>
#include <stdexcept>
#include <thread>

using namespace std;

struct Config {
    string server_ip;
    int server_port;
    int k;
    int p;
    int n;
    string file;
    int T;

    string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    string parseJsonString(const string& line) {
        size_t start = line.find(':');
        if (start == string::npos) return "";
        start = line.find('"', start); 
        if (start == string::npos) return "";
        start++;
        size_t end = line.find('"', start);
        if (end == string::npos) return "";
        return line.substr(start, end - start);
    }

    int parseJsonNumber(const string& line) {
        size_t start = line.find(':');
        if (start == string::npos) return 0;
        start++;
        string numStr = trim(line.substr(start));
        try {
            return stoi(numStr);
        } catch (const exception&) {
            return 0;
        }
    }

    void readConfig() {
        ifstream file("config.json");
        if (!file.is_open()) {
            cerr << "Failed to open config.json" << endl;
            exit(1);
        }

        string line;

        while (getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '{' || line[0] == '}') continue;

            if (line.find("server_ip") != string::npos) {
                this->server_ip = parseJsonString(line);
            } else if (line.find("server_port") != string::npos) {
                this->server_port = parseJsonNumber(line);
            } else if (line.find("\"k\"") != string::npos) {
                this->k = parseJsonNumber(line);
            } else if (line.find("\"p\"") != string::npos) {
                this->p = parseJsonNumber(line);
            } else if (line.find("\"n\"") != string::npos) {
                this->n = parseJsonNumber(line);
            } else if (line.find("file") != string::npos) {
                this->file = parseJsonString(line);
            } else if (line.find("T") != string::npos) {
                this->T = parseJsonNumber(line);
            } else if (line.find("num_clients") != string::npos) {
                this->n = parseJsonNumber(line);
            } else if (line.find("input_file") != string::npos) {
                this->file = parseJsonString(line);
            } 
        }

        file.close();
    }
};