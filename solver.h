#ifndef SOLVER_H_INCLUDED
#define SOLVER_H_INCLUDED

#include <vector>
#include <string>
#include <utility>

class solver {
public:
	void startMessage(const std::vector<std::string>& start_infos);
	std::vector<std::string> processTick(const std::vector<std::string>& infos);
};

#endif // SOLVER_H_INCLUDED
