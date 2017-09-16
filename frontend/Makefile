TARGETS_MAC = mc
TARGETS_VM = mcvm
FLAGS = -Iinclude --std=c++11 -g 
mac: $(TARGETS_MAC)
vm: $(TARGETS_VM)

%.o: %.cc
	g++ $^ $(FLAGS) -c -o $@ 

mc: http2.o MasterConfig.o
	g++ $^ -o $@ -lboost_serialization -lboost_system-mt

mcvm: http2.o MasterConfig.o
	g++ $^ -o $@ -lpthread -lboost_serialization -lboost_system -lsfml-network -lsfml-system -lsfml-window

clean:
	rm -fv $(TARGETS_MAC) $(TARGETS_VM) *~ *.o
