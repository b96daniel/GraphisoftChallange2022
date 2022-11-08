#include "logic.h"
#include "action.h"
#include "constants.h"
#include <math.h>
#include <map>
#include <deque>

#define TIMEOUT 1.7

// TODOs
// TODO: Add time check to get_next_actions
// TODO: Discuss fields reset with the team
// TODO: Game can stuck if there is no unit and there is a building on every field

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
				curr_buy.value += get_economic_value(*current_field, cost, Field::get_income(Field::FARM));
				curr_buy.value -= static_cast<float>(current_field->distance(*infos.castle)) / infos.radius;

				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}

	cost = map.get_cost(Field::TOWER);
	if (infos.gold >= cost && ((map.income + Field::get_income(Field::TOWER)) > get_income_goal())) {
		curr_buy.type = Field::TOWER;
		for (const auto& current_field : map.own_fields) {
			if (current_field->type == Field::EMPTY || current_field->type == Field::GRAVE) {
				curr_buy.pos = current_field->pos;

				curr_buy.value = 0;
				int tower_cover = map.get_tower_cover(current_field);
				if (tower_cover) {
					curr_buy.value += get_economic_value(*current_field, cost, Field::get_income(Field::TOWER));
					curr_buy.value += tower_cover;
				}
				else
					curr_buy.value = Action::MIN_VALUE;

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
				int tower_cover = map.get_tower_cover(current_field);
				if ( ((map.income + (Field::get_income(Field::FORT) - Field::get_income(current_field->type))) > get_income_goal())
					&& tower_cover ) {
					curr_buy.value = 0;
					curr_buy.value += get_economic_value(*current_field, cost, Field::get_income(Field::FORT) - Field::get_income(current_field->type));
					curr_buy.value += tower_cover;
				}
				// Invalidate decision
				else curr_buy.value = Action::MIN_VALUE;

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
				
					int merged_type = type;
					int income = Field::get_income(merged_type);

					if (current_field->type >= Field::PEASANT && current_field->type <= Field::KNIGHT) {
						merged_type = Field::get_merged_type(type, current_field->type);
						income = Field::get_income(merged_type) - Field::get_income(current_field->type);
					}

					if ((map.income + income) > get_income_goal()) {
						curr_buy.value = 0;
						curr_buy.value += get_economic_value(*current_field, cost, income);
						curr_buy.value += get_offense_value(current_field, static_cast<Field::Type>(merged_type));
					}
					// Invalidate decision
					else curr_buy.value = Action::MIN_VALUE;

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

			map.iterate_neighbours(*current_field, [unit, &visited, &not_visited, current_field, this](Field& neighbour) {
				if (visited.find(&neighbour) == visited.end()) {
					/* Can step on this field or can step through it */ 
					if (!neighbour.water &&
						map.neighbours_detected(neighbour) &&
						((neighbour.owner == infos.id) || (unit->get_offense() > map.get_defense(&neighbour))))
					{ 
						visited[&neighbour] = visited[current_field] + 1;
						if ((visited[&neighbour] < 4) && (neighbour.owner == infos.id)) /* Can step further */
							not_visited.push_back(&neighbour);
					}
				}
			});
		}

		// Possible endpoints for movement
		std::vector<Field*> endpoints;
		for (const auto& value : visited) {
			if (value.first->owner == infos.id) {
				if (value.first->type <= Field::KNIGHT && value.first->type >= Field::PEASANT) {
					// Merge check
					if (unit == value.first) endpoints.push_back(value.first);
					else if (Field::get_merged_type(unit->type, value.first->type) > -1) {
						int income = Field::get_income(Field::get_merged_type(unit->type, value.first->type)) - Field::get_income(value.first->type);
						if ((map.income + income) > get_income_goal()) endpoints.push_back(value.first);
					}
				}
				else if (value.first->type == Field::GRAVE && value.first->type <= Field::PALM) {
					// Avoid all our buildings
					endpoints.push_back(value.first);
				}
			}
			else {
				// else enemy - already checked we have enough offense
				endpoints.push_back(value.first);
			}			
		}

		for (const auto& field_to : endpoints) {
			current_move.to_pos = field_to->pos;
			current_move.value = 0;
			if (field_to->owner != infos.id) current_move.value = 1;
			if (current_move.value > move.value) move = current_move;
		}
	}
}

// Applies the decesion on the internal implementation caused by the buy action
void Logic::apply_buy(Buy& buy, std::vector<Field*>& moveable_units) {
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
		else {
			map.units.push_back(&current_field);
			moveable_units.push_back(&current_field);
		}
		map.income += Field::get_income(new_type);
		current_field.type = static_cast<Field::Type>(new_type);
		break;

	default:
		break;
	}
}

