#include "logic.h"
#include "action.h"

// TODO: Add time check
std::vector<std::string> Logic::get_next_actions(std::chrono::steady_clock::time_point start) {
	std::vector<std::string> result;
	for (const auto& pos: map.own_fields) {
		map.iterate_neighbours(pos, [this](std::pair<int, int> n_pos) {
			Field& current_field = map.get_field(n_pos);
			if (current_field.owner != infos.id) this->map.neighbouring_fields.insert(n_pos);
		});
	}

	for (auto& unit : units) {
		map.iterate_neighbours(unit, [&result, &unit, this](std::pair<int, int> n_pos) {
			Field& current_field = map.get_field(n_pos);
			if (current_field.owner != infos.id && !current_field.water && result.empty()) {
				result.push_back(Move(unit, n_pos).str());
				unit = n_pos;
			}
			});
	}

	if (result.empty()) {
		for (const auto& pos : map.own_fields) {
			map.iterate_neighbours(pos, [&result, &pos, this](std::pair<int, int> n_pos) {
				Field& current_field = map.get_field(n_pos);
				if (current_field.owner != infos.id && result.empty() && current_field.type == Field::EMPTY) {
					result.push_back(Buy(n_pos, Buy::PEASANT).str());
					units.push_back(n_pos);
				}
				});
		}
	}

#if DEBUG
	std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
	std::cerr << "[logic] Process took: " << process_seconds.count() << " seconds\n";
#endif

	reset();
	return result;
}

void Logic::reset() {
	map.reset();
}

void Logic::init() {
	map.init();
}