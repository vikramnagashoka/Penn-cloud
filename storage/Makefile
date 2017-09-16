TARGETS_MAC = tabletmac mastermac
TARGETS_VM = tabletvm mastervm
FLAGS = -Iinclude --std=c++11 -g 
mac: $(TARGETS_MAC)
vm: $(TARGETS_VM)

%.o: %.cc
	g++ $^ $(FLAGS) -c -o $@ 

client: chatclient.o
	g++ $^ -o $@

tabletmac: tablet_server.o tablet_common.o tablet_internal.o MasterConfig.o tablet.o ClientSession.o tablet_thread.o common.o jsoncpp.o
	g++ $^ -o $@ -lpthread -lboost_serialization -lboost_system-mt -lboost_thread-mt 

tabletvm: tablet_server.o tablet_common.o tablet_internal.o MasterConfig.o tablet.o ClientSession.o tablet_thread.o common.o jsoncpp.o
	g++ $^ -o $@ -lpthread -lboost_serialization -lboost_system -lboost_thread 

mastermac: master.o MasterConfig.o master_admin.o common.o jsoncpp.o
	g++ $^ -o $@ -lpthread -lboost_serialization -lboost_system-mt -lboost_thread-mt

mastervm: master.o MasterConfig.o master_admin.o common.o jsoncpp.o
	g++ $^ -o $@ -lpthread -lboost_serialization -lboost_system -lboost_thread

clean:
	rm -fv $(TARGETS_MAC) $(TARGETS_VM) *~ *.o logs/* snapshots/*
