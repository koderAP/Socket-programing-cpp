# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11 -Wno-pragma-once-outside-header -Wunused-variable -Wno-unused-variable

# Executables
SERVER = server
CLIENT = client


PORT_NUMBER = 8080
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp

.PHONY: build run clean

build:
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o $(SERVER)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o $(CLIENT)

server: 
	$(CXX) $(CXXFLAGS) server.cpp -o server

client: 
	$(CXX) $(CXXFLAGS) client.cpp -o client

run:
	./server &
	sleep 1
	./client

plot:
	python3 test.py

clear_port:
	@echo "Clearing port $(PORT_NUMBER)..."
	@sudo kill -9 $$(sudo lsof -t -i :$(PORT_NUMBER)) || echo "No processes found on port $(PORT_NUMBER)"
	@echo "Port $(PORT_NUMBER) cleared."

test:
	python3 tester.py

clear_txt:
	find . -name "*.txt" ! -name "words.txt" -type f -delete
clean:
	rm -f server client
	find . -name "*.txt" ! -name "words.txt" -type f -delete