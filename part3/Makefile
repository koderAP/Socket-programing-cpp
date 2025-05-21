# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11 -Wno-pragma-once-outside-header -Wunused-variable -Wno-unused-variable -Wno-return-type

PORT_NUMBER = 8080

# Executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

.PHONY: all build run clean

# Default target
all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o server

client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

run-aloha:
	./server &
	sleep 1 &
	./client "slotted-aloha"

run-beb:
	./server &
	sleep 1 &
	./client "beb"


run-cscd:
	./server &
	sleep 1 &
	./client "sensing-beb"

clean_port:
	@echo "Clearing port $(PORT_NUMBER)..."
	@sudo kill -9 $$(sudo lsof -t -i :$(PORT_NUMBER)) || echo "No processes found on port $(PORT_NUMBER)"
	@echo "Port $(PORT_NUMBER) cleared."
plot:
	python3 test.py

test:
	python3 tester.py
clean_txt:
	find . -name "*.txt" ! -name "words.txt" -type f -delete

clear_txt:
	find . -name "*.txt" ! -name "words.txt" -type f -delete
clean:
	rm -f server client
	find . -name "*.txt" ! -name "words.txt" -type f -delete