all: cpubus

CXX?=g++

OBJS=common.o cpu.o shared.o ubus_cmds.o ubus_node.o ubus.o cmdline.o loop.o main.o
LIBS=-lubox -lblobmsg_json -lubus
INCLUDES=-I./include -I.

common.o: common.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

cpu.o: cpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

shared.o: shared.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus_cmds.o: ubus_cmds.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus_node.o: ubus_node.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus.o: ubus.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

cmdline.o: cmdline.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

loop.o: loop.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

cpubus: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@;

clean:
	rm -f *.o cpubus
