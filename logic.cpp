#include "logic.h"
#include "action.h"
#include <math.h>

void Logic::calculate_neighbouring_fields() {
	for (const auto& pos : map.own_fields) {
		map.iterate_neighbours(pos, [this](std::pair<int, int> n_pos) {
			Field& current_field = map.get_field(n_pos);
			if (current_field.owner != infos.id && !current_field.water) map.neighbouring_fields.insert(n_pos);
			});
	}
}

// TODO: Add time check
std::vector<std::string> Logic::get_next_actions(std::chrono::steady_clock::time_point start) {
	std::vector<std::string> result;
	calculate_neighbouring_fields();

	while (true) {
		Buy best_buy;
		Move best_move;
		
		check_farm_buy(best_buy);

		if (best_buy.value > best_move.value) {
			result.push_back(best_buy.str());
			apply_buy(best_buy);
		}
		else if (best_move.value > Action::MIN_VALUE) {
			result.push_back(best_buy.str());
			apply_move(best_move);
		}
		else break;
	}

	reset();

#if DEBUG
	std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - start;
	std::cerr << "[logic] Process took: " << process_seconds.count() << " seconds\n";
#endif
	for (const auto& res: result) std::cerr << "Given command: " << res << "\n";
	return result;
}

void Logic::reset() {
	map.reset();
}

void Logic::init() {
	map.init();
}

void Logic::check_farm_buy(Buy& result) {
	int cost = 12 + 2 * static_cast<int>(map.farms.size());
	if (infos.gold >= cost) {
		Buy current;
		current.type = Buy::FARM;

		for (const auto& pos : map.own_fields) {
			Field& current_field = map.get_field(pos);
			if (current_field.type == Field::EMPTY || current_field.type == Field::GRAVE) {
				current.pos = pos;
				current.value = 0;

				// Economic effect
				current.value += get_economic_value(pos, -cost ,Field::get_income(Field::FARM));

				// Safety value
				current.value += get_threat_value(pos, Field::get_defense(Field::FARM));

				if (current.value > result.value) result = current;
			}
		}
	}
}

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

	if (goal_income - map.income > 0) ret += (goal_income - map.income) * income_mod;
	else ret += income_mod;

	if (goal_gold - infos.gold > 0) ret += (goal_gold - infos.gold) * gold_mod;
	else ret += income_mod;

	return ret;
}

float Logic::get_threat_value(std::pair<int, int> pos, int self_defense) {
	int defense = 0;
	map.iterate_neighbours(pos, [this, &defense](std::pair<int, int> n_pos) {
		int n_defense = Field::get_defense(map.get_field(n_pos).type);
		if (defense < n_defense) defense = n_defense;
		});
	if (defense < self_defense) defense = self_defense;


	return defense - map.get_threat(pos);
}

void Logic::apply_buy(Buy& buy) {
	switch (buy.type)
	{
	case Buy::FARM:
		infos.gold -= 12 + 2 * static_cast<int>(map.farms.size());
		map.income += Field::get_income(Field::FARM);
		map.farms.push_back(buy.pos);
		map.get_field(buy.pos).type = Field::FARM;
		break;
	default:
		break;
	}
}

void Logic::apply_move(Move& move) {

}