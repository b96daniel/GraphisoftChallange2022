#include "solver.h"

#include <iostream>
#include <algorithm>
#include <cmath>

inline int solver::get_next_int(std::stringstream& ss, std::string& substring) {
	ss >> substring;
	return std::stoi(substring);
}

void solver::startMessage(const std::vector<std::string>& start_infos) {
	std::stringstream ss;
	std::string substring;
	int max_tick = 0, radius = 0, palm_spread = 0, pine_spread = 0;

	// Split and process all infos
	for (const auto& info : start_infos) {
#if DEBUG
		std::cerr << info << std::endl;
#endif
		ss << info;
		ss >> substring;
		
		// Handle each necessary info
		if (substring == "MAXTICK") max_tick = get_next_int(ss, substring);
		else if (substring == "RADIUS") radius = get_next_int(ss, substring);
		else if (substring == "PALMSPREAD") palm_spread = get_next_int(ss, substring);
		else if (substring == "PINESPREAD") pine_spread = get_next_int(ss, substring);

		// Clear stringstream
		ss.str("");
		ss.clear();
	}
	
	global_infos.radius = radius;
	logic.init();
}

std::vector<std::string> solver::processTick(const std::vector<std::string>& infos) {
	std::vector<std::string> commands{infos[0]};
	commands[0][2] = 'S';

	std::stringstream ss;
	std::string substring;
	int id = 0, gold = 0, q, r, value, owner;
	bool is_water;

	// Split and process all infos
	for (const auto& info : infos) {
		ss << info;
		ss >> substring;

		// Handle each necessary info
		if (substring == "REQ") {
			for (int i = 0; i < 3; ++i) ss >> substring;
			id = std::stoi(substring);

			global_infos.id = id;
		}
		else if (substring == "WARN") {
			std::cerr << info << std::endl;
		}
		else if (substring == "GOLD") gold = get_next_int(ss, substring);
		else if (substring == "LOSSES") {
			while (!ss.eof()) {
				q = get_next_int(ss, substring);
				r = get_next_int(ss, substring);
			}
		}
		else if (substring == "FIELD") {
			while (!ss.eof()) {
				q = get_next_int(ss, substring);
				r = get_next_int(ss, substring);
				
				ss >> substring;
				is_water = (substring == "1");
				
				value = get_next_int(ss, substring);
				owner = get_next_int(ss, substring);

				// Type
				ss >> substring;

				logic.map.set_field({q, r}, value, owner, substring, is_water);
			}
		}
		else if (substring == "END") {
			std::cerr << info << std::endl;
		}

		// Clear stringstream
		ss.str("");
		ss.clear();
	}

	logic.get_next_actions();

	return commands;
}
