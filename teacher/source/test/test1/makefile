CFLAG= -std=c++11 -I ../../../build/release-install-cpp11/include/
LFLAG= -L../../../build/release-install-cpp11/lib  -ljsoncpp -lmuduo_net -lmuduo_base -pthread
all : client server
server : test_server.cpp
	g++  -g $(CFLAG) $^ -o $@ $(LFLAG)
client : test_client.cpp
	g++  -g $(CFLAG) $^ -o $@ $(LFLAG)