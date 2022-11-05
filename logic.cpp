#include "logic.h"
#include "action.h"
#include <math.h>
#include <map>
#include <deque>

// TODOs
// TODO: Add time check to get_next_actions
// TODO: Discuss fields reset with the team

// -----------------
// Private functions
// -----------------

// Checks the possible buy options, chooses the best one out of them
void Logic::check_buy(Buy& buy) {
	Buy curr_buy;

	int cost = map.get_cost(Field::FARM);
	if (infos.gold >= cost) {		
		curr_buy.type = Field::FARM;

		for (const auto& current_field : map.own_fields) {
			if (map.is_farmable(current_field)) {
				curr_buy.pos = current_field->pos;
				curr_buy.value = 0;

				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}

	cost = map.get_cost(Field::TOWER);
	if (infos.gold >= cost) {
		curr_buy.type = Field::TOWER;
		for (const auto& current_field : map.own_fields) {
			if (current_field->type == Field::EMPTY || current_field->type == Field::GRAVE) {
				curr_buy.pos = current_field->pos;
				curr_buy.value = 0;

				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}

	cost = map.get_cost(Field::FORT);
	if (infos.gold >= cost) {
		curr_buy.type = Field::FORT;
		for (const auto& current_field : map.own_fields) {
			if (current_field->type == Field::EMPTY || current_field->type == Field::GRAVE 
				|| current_field->type == Field::TOWER) {
				curr_buy.pos = current_field->pos;
				curr_buy.value = 0;

				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}

	for (int type = Field::PEASANT; type <= Field::KNIGHT; ++type) {
		int cost = map.get_cost(type);
		if (infos.gold >= cost) {
			Buy curr_buy;
			curr_buy.type = Field::Type(type);

			for (const auto& current_field : map.own_fields) {
				if (!(current_field->type >= Field::CASTLE && current_field->type <= Field::FORT)) {
					if (current_field->type >= Field::PEASANT && current_field->type <= Field::KNIGHT &&
						Field::get_merged_type(type, current_field->type) == -1)
						continue;

					curr_buy.pos = current_field->pos;
					curr_buy.value = 0;
					int merged_type = type;

					int income = Field::get_income(merged_type);
					if (current_field->type >= Field::PEASANT && current_field->type <= Field::KNIGHT) {
						merged_type = Field::get_merged_type(type, current_field->type);
						income = Field::get_income(merged_type) - Field::get_income(current_field->type);
					}

					if (curr_buy.value > buy.value) buy = curr_buy;
				}
			}
		}
	}
}

// Checks the possible movements of the units, chooses the best one out of them
void Logic::check_move(Move& move, std::vector<Field*>& moveable_units) {
	Move current_move;

	// Check all moveable units
	for (const auto& unit : moveable_units) {
		current_move.from_pos = unit->pos;
		
		// Store reachable fields in visited, with the number of steps needed to reach that field
		std::map<Field*, int> visited;
		std::deque<Field*> not_visited;
		visited[unit] = 0;
		not_visited.push_back(unit);

		while (!not_visited.empty()) {
			Field* current_field = not_visited.front();
			not_visited.pop_front();

			map.iterate_neighbours(*current_field, [&visited, &not_visited, current_field, this](Field& neighbour) {
				if (visited.find(current_field) == visited.end()) {
					if (!neighbour.water) { /* Can step on this field or can step through it */ // <---- Ide azok a feltételek amik kizárják a mezõn átlépés lehetõségét (pl: nagyobb védelem)
						visited[&neighbour] = visited[current_field] + 1;

						if ((visited[&neighbour] < 4) && (neighbour.owner == infos.id)) /* Can step further */ // <---- Ide azok a feltételek amik esetén továbbléphetünk a mezõrõl (Szerintem ez ennyi)
							not_visited.push_back(&neighbour);
					}
				}
			});
		}

		/*
		Ezután a visited-ben azok a mezõk lesznek, amin áthaladhat vagy ráérkezhet a unit.
		Ezeket még le kell ellenõrizni, pl: upgrade, figyelmen kell hagyni az épülettel rendelkezõ mezõket
		*/

		/*
		for (const auto& value : visited) {
			if (value.first->owner != infos.id) {
				current_move.value = 0;
				current_move.to_pos = value.first->pos;
				units.erase(std::find(units.begin(), units.end(), result.from_pos));
				return;
			}
		}
		*/
	}
}

// Applies the decesion on the internal implementation caused by the buy action
void Logic::apply_buy(Buy& buy) {
	Field& current_field = map.get_field(buy.pos);
	infos.gold -= map.get_cost(buy.type);
	map.income -= Field::get_income(current_field.type);
	int new_type;

	switch (buy.type)
	{
	case Field::FARM:
		map.income += Field::get_income(Field::FARM);
		current_field.type = Field::FARM;
		map.farms.push_back(&current_field);
		break;

	case Field::TOWER:
		map.income += Field::get_income(Field::TOWER);
		current_field.type = Field::TOWER;
		break;

	case Field::FORT:
		map.income += Field::get_income(Field::FORT);
		current_field.type = Field::FORT;
		break;

	case Field::PEASANT:
	case Field::SPEARMAN:
	case Field::SWORDSMAN:
	case Field::KNIGHT:
		new_type = buy.type;
		if (current_field.type >= Field::PEASANT && current_field.type <= Field::KNIGHT) {
			new_type = Field::get_merged_type(buy.type, current_field.type);
		}
		else map.units.push_back(&current_field);
		map.income += Field::get_income(new_type);
		current_field.type = static_cast<Field::Type>(new_type);
		break;

	default:
		break;
	}
}

// Applies the decesion on the internal implementation caused by the move action
void Logic::apply_move(Move& move) {

}

// ----------------
// Public functions
// ----------------

std::vector<std::string> Logic::get_next_actions(std::chrono::steady_clock::time_point start) {
	std::vector<std::string> result;
	std::vector<Field*> moveable_units = map.units;

	// Look for the next step while there is a reasonable one or until the limit is reached
	while (static_cast<int>(result.size()) < 1024) {
		Buy best_buy;
		Move best_move;

		check_buy(best_buy);
		check_move(best_move, moveable_units);

		// Choose and apply best action
		if (best_buy.value > best_move.value) {
			result.push_back(best_buy.str());
			apply_buy(best_buy);
		}
		else if (best_move.value > Action::MIN_VALUE) {
			result.push_back(best_move.str());
			apply_move(best_move);
		}
		// If no reasonable decesion was found, then break loop
		else break;
	}

	map.reset();

	std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
	std::cerr << "[logic] Process took: " << process_seconds.count() << " seconds\n";
	for (const auto& res : result) std::cerr << "[logic] Given command: " << res << "\n";

	return result;
}

/*
void Logic::calculate_neighbouring_fields() {
	for (const auto& pos : map.own_fields) {
		map.iterate_neighbours(pos, [this](std::pair<int, int> n_pos) {
			Field& current_field = map.get_field(n_pos);
			if (current_field.owner != infos.id && !current_field.water) map.neighbouring_fields.insert(n_pos);
			});
	}
}

// TODO: Check defense only for own fields
// TODO: Buy units only on lower defense fields
float Logic::get_economic_value(std::pair<int, int> pos, int gold_mod, int income_mod) {
	float goal_income = 5 * sqrtf(infos.tick);
	float goal_gold = 10 * sqrtf(infos.tick);
	float ret = 0;

	Field& current_field = map.get_field(pos);
	if (current_field.type == Field::PINE) {
		++income_mod;
		gold_mod += 3;
	}
	else if (current_field.type == Field::PALM) {
		++income_mod;
		gold_mod += 6;
	}

	// TODO: Tower

	if (goal_income - map.income > 0) ret += (goal_income - map.income) * income_mod;
	else ret += income_mod;

	if (goal_gold - infos.gold > 0) ret += (goal_gold - infos.gold) * gold_mod;
	else ret += income_mod;

	return ret;
}

float Logic::get_threat_value(std::pair<int, int> pos, int self_defense) {
	int defense = map.get_defense(pos);
	if (defense < self_defense) defense = self_defense;

	return defense - map.get_threat(pos);
}

float Logic::get_defense_value(std::pair<int, int> pos, int self_defense) {
	int deffed_fields = 0;
	if (self_defense >= map.get_threat(pos) && map.get_defense(pos) < map.get_threat(pos)) ++deffed_fields;
	map.iterate_neighbours(pos, [this, &self_defense, &deffed_fields](std::pair<int, int> n_pos) {
		int n_defense = map.get_defense(n_pos);
		if (self_defense >= map.get_threat(n_pos) && n_defense < map.get_threat(n_pos)) ++deffed_fields;
		});
	
	return 10.0 * deffed_fields;
}
*/