#ifndef MAP_H
#define MAP_H

#include "infos.h"
#include <utility>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <set>

class Field {
public:
    enum {
        EMPTY,
        PINE,
        PALM,
        CASTLE,
        FARM,
        TOWER,
        FORT,
        PEASANT,
        SPEARMAN,
        SWORDSMAN,
        KNIGHT,
        GRAVE
    };

    std::pair<int, int> pos{ 0, 0 };
    bool water{ false };
    int value{ -1 };
    int owner{ -1 };
    int type{ EMPTY };

    static int get_type_from_str(std::string& type_str);
    Field() = default;
    Field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water = false);
    bool operator==(const Field& f);
};

class Map {
    Infos& infos{ Infos::get_instance() };
    
public:
    std::vector<std::vector<Field>> fields;
    std::vector<std::pair<int, int>> own_fields;
    std::set<std::pair<int, int>> neighbouring_fields;
    void init();
    void set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water);
    void reset();
    void iterate_neighbours(std::pair<int, int> pos, const std::function<void(std::pair<int, int>)>& func);
    Field& get_field(std::pair<int, int> pos);
};

#endif
