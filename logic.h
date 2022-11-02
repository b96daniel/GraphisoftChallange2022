#ifndef LOGIC_H
#define LOGIC_H

#include "action.h"
#include "map.h"
#include <vector>
#include <chrono>

#define DEBUG 1

class Logic {
	void reset();

public:
	Infos& infos{ Infos::get_instance() };
	Map map;
	std::vector<std::pair<int, int>> units;

	std::vector<std::string> get_next_actions(std::chrono::steady_clock::time_point start);
	void init();
};

#endif
