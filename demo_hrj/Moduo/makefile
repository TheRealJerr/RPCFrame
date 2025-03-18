CFLAG= -std=c++11 -I /home/hrj/projects/RPCFrame/build/release-install-cpp11/include
LFLAG= -L/home/hrj/projects/RPCFrame/build/release-install-cpp11/lib -lmuduo_net -lmuduo_base -lpthread

all:server client

server:server.cpp
	
	g++ $(CFLAG) $^ -o $@ $(LFLAG) 

client:client.cpp
	
	g++ $(CFLAG) $^ -o $@ $(LFLAG)

.PHONY:clean
clean:
	rm server client