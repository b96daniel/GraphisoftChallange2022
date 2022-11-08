#include "field.h"
#include <math.h>
#include <string>

// ----------------
// Public functions
// ----------------
Field::Field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water)
    : pos{ pos }, value{ value }, owner{ owner }, water{ water }, type{ get_type_from_str(type_str) } {}

Field::Type Field::get_type_from_str(std::string& type_str) {
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

std::string Field::get_type_str(Type type) {
    switch (type)
    {
    case EMPTY:
        return "EMPTY";
    case PINE:
        return "PINE";
    case PALM:
        return "PALM";
    case CASTLE:
        return "CASTLE";
    case FARM:
        return "FARM";
    case TOWER:
        return "TOWER";
    case FORT:
        return "FORT";
    case PEASANT:
        return "PEASANT";
    case SPEARMAN:
        return "SPEARMAN";
    case SWORDSMAN:
        return "SWORDSMAN";
    case KNIGHT:
        return "KNIGHT";
    case GRAVE:
        return "GRAVE";
    default:
        return "EMPTY";
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

int Field::get_defense(Type type) {
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

int Field::get_defense() {
    return get_defense(type);
};

int Field::get_offense() {
    return get_offense(type);
}

int Field::get_offense(Type type) {
    switch (type)
    {
    case EMPTY:
    case GRAVE:
    case FARM:
    case PINE:
    case PALM:
    case TOWER:
    case FORT:
    case CASTLE:
        return 0;
    case PEASANT:
        return 1;
    case SPEARMAN:
        return 2;
    case SWORDSMAN:
        return 3;
    case KNIGHT:
        return 4;
    default:
        return 0;
    }
};

int Field::distance(Field& field) {
    return (abs(pos.first - field.pos.first)
        + abs(pos.first + pos.second - field.pos.first - field.pos.second)
        + abs(pos.second - field.pos.second)) / 2;
}

int Field::get_threat() {
    for (int i = 3; i >= 0; --i) {
        if (threats[i] > 0) return i + 1;
    }
    return 0;
}

int Field::get_score() {
    switch (type)
    {
    case EMPTY:
    case GRAVE:
    case CASTLE:
    case PINE:
    case PALM:
        return 0;
    case FARM:
        return 12;
    case TOWER:
        return 15;
    case FORT:
        return 35;
    case PEASANT:
        return 10;
    case SPEARMAN:
        return 20;
    case SWORDSMAN:
        return 30;
    case KNIGHT:
        return 40;
    default:
        return 0;
    }
};