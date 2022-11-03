#ifndef LOGIC_H
#define LOGIC_H

#include "action.h"
#include "map.h"
#include <vector>
#include <chrono>

#define DEBUG 1

class Logic {
	void apply_buy(Buy& buy);
	void apply_move(Move& move);
	void reset();
	void calculate_neighbouring_fields();
	void check_farm_buy(Buy& result);
	float get_economic_value(std::pair<int, int> pos, int gold_mod, int income_mod);
	float get_threat_value(std::pair<int, int> pos, int self_defense);

public:
	Infos& infos{ Infos::get_instance() };
	Map map;
	std::vector<std::pair<int, int>> units;

	std::vector<std::string> get_next_actions(std::chrono::steady_clock::time_point start);
	void init();
};

#endif
