#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#define DEBUG 1

#include "logic.h"
#include "infos.h"

#include <vector>
#include <string>
#include <utility>
#include <sstream>

class solver {
	Infos& global_infos{ Infos::get_instance() };
	Logic logic;
	inline int get_next_int(std::stringstream& ss, std::string& substring);
public:
	void startMessage(const std::vector<std::string>& start_infos);
	std::vector<std::string> processTick(const std::vector<std::string>& infos);
};

#endif // SOLVER_H_INCLUDED
