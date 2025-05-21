# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Wno-pragma-once-outside-header -Wunused-variable -Wno-unused-variable -Wno-return-type -Wc++17-extensions
PORT_NUMBER = 8080

# Executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.cpp
CLIENT_SRC = client.cpp


build: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $(SERVER) $(SERVER_SRC)

$(CLIENT): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $(CLIENT_SRC)

run-fifo:
	./server FIFO &
	sleep 1
	./client

run-rr:
	./server ROUND_ROBIN &
	sleep 1
	./client

plot:
	python3 analysis.py

fairness:
	python3 test.py

clean_port:
	@echo "Clearing port $(PORT_NUMBER)..."
	@sudo kill -9 $$(sudo lsof -t -i :$(PORT_NUMBER)) || echo "No processes found on port $(PORT_NUMBER)"
	@echo "Port $(PORT_NUMBER) cleared."

test:
	python3 tester.py
clean_txt:
	find . -name "*.txt" ! -name "words.txt" -type f -delete

clean:
	rm -f server client client_single
	find . -name "*.txt" ! -name "words.txt" -type f -delete