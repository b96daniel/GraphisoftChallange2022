#include "map.h"
#include <algorithm>
#include <array>

// TODO: Timer started from solver -> Early return in get next actions

int Field::get_type_from_str(std::string& type_str) {
    if (type_str == "EMPTY") return EMPTY;
    else if (type_str == "PINE") return PINE;
    else if (type_str == "PALM") return PALM;
    else if (type_str == "CASTLE") return CASTLE;
    else if (type_str == "FARM") return FARM;
    else if (type_str == "TOWER") return TOWER;
    else if (type_str == "FORT") return FORT;
    else if (type_str == "PEASANT") return PEASANT;
    else if (type_str == "SPEARMAN") return SPEARMAN;
    else if (type_str == "SWORDSMAN") return SWORDSMAN;
    else if (type_str == "KNIGHT") return KNIGHT;
    else if (type_str == "GRAVE") return GRAVE;
    else return EMPTY;
}

Field::Field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water)
    : pos{ pos }, value{ value }, owner{ owner }, water{ water }, type{get_type_from_str(type_str)}
{
}

bool Field::operator==(const Field& f) {
    return pos == f.pos;
}

int Field::get_income(int type) {
    switch (type)
    {
    case EMPTY:
    case CASTLE:
    case GRAVE:
        return 0;

    case PINE:
    case PALM:
    case TOWER:
        return -1;

    case FARM:
        return 4;

    case FORT:
    case SPEARMAN:
        return -6;

    case PEASANT:
        return -2;

    case SWORDSMAN:
        return -18;

    case KNIGHT:
        return -36;
    default:
        return 0;
    }
};

int Field::get_defense(int type) {
    switch (type)
    {
    case EMPTY:
    case GRAVE:
    case FARM:
    case PINE:
    case PALM:
        return 0; 

    case PEASANT:
        return 1;

    case TOWER:
    case SPEARMAN:
        return 2;

    case FORT:
    case KNIGHT:
    case SWORDSMAN:
        return 3;

    case CASTLE:
        return 5;

    default:
        return 0;
    }
};

void Map::init() {
    fields = std::vector(2 * infos.radius + 1, std::vector(2 * infos.radius + 1, Field()));
    threat_levels = std::vector(2 * infos.radius + 1, std::vector(2 * infos.radius + 1, 0));
    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        for (int j = 0; j < static_cast<int>(fields.size()); ++j) {
            fields[i][j].pos = {i - infos.radius, j - infos.radius };
        }
    }
}

void Map::reset() {
    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        for (int j = 0; j < static_cast<int>(fields.size()); ++j) {
            threat_levels[i][j] = 0;
        }
    }

    own_fields.clear();
    neighbouring_fields.clear();
    income = 0;
}

void Map::set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water) {
    Field& current_field = fields[pos.first + infos.radius][pos.second + infos.radius];

    current_field.water = water;
    current_field.value = value;
    current_field.owner = owner;
    int current_type = Field::get_type_from_str(type_str);
    current_field.type = current_type;

    if (owner == infos.id) {
        own_fields.push_back(current_field.pos);

        income += value + current_field.get_income(current_type);
    }
    else if (current_type >= Field::PEASANT && current_type <= Field::KNIGHT) {
        set_threat(current_field.pos, (current_type - Field::PEASANT) + 1);
    };
}

Field& Map::get_field(std::pair<int, int> pos) {
    return fields[pos.first + infos.radius][pos.second + infos.radius];
}

void Map::iterate_neighbours(std::pair<int, int> pos, const std::function<void(std::pair<int, int>)>& func) {
    int q = pos.first;
    int r = pos.second;

    // TODO: Can be restructured
    if (q - 1 >= -infos.radius - std::min(0, r)) func({ q - 1, r});
    if (q + 1 <= infos.radius - std::max(0, r)) func({ q + 1, r});
    if (r - 1 >= -infos.radius - std::min(0, q)) func({ q, r - 1});
    if (r + 1 <= infos.radius - std::max(0, q)) func({ q, r + 1});

    if (q - 1 >= -infos.radius - std::min(0, r) &&
        r + 1 <= infos.radius - std::max(0, q)) func({ q - 1, r + 1});
       
    if (q + 1 <= infos.radius - std::max(0, r) &&
        r - 1 >= -infos.radius - std::min(0, q)) func({ q + 1, r - 1});
}

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
        return 2 + 4 * static_cast<int>(units.size());
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

int Field::get_merged_type(int add_type, int base_type) {
    if (add_type == PEASANT && base_type == PEASANT) return SPEARMAN;
    else if (add_type == SPEARMAN && base_type == SPEARMAN) return KNIGHT;
    else if (add_type == PEASANT && base_type == SPEARMAN) return SWORDSMAN;
    else if (add_type == SPEARMAN && base_type == PEASANT) return SWORDSMAN;
    else if (add_type == PEASANT && base_type == SWORDSMAN) return KNIGHT;
    else if (add_type == SWORDSMAN && base_type == PEASANT) return KNIGHT;
    
    return -1;
}

int Map::get_defense(std::pair<int, int> pos) {
    int defense = 0;
    iterate_neighbours(pos, [this, &defense](std::pair<int, int> n_pos) {
        int n_defense = Field::get_defense(get_field(n_pos).type);
        if (defense < n_defense) defense = n_defense;
        });
    return defense;
}