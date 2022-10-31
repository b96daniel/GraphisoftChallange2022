#include "map.h"

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
    fields = std::vector(2 * infos.radius + 1, std::vector(2 * infos.radius + 1, Field()));
    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        for (int j = 0; j < static_cast<int>(fields.size()); ++j) {
            fields[i][j].pos = {i - offset, j - offset};
        }
    }

    offset = infos.radius;
}

void Map::set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water) {
    Field& current_field = fields[pos.first + offset][pos.second + offset];

    current_field.water = water;
    if (current_field.water) return;

    current_field.value = value;
    
    if (current_field.owner != owner) {
        if (owner == infos.id) own_fields.push_back(current_field);
        // TODO: Check remove
        else if (current_field.owner == infos.id)
            own_fields.erase(std::find_if(own_fields.begin(),
                own_fields.end(),
                [&current_field](std::reference_wrapper<Field> f) { return f.get() == current_field; }));

        current_field.owner = owner;
    }

    int current_type = Field::get_type_from_str(type_str);
    current_field.type = current_type;
}