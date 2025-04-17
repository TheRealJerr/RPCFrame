CFLAG= -std=c++17 -I /home/hrj/projects/RPCFrame/build/release-install-cpp11/include
LFLAG= -L/home/hrj/projects/RPCFrame/build/release-install-cpp11/lib -lmuduo_net -lmuduo_base -lpthread -ljsoncpp

all:server PublishClient subscribeClient

server:server.cpp
	
	g++ $(CFLAG) $^ -o $@ $(LFLAG) 

PublishClient:PublishClient.cpp
	
	g++ $(CFLAG) $^ -o $@ $(LFLAG)
subscribeClient:subscribeClient.cpp

	g++ $(CFLAG) $^ -o $@ $(LFLAG)


.PHONY:clean
clean:
	rm server PublishClient subscribeClient
