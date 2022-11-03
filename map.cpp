#include "map.h"
#include <algorithm>

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

void Map::init() {
    fields = std::vector<std::vector<Field>>(2 * infos.radius + 1, std::vector<Field>(2 * infos.radius + 1, Field()));
    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        for (int j = 0; j < static_cast<int>(fields.size()); ++j) {
            fields[i][j].pos = {i - infos.radius, j - infos.radius };
        }
    }
}

void Map::reset() {
    own_fields.clear();
    neighbouring_fields.clear();
}

void Map::set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water) {
    Field& current_field = fields[pos.first + infos.radius][pos.second + infos.radius];

    current_field.water = water;
    current_field.value = value;
    current_field.owner = owner;
    int current_type = Field::get_type_from_str(type_str);
    current_field.type = current_type;

    if (owner == infos.id) own_fields.push_back(current_field.pos);
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