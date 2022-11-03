#include "logic.h"
#include "action.h"

// TODO: Add time check
std::vector<std::string> Logic::get_next_actions(std::chrono::steady_clock::time_point start) {
	std::vector<std::string> result;
	calc_income();
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

void Logic::calc_income()
{
	income = 0;
	for (const auto& pos : map.own_fields) {
		Field& curr_field = map.get_field(pos);
		switch (curr_field.type) {
		case Field::EMPTY:
			income += curr_field.value;
			break;
		case Field::PINE:
			income += curr_field.value - 1;			
			break;
		case Field::PALM:
			income += curr_field.value - 1;
			break;
		case Field::CASTLE:
			income += curr_field.value;
			break;
		case Field::FARM:
			income += curr_field.value + 4;
			break;
		case Field::TOWER:
			income += curr_field.value - 1;
			break;
		case Field::FORT:
			income += curr_field.value - 6;
			break;
		case Field::PEASANT:
			income += curr_field.value - 2;
			break;
		case Field::SPEARMAN:
			income += curr_field.value - 6;
			break;
		case Field::SWORDSMAN:
			income += curr_field.value - 18;
			break;
		case Field::KNIGHT:
			income += curr_field.value - 36;
			break;
		case Field::GRAVE:
			income += curr_field.value;
			break;
		default:
			break;
		}
	}
}

void Logic::reset() {
	map.reset();
}

void Logic::init() {
	map.init();
}