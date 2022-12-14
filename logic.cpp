#include "logic.h"
#include "action.h"
#include "constants.h"
#include <math.h>
#include <map>
#include <deque>
#include <algorithm>

#define TIMEOUT 1.7

// TODOs
// TODO: Plus point for merging into a moveable unit
// TODO: Minus point for standing on the border
// TODO: Clustering
// TODO: Trees

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
				curr_buy.value += get_economic_value(*current_field, -cost, Field::get_income(Field::FARM));
				curr_buy.value -= static_cast<float>(current_field->distance(*infos.castle)) / infos.radius;
				map.iterate_neighbours(*current_field, [&curr_buy](Field& n) {
					if ((n.type >= Field::CASTLE && n.type <= Field::FORT) || n.water) curr_buy.value -= 0.1;
					});
				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}

	/*
	cost = map.get_cost(Field::TOWER);
	if (infos.gold >= cost && ((map.income + Field::get_income(Field::TOWER)) > get_income_goal())) {
		curr_buy.type = Field::TOWER;
		for (const auto& current_field : map.own_fields) {
			float defense_value = get_defense_value(current_field, Field::get_defense(Field::TOWER));
			if ((current_field->type == Field::EMPTY || current_field->type == Field::GRAVE)
				&& defense_value) {
				curr_buy.pos = current_field->pos;

				curr_buy.value = 0;
				curr_buy.value += get_economic_value(*current_field, -cost, Field::get_income(Field::TOWER));
				curr_buy.value += defense_value;
				curr_buy.value += map.get_tower_cover(current_field, Field::TOWER);

				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}

	cost = map.get_cost(Field::FORT);
	if (infos.gold >= cost) {
		curr_buy.type = Field::FORT;
		for (const auto& current_field : map.own_fields) {
			float defense_value = get_defense_value(current_field, Field::get_defense(Field::FORT));
			if ((current_field->type == Field::EMPTY || current_field->type == Field::GRAVE || current_field->type == Field::TOWER)
				&& defense_value 
				&& ((map.income + (Field::get_income(Field::FORT) - Field::get_income(current_field->type))) > get_income_goal())) {
				curr_buy.pos = current_field->pos;
				curr_buy.value = 0;
				curr_buy.value += get_economic_value(*current_field, cost, Field::get_income(Field::FORT) - Field::get_income(current_field->type));
				curr_buy.value += defense_value;
				curr_buy.value += map.get_tower_cover(current_field, Field::FORT);

				if (curr_buy.value > buy.value) buy = curr_buy;
			}
		}
	}
	*/

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

					float defense_value = get_defense_value(current_field, Field::get_offense(static_cast<Field::Type>(merged_type)));
					float offense_value = get_offense_value(current_field, static_cast<Field::Type>(merged_type));
					//float offense_value = 0;
					if ((map.income + income) > 0 && map.income > get_income_goal() && (defense_value > 0 || offense_value > 0)) {
						curr_buy.value = 0;
						curr_buy.value += get_economic_value(*current_field, -cost, income);
						curr_buy.value += offense_value;
						curr_buy.value += defense_value;
						curr_buy.value += static_cast<float>(current_field->distance(*infos.castle)) / infos.radius;
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
		std::vector<Field*> enemy_endpoints;
		for (const auto& value : visited) {
			if (value.first->owner == infos.id) {
				if (value.first->type <= Field::KNIGHT && value.first->type >= Field::PEASANT) {
					// Merge check
					if (unit == value.first) endpoints.push_back(value.first);
					else if (Field::get_merged_type(unit->type, value.first->type) > -1) {
						int income = Field::get_income(Field::get_merged_type(unit->type, value.first->type)) - Field::get_income(value.first->type);
						if ((map.income + income) > 0 && map.income > get_income_goal()) endpoints.push_back(value.first);
					}
				}
				else if (value.first->type == Field::GRAVE || value.first->type <= Field::PALM) {
					// Avoid all our buildings
					endpoints.push_back(value.first);
				}
			}
			else {
				// else enemy - already checked we have enough offense
				enemy_endpoints.push_back(value.first);
			}			
		}

		if (!enemy_endpoints.empty()) {
			for (const auto& field_to : enemy_endpoints) {
				current_move.to_pos = field_to->pos;
				current_move.value = 0;
				current_move.value += get_move_offense_value(unit, field_to);
				if (current_move.value > move.value) move = current_move;
			}
			return;
		}
		else {
			bool should_search = true;
			for (const auto& field_to : endpoints) {
				current_move.to_pos = field_to->pos;
				current_move.value = 0;
				current_move.value += get_inline_move_value(unit, field_to);
				current_move.value += get_economic_value(*field_to, 0, 0);
				if (current_move.value > move.value) move = current_move;
				
				should_search &= (current_move.value == 0);
			}

			if (should_search) {
				Field* dangerours_field = unit;
				int max_diff = std::max(unit->get_threat() - map.get_defense(unit), 0);
				int max_threat = unit->get_threat();
				int diff;
				for (const auto& f : map.own_fields) {
					if (f->type == Field::PALM || f->type == Field::PINE) {
						dangerours_field = f;
						break;
					}
					diff = f->get_threat() - map.get_defense(f);
					if (max_diff == 0) {
						if (f->get_threat() > max_threat) {
							max_threat = f->get_threat();
							dangerours_field = f;
						}
					}
					else if (diff > max_diff) {
						max_diff = diff;
						dangerours_field = f;
					}
				}
				std::map<Field*, int> open;
				std::map<Field*, int> closed;
				closed[unit] = 0;
				bool goal_found = false;
				map.iterate_neighbours(*unit, [&open, this, dangerours_field, &goal_found, &unit, &closed](Field& n) {
					if (!goal_found) {
						if (n.owner == infos.id) {
							if (&n == dangerours_field) {
								goal_found = true;
								closed[&n] = 1;
							}
							else open[&n] = 1;
						}
					}
					});

				while (!open.empty() && !goal_found) {
					Field* current_field = std::min_element(open.begin(), open.end(),
						[&dangerours_field](const auto& a, const auto& b) {
							return a.first->distance(*dangerours_field) + a.second < b.first->distance(*dangerours_field) + b.second;
						})->first;

					map.iterate_neighbours(*current_field, [&open, &closed, this, dangerours_field, &goal_found, current_field](Field& n) {
						if (!goal_found) {
							if (n.owner == infos.id) {
								if (&n == dangerours_field) {
									goal_found = true;
									closed[&n] = open[current_field] + 1;
								}
								else {
									if ((open.find(&n) == open.end() || open[&n] > (1 + open[current_field]))
										&& (closed.find(&n) == closed.end() || closed[&n] > (1 + open[current_field]))) {
										open[&n] = 1 + open[current_field];
									}
								}
							}
						}
						});

					closed[current_field] = open[current_field];
					open.erase(current_field);
				}

				std::vector<Field*> path;
				if (goal_found) {
					Field* current_node = dangerours_field;
					path.push_back(current_node);
					while (current_node != unit) {
						map.iterate_neighbours(*current_node, [&current_node, &closed, dangerours_field](Field& n) {
							if (closed.find(&n) != closed.end() && closed[&n] == (closed[current_node] - 1)) {
								current_node = &n;
							}
							});
						path.push_back(current_node);
					}

					current_move.to_pos = unit->pos;
					for (const auto& element : path) {
						if (std::find(endpoints.begin(), endpoints.end(), element) != endpoints.end()) {
							current_move.to_pos = element->pos;
							break;
						}
					}

					current_move.value = 0.1;
				}
				else {
					current_move.value = 0.1;
					current_move.to_pos = unit->pos;
				}
				current_move.value = 0.1;
				move = current_move;
				return;
			}
			else return;
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

	
	moveable_units.erase(std::find(moveable_units.begin(), moveable_units.end(), &from_field));
	// Movements in the empire
	if (to_field.owner == infos.id) {
		map.income -= Field::get_income(to_field.type);
		// Merge moves
		if (to_field.type <= Field::KNIGHT && to_field.type >= Field::PEASANT) {
			int merged_type = Field::get_merged_type(from_field.type, to_field.type);
			map.income += Field::get_income(merged_type) - Field::get_income(from_field.type);
			to_field.type = static_cast<Field::Type>(merged_type);
			map.units.erase(std::find(map.units.begin(), map.units.end(), &from_field));
		}

		// Not merge moves
		else {
			to_field.type = from_field.type;
			map.units.erase(std::find(map.units.begin(), map.units.end(), &from_field));
			map.units.push_back(&to_field);
			// *std::find(map.units.begin(), map.units.end(), &from_field) = &to_field;
		}
	}

	// Conquering moves
	else {
		map.remove_threat(&to_field);
		to_field.owner = infos.id;
		to_field.type = from_field.type;
		map.own_fields.push_back(&to_field);
		map.income += to_field.value;
		map.units.erase(std::find(map.units.begin(), map.units.end(), &from_field));
		map.units.push_back(&to_field);
		// *std::find(map.units.begin(), map.units.end(), &from_field) = &to_field;
	}
	if (to_field.type == Field::PALM) infos.gold += 6;
	else if (to_field.type == Field::PINE) infos.gold += 3;
	from_field.type = Field::EMPTY;
}

float Logic::get_income_goal() {
	return Constants::GOAL_INCOME_M * (sqrtf(infos.tick) - 1);
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
					if ((visited[&neighbour] < 2) && (neighbour.owner == infos.id)) /* Can step further */
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

float Logic::get_defense_value(Field* field, int self_defense) {
	float deffed_fields = 0;
	if (self_defense >= field->get_threat() && map.get_defense(field) < field->get_threat()) ++deffed_fields;
	map.iterate_neighbours(*field, [this, &self_defense, &deffed_fields](Field& f) {
		if (f.owner == infos.id) {
			int n_defense = map.get_defense(&f);
			if (self_defense >= f.get_threat() && n_defense < f.get_threat()) ++deffed_fields;
		}
		});

	return Constants::DEFENSE_VALUE_M * deffed_fields;
}

float Logic::get_move_offense_value(Field* from_field, Field* to_field) {
	if (to_field->owner != infos.id) {
		float common_neighbours = 0;
		map.iterate_neighbours(*from_field, [this, &common_neighbours](Field& f) {
			if (f.owner == infos.id) ++common_neighbours;
		});
		return (common_neighbours
			- (to_field->distance(*infos.castle) / (infos.radius * 10.0))
			+ (static_cast<float>(to_field->get_score()) / 20.0)) * Constants::MOVE_OFFENSE_VALUE_M
			+ get_economic_value(*to_field, 0, to_field->value) / 6;
	}
	return 0;
}

float Logic::get_inline_move_value(Field* from_field, Field* to_field) {
	float ret = 0;
	if (from_field != to_field) {
		int from_prev_defended = 0;
		if (map.is_defended(from_field)) ++from_prev_defended;
		map.iterate_neighbours(*from_field, [this, &from_prev_defended](Field& neighbour) {
			if (neighbour.owner == infos.id && map.is_defended(&neighbour)) ++from_prev_defended;
			});

		// Check number of defended fields after move at from_pos
		int from_next_defended = 0;
		bool is_defended = false;
		map.iterate_neighbours(*from_field, [from_field, this, &is_defended, &from_next_defended](Field& neighbour) {
			if (neighbour.owner == infos.id) {
				is_defended |= (neighbour.get_defense() >= from_field->get_threat());
				bool is_n_defended = (neighbour.get_defense() >= neighbour.get_threat());
				map.iterate_neighbours(neighbour, [this, from_field, &is_n_defended, &neighbour](Field& n) {
					if (&n != from_field) {
						is_n_defended |= (n.owner == infos.id && n.get_defense() >= neighbour.get_threat());
					}
					});
				if (is_n_defended) ++from_next_defended;
			}
			});
		if (is_defended) ++from_next_defended;

		// Check number of defended field after move at to_pos
		float to_defense_change = 0;
		Field::Type new_type = from_field->type;
		if (Field::get_merged_type(from_field->type, to_field->type) > -1)
			new_type = static_cast<Field::Type>(Field::get_merged_type(from_field->type, to_field->type));

		if ((map.get_defense(to_field) < to_field->get_threat()) && (to_field->get_threat() <= Field::get_defense(new_type))) ++to_defense_change;
		map.iterate_neighbours(*to_field, [this, &new_type, &to_defense_change](Field& neighbour) {
			if (neighbour.owner == infos.id
				&& (map.get_defense(&neighbour) < neighbour.get_threat()) && (neighbour.get_threat() <= Field::get_defense(new_type)))
				++to_defense_change;
			});

		ret = to_defense_change + from_next_defended - from_prev_defended;
	}
	
	map.iterate_neighbours(*to_field, [this, &ret](Field& f) {
		if (f.owner != -1 && f.owner != infos.id) ret -= 10;
	});

	/*
	std::map<Field*, int> visited;
	std::deque<Field*> not_visited;
	visited[to_field] = 0;
	not_visited.push_back(to_field);

	Field::Type new_type = from_field->type;
	if (Field::get_merged_type(from_field->type, to_field->type) > -1)
		new_type = static_cast<Field::Type>(Field::get_merged_type(from_field->type, to_field->type));

	while (!not_visited.empty()) {
		Field* current_field = not_visited.front();
		not_visited.pop_front();

		map.iterate_neighbours(*current_field, [from_field, to_field, &visited, &not_visited, current_field, this, &new_type](Field& neighbour) {
			if (visited.find(&neighbour) == visited.end()) {
				// Can step on this field or can step through it
				if (!neighbour.water &&
					((neighbour.owner == infos.id) || (Field::get_offense(new_type) > map.get_defense(&neighbour))))
				{
					visited[&neighbour] = visited[current_field] + 1;
					if ((visited[&neighbour] < 4) && (neighbour.owner == infos.id)) // Can step further
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
	ret += sum_value;
	*/

	return ret;
}


// ----------------
// Public functions
// ----------------

std::vector<std::string> Logic::get_next_actions(std::chrono::steady_clock::time_point start) {
	std::vector<std::string> result;
	std::vector<Field*> moveable_units = map.units;
	bool try_buy = true;

	// Look for the next step while there is a reasonable one or until the limit is reached
	while (static_cast<int>(result.size()) < 1024) {
		/*
		Buy best_buy;
		do {
			best_buy.value = Action::MIN_VALUE;
			check_buy(best_buy);
			if (best_buy.value > Action::MIN_VALUE) {
				result.push_back(best_buy.str());
				apply_buy(best_buy, moveable_units);
			}
			std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
			if (process_seconds.count() > TIMEOUT) break;
		} while (best_buy.value > Action::MIN_VALUE && static_cast<int>(result.size()));

		Move best_move;
		bool changed = false;
		do {
			best_move.value = Action::MIN_VALUE;
			check_move(best_move, moveable_units);
			if (best_move.value > Action::MIN_VALUE) {
				if (best_move.from_pos == best_move.to_pos)
					moveable_units.erase(std::find(moveable_units.begin(), moveable_units.end(), &map.get_field(best_move.to_pos)));
				else {
					changed = true;
					result.push_back(best_move.str());
					apply_move(best_move, moveable_units);
				}
			}
			std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
			if (process_seconds.count() > TIMEOUT) break;
		} while (best_move.value > Action::MIN_VALUE && static_cast<int>(result.size()));

		std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
		if (process_seconds.count() > TIMEOUT) break;
		if (!changed) break;
		*/

		Move best_move;
		Buy best_buy;
		if (try_buy) check_buy(best_buy);
		check_move(best_move, moveable_units);

		if (best_buy.value == Action::MIN_VALUE) try_buy = false;
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
