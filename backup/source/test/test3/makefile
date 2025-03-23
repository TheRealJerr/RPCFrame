CFLAG= -std=c++11 -I ../../../build/release-install-cpp11/include/
LFLAG= -L../../../build/release-install-cpp11/lib  -ljsoncpp -lmuduo_net -lmuduo_base -pthread
all : server publish_client subscribe_client
server : server.cpp
	g++  -g $(CFLAG) $^ -o $@ $(LFLAG)
publish_client : publish_client.cpp
	g++  -g $(CFLAG) $^ -o $@ $(LFLAG)
subscribe_client : subscribe_client.cpp
	g++  -g $(CFLAG) $^ -o $@ $(LFLAG)