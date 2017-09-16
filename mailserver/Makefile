TARGETS = master
FLAGS = -Iinclude --std=c++11 -g 
all: $(TARGETS) 

master: master.cc
	g++ $^ -lpthread -g -o $@

clean::
	rm -fv $(TARGETS) *~ *.o
