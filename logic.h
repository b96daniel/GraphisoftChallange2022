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
	float get_income_goal();
	float get_economic_value(Field& field, int gold_mod, int income_mod);
	float get_offense_value(Field* field, Field::Type unit_type);
	float get_defense_value(Field* field, int self_defense);
	float get_move_offense_value(Field* from_field, Field* to_field);
	float get_move_defense_value(Field* from_field, Field* to_field);
	float get_inline_move_value(Field* from_field, Field* to_field);
public:
	// Public variables
	Map map;

	// Public functions
	std::vector<std::string> get_next_actions(std::chrono::steady_clock::time_point start);
};

#endif