// Applies the decesion on the internal implementation caused by the move action
void Logic::apply_move(Move& move, std::vector<Field*>& moveable_units) {
	Field& from_field = map.get_field(move.from_pos);
	Field& to_field = map.get_field(move.to_pos);

	if (from_field.pos == std::pair<int, int>{1, 6} && to_field.pos == std::pair<int, int>{4, 4}) {
		map.iterate_neighbours(to_field, [](Field& n) {
			std::cerr << "Neighbour:" << n.pos.first << " " << n.pos.second << " " << n.type << "\n";
			});
	}

	moveable_units.erase(std::find(moveable_units.begin(), moveable_units.end(), &from_field));
	// Movements in the empire
	if (to_field.owner == infos.id) {
		map.income -= Field::get_income(to_field.type);
		// Merge moves
		if (from_field.type <= Field::KNIGHT && from_field.type >= Field::PEASANT) {
			int merged_type = Field::get_merged_type(from_field.type, to_field.type);
			map.income += Field::get_income(merged_type) - Field::get_income(from_field.type);
			to_field.type = static_cast<Field::Type>(merged_type);
			map.units.erase(std::find(map.units.begin(), map.units.end(), &from_field));
		}

		// Not merge moves
		else {
			to_field.type = from_field.type;
			*std::find(map.units.begin(), map.units.end(), &from_field) = &to_field;
		}
	}

	// Conquering moves
	else {
		*std::find(map.units.begin(), map.units.end(), &from_field) = &to_field;
		to_field.owner = infos.id;
		to_field.type = from_field.type;
		map.own_fields.push_back(&to_field);
		map.income += to_field.value;
	}
	if (to_field.type == Field::PALM) infos.gold += 6;
	else if (to_field.type == Field::PINE) infos.gold += 3;
	from_field.type = Field::EMPTY;
}

float Logic::get_income_goal() {
	return Constants::GOAL_INCOME_M * sqrtf(infos.tick);
}

float Logic::get_economic_value(Field& field, int gold_mod, int income_mod) {
	float goal_income = get_income_goal();
	// float goal_gold = Constants::GOAL_GOLD_M * sqrtf(infos.tick);
	float ret = 0;

	if (field.type == Field::PINE) {
		++income_mod;
		gold_mod += 3;
	}
	else if (field.type == Field::PALM) {
		++income_mod;
		gold_mod += 6;
	}

	if (goal_income - map.income > 0) ret += (goal_income - map.income) * income_mod;
	else ret += income_mod;

	// if (goal_gold - infos.gold > 0) ret += (goal_gold - infos.gold) * gold_mod;
	// else ret += gold_mod;
	ret += gold_mod;

	return ret;
}

float Logic::get_offense_value(Field* field, Field::Type unit_type) {
	std::map<Field*, int> visited;
	std::deque<Field*> not_visited;
	visited[field] = 0;
	not_visited.push_back(field);

	while (!not_visited.empty()) {
		Field* current_field = not_visited.front();
		not_visited.pop_front();

		map.iterate_neighbours(*current_field, [unit_type, field, &visited, &not_visited, current_field, this](Field& neighbour) {
			if (visited.find(&neighbour) == visited.end()) {
				/* Can step on this field or can step through it */
				if (!neighbour.water &&
					((neighbour.owner == infos.id) || (Field::get_offense(unit_type) > map.get_defense(&neighbour))))
				{
					visited[&neighbour] = visited[current_field] + 1;
					if ((visited[&neighbour] < 4) && (neighbour.owner == infos.id)) /* Can step further */
						not_visited.push_back(&neighbour);
				}
			}
			});
	}
	
	int sum_value = 0;
	for (const auto& value : visited) {
		if (!(value.first->owner == infos.id)) {
			sum_value += value.first->value;
		}
	}

	return Constants::OFFENSE_VALUE_M * sum_value;
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
			apply_buy(best_buy, moveable_units);
		}
		else if (best_move.value > Action::MIN_VALUE) {
			if (best_move.from_pos == best_move.to_pos)
				moveable_units.erase(std::find(moveable_units.begin(), moveable_units.end(), &map.get_field(best_move.to_pos)));
			else {
				result.push_back(best_move.str());
				apply_move(best_move, moveable_units);
			}
		}
		// If no reasonable decesion was found, then break loop
		else break;

		std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
		if (process_seconds.count() > TIMEOUT) break;
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