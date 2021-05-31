#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "common.h"
#include "cpu.h"

cpu_node::cpu_node(int id, std::string name, std::string vendor, std::string model, int mhz, float load) {

	this -> id = id;
	this -> name = name;
	this -> vendor = vendor;
	this -> model = model;
	this -> mhz = mhz;
	this -> load = load;
}

void cpu_node::update(float load) {

	this -> load = load;
}

cpu_t::cpu_t(void) {

        this -> count = 0;
        this -> load = 0;
	this -> delay = 100;

	std::string line;
	std::vector<std::string> name;
	std::vector<std::string> vendor;
	std::vector<std::string> model;
	std::vector<int> mhz;

	std::ifstream statfd("/proc/stat");

	while ( std::getline(statfd, line)) {
		std::vector<std::string> words = common::split(common::to_lower(line), ' ');
		if ( words.empty() || words[0] == "cpu" || !common::has_prefix(words[0], "cpu"))
			continue;
		this -> count++;
		name.push_back(words[0]);
	}

	statfd.close();

	this -> tcks[0] = (tck_t *)malloc(sizeof(tck_t) * ( this -> count + 1));
	this -> tcks[1] = (tck_t *)malloc(sizeof(tck_t) * ( this -> count + 1));

	std::ifstream cpuinfo("/proc/cpuinfo");
	int n = 0;
	int prev_mhz = 0;

	while ( std::getline(cpuinfo, line) && n < this -> count) {

		std::vector<std::string> words = common::split(common::to_lower(line), ':', "\t");

		if ( words.size() < 2 ) continue;
		switch ( common::hash(words[0].c_str())) {
			case common::hash("bogomips"): n++; break;
			case common::hash("vendor_id"): vendor.push_back(words[1]); break;
			case common::hash("model name"): model.push_back(words[1]); break;
			case common::hash("cpu mhz"):
				int value = 0;
				try { value = stoi(words[1]); } catch(...) { value = prev_mhz; }
				mhz.push_back(value); prev_mhz = value;
				break;
		}
	}

	cpuinfo.close();

	while ( name.size() < this -> count ) name.push_back("cpu" + std::to_string(name.size()));
	while ( vendor.size() < this -> count ) vendor.push_back("unknown");
	while ( model.size() < this -> count ) model.push_back("unknown");
	while ( mhz.size() < this -> count ) mhz.push_back(prev_mhz);

	n = 0;
	while ( this -> nodes.size() < this -> count ) {
		this -> nodes.push_back(
			cpu_node(
				n, 
				name[n],
				vendor[n],
				model[n],
				mhz[n], 0));
		n++;
	}

	this -> index = 0;
	this -> read_cpustat(this -> tcks[index]);

	std::this_thread::sleep_for(std::chrono::milliseconds(this -> delay));
	this -> update();
	std::this_thread::sleep_for(std::chrono::milliseconds(this -> delay));
	this -> update();
}

void cpu_t::update(void) {

	this -> read_cpustat(this -> tcks[this -> index]);
	this -> calculate_cpuload(this -> tcks[this -> index^1], this -> tcks[this -> index]);
	this -> index ^= 1;
}

int cpu_t::indexOf(std::string name) {

	std::string _name = common::to_lower(name);
	auto it = std::find_if(this -> nodes.begin(), this -> nodes.end(),
			[_name](const cpu_node& node) {
				return node.name == _name;
			});

	return it == this -> nodes.end() ? 0 : (( it - this -> nodes.begin() + 1));
}

void cpu_t::calculate_cpuload(tck_t *prev, tck_t *curr) {

	uint64_t total, idle, active;
	float load;

	for ( int i = 0; i <= this -> count; i++) {
		total = this -> total_ticks(curr + i) - this -> total_ticks(prev + i);
		idle = this -> idle_ticks(curr + i) - this -> idle_ticks(prev + i);
		active = total - idle;
		load = active * 100.f / total;
		if ( i == 0 ) this -> load = load;
		else this -> nodes[i-1].update(load);
	}
}

void cpu_t::read_cpustat(tck_t *stat) {

	std::ifstream statfd("/proc/stat");
	std::string line;
	int n = 0;

	while ( std::getline(statfd, line) && n <= this -> count ) {

		if ( !common::has_prefix(common::to_lower(line), "cpu")) continue;
		std::vector<std::string> words = common::split(line, ' ');

		std::string values = words[1] + " " + words[2] + " " + words[3] + " " + words[4] + " " +
			words[5] + " " + words[6] + " " + words[7] + " " + words[8] + " " +
			words[9] + " " + words[10];

		std::istringstream iss(values);
		iss >> stat[n].tcks[TCK_USER] >> stat[n].tcks[TCK_NICE] >> stat[n].tcks[TCK_SYSTEM] >>
			stat[n].tcks[TCK_IDLE] >> stat[n].tcks[TCK_IOWAIT] >> stat[n].tcks[TCK_IRQ] >>
			stat[n].tcks[TCK_SOFTIRQ] >> stat[n].tcks[TCK_STEAL] >> 
			stat[n].tcks[TCK_GUEST] >> stat[n].tcks[TCK_GUEST_NICE];

		n++;
	}

	statfd.close();
}

uint64_t cpu_t::idle_ticks(tck_t *stat) {
	return stat -> tcks[TCK_IDLE] + stat -> tcks[TCK_IOWAIT];
}

uint64_t cpu_t::total_ticks(tck_t *stat) {

	uint64_t total = 0;

	for ( int i = 0; i < NUM_TCK_TYPES; i++) total += stat -> tcks[i];
	return total;
}
