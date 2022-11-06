#ifndef LOGIC_H
#define LOGIC_H

#include "action.h"
#include "map.h"

#include <vector>
#include <chrono>

class Logic {
	// Private variables
	Infos& infos{ Infos::get_instance() };

	// Private functions
	void check_buy(Buy& buy);
	void check_move(Move& move, std::vector<Field*>& moveable_units);
	void apply_buy(Buy& buy, std::vector<Field*>& moveable_units);
	void apply_move(Move& move, std::vector<Field*>& moveable_units);
public:
	// Public variables
	Map map;

	// Public functions
	std::vector<std::string> get_next_actions(std::chrono::steady_clock::time_point start);

	/*
	void calculate_neighbouring_fields();
	float get_economic_value(std::pair<int, int> pos, int gold_mod, int income_mod);
	float get_threat_value(std::pair<int, int> pos, int self_defense);
	float get_defense_value(std::pair<int, int> pos, int self_defense);
	*/
};

#endif
