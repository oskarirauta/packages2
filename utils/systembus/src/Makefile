all: systembus

CXX?=g++

OBJS:=common.o cpu.o security.o info.o shared.o ubus_cpu.o ubus_security.o ubus_info.o ubus.o cmdline.o loop.o main.o
LIBS:=-lubox -lblobmsg_json -lubus -luci
INCLUDES:=-I./include -I.

ifdef selinux
LIBS+= -lselinux
endif

common.o: common.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

cpu.o: cpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

security.o: security.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

info.o: info.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

shared.o: shared.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus_cpu.o: ubus_cpu.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus_security.o: ubus_security.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus_info.o: ubus_info.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

ubus.o: ubus.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

cmdline.o: cmdline.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

loop.o: loop.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

systembus: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@;

clean:
	rm -f *.o systembus
