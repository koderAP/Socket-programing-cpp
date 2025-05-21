# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++11 -Wno-pragma-once-outside-header -Wunused-variable -Wno-unused-variable




SERVER = server
CLIENT = client
# Source files
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp


.PHONY: build run clean


build:
	$(CXX) $(CXXFLAGS) $(SERVER_SRC) -o server
	$(CXX) $(CXXFLAGS) $(CLIENT_SRC) -o client
server:
	$(CXX) $(CXXFLAGS) server.cpp -o server

client:
	$(CXX) $(CXXFLAGS) client.cpp -o client

run:
	./server &
	sleep 1
	./client
plot:
	python3 plot.py

clear_port:
	@echo "Clearing port $(PORT_NUMBER)..."
	@sudo kill -9 $$(sudo lsof -t -i :$(PORT_NUMBER)) || echo "No processes found on port $(PORT_NUMBER)"
	@echo "Port $(PORT_NUMBER) cleared."
clean:
	rm -f server client