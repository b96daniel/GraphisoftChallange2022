#include "map.h"
#include <algorithm>
#include <array>

// ----------------
// Public functions
// ----------------

void Map::init() {
    // Set the appropriate pos values for the fields
    fields = std::vector(2 * infos.radius + 1, std::vector(2 * infos.radius + 1, Field()));
    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        for (int j = 0; j < static_cast<int>(fields.size()); ++j) {
            fields[i][j].pos = { i - infos.radius, j - infos.radius };
        }
    }
    /*threat_levels = std::vector(2 * infos.radius + 1, std::vector(2 * infos.radius + 1, 0));*/
}

void Map::reset() {
    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        for (int j = 0; j < static_cast<int>(fields.size()); ++j) {
            fields[i][j].owner = -1;
            fields[i][j].type = Field::EMPTY;
        }
    }

    own_fields.clear();
    units.clear();
    farms.clear();
    income = 0;
}

// Returns a field element from the array, based on a position from the in-game coordinate system
Field& Map::get_field(std::pair<int, int> pos) {
    return fields[pos.first + infos.radius][pos.second + infos.radius];
}

// Processes the received field input at the start of the turn
void Map::set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water) {
    Field& current_field = get_field(pos);

    // Overwrite the values of the field
    current_field.value = value;
    current_field.owner = owner;
    current_field.water = water;
    current_field.type = Field::get_type_from_str(type_str);

    if (owner == infos.id) {
        own_fields.push_back(&current_field);
        if (current_field.type == Field::FARM) farms.push_back(&current_field);
        if (current_field.type >= Field::PEASANT && current_field.type <= Field::KNIGHT) units.push_back(&current_field);
        income += value + current_field.get_income(current_field.type);
    }
    /*else if (current_type >= Field::PEASANT && current_type <= Field::KNIGHT) {
        set_threat(current_field.pos, (current_type - Field::PEASANT) + 1);
    };*/
}

bool Map::is_farmable(Field* own_field)
{
    bool res = false;
    if (Field::GRAVE > own_field->type && own_field->type > Field::EMPTY)
    {
        return false;
    }
    else
    {
        iterate_neighbours(own_field->pos, [this, &res](std::pair<int, int> n_pos) {
            Field& curr_field = get_field(n_pos);
            if (curr_field.owner == infos.id &&
                (curr_field.type == Field::FARM || curr_field.type == Field::CASTLE)) res = true;
        });
    }
    return res;
}


// Iterates through the neighbours of a field
void Map::iterate_neighbours(std::pair<int, int> pos, const std::function<void(std::pair<int, int>)>& func) {
    int q = pos.first;
    int r = pos.second;

    if (q - 1 >= -infos.radius - std::min(0, r)) func({ q - 1, r});
    if (q + 1 <= infos.radius - std::max(0, r)) func({ q + 1, r});
    if (r - 1 >= -infos.radius - std::min(0, q)) func({ q, r - 1});
    if (r + 1 <= infos.radius - std::max(0, q)) func({ q, r + 1});

    if (q - 1 >= -infos.radius - std::min(0, r) &&
        r + 1 <= infos.radius - std::max(0, q)) func({ q - 1, r + 1});
       
    if (q + 1 <= infos.radius - std::max(0, r) &&
        r - 1 >= -infos.radius - std::min(0, q)) func({ q + 1, r - 1});
}

int Map::get_numof_peasants()
{      
    int res = 0;
    for (const auto& unit : units)
    {
        if (unit->type == Field::PEASANT)
            ++res;
    }
    return res;
}

int Map::get_cost(int field_type) {
    switch (field_type)
    {
    case Field::FARM:
        return 12 + 2 * static_cast<int>(farms.size());
    case Field::TOWER:
        return 15;
    case Field::FORT:
        return 35;
    case Field::PEASANT:
        return std::max(2 + 4 * get_numof_peasants(), 10);
    case Field::SPEARMAN:
        return 20;
    case Field::SWORDSMAN:
        return 30;
    case Field::KNIGHT:
        return 40;
    default:
        return 0;
    }
}

/*

void Map::process_loss(std::pair<int, int> pos) {
    auto it = std::find(farms.begin(), farms.end(), pos);
    if (it != farms.end()) {
        farms.erase(it);
        return;
    }

    it = std::find(units.begin(), units.end(), pos);
    if (it != units.end()) {
        units.erase(it);
        return;
    }
}

void Map::set_threat(std::pair<int, int> pos, int level) {
    constexpr int depth = 4;
    std::array<std::set<std::pair<int, int>>, depth + 1> buffers;
    buffers[0].insert(pos);

    for (int i = 0; i < depth; ++i) {
        for (const auto& surrounding : buffers[i]) {
            iterate_neighbours(surrounding, [&i, &buffers](std::pair<int, int> s_pos) {buffers[i + 1].insert(s_pos);});
        }
    }

    for (const auto& surrounding : buffers[depth]) {
        int& threat_level = get_threat(surrounding);
        if (threat_level < level) threat_level = level;
    }
}

int& Map::get_threat(std::pair<int, int> pos) {
    return threat_levels[pos.first + infos.radius][pos.second + infos.radius];
}
*/

/*
int Map::get_defense(std::pair<int, int> pos) {
    int defense = 0;
    iterate_neighbours(pos, [this, &defense](std::pair<int, int> n_pos) {
        int n_defense = Field::get_defense(get_field(n_pos).type);
        if (defense < n_defense) defense = n_defense;
        });
    return defense;
}
*/