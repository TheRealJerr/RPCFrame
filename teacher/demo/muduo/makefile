CFLAG= -std=c++11 -I ../../build/release-install-cpp11/include/
LFLAG= -L../../build/release-install-cpp11/lib  -lmuduo_net -lmuduo_base -pthread
all: server client
server: server.cpp
	g++  $(CFLAG) $^ -o $@ $(LFLAG)
client: client.cpp
	g++  $(CFLAG) $^ -o $@ $(LFLAG)