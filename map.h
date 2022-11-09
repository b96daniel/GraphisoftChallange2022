#ifndef MAP_H
#define MAP_H

#include "infos.h"
#include "field.h"

#include <utility>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <set>

class Map {
    // Private variables
    Infos& infos{ Infos::get_instance() };
    std::vector<std::vector<Field>> fields;

    // Private functions
    void set_threat(Field* field);
    
public:
    // Public variables
    std::vector<Field*> own_fields;
    std::vector<Field*> farms;
    std::vector<Field*> units;
    int income{ 0 };

    // Public functions
    void init();
    void reset();
    void iterate_neighbours(Field& field, const std::function<void(Field&)>& func);
    Field& get_field(std::pair<int, int> pos);
    void set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water);
    int get_cost(int field_type);
    int get_numof_peasants();
    bool is_farmable(Field* field);
    int get_defense(Field* field);
    bool neighbours_detected(Field& field);
    int get_tower_cover(Field* field, Field::Type type);
    bool is_defended(Field* f);
    void remove_threat(Field* field);
};

#endif
