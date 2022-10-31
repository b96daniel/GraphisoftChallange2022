#ifndef LOGIC_H
#define LOGIC_H

#include "action.h"
#include "map.h"
#include <vector>

class Logic {
	float unit_buy_goal_funciton();
	
public:
	Map map;

	std::vector<Action> get_next_actions();
	void init();
};

#endif
