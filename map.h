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
    
public:
    // Public variables
    std::vector<Field*> own_fields;

    // Public functions
    void init();
    void reset();
    Field& get_field(std::pair<int, int> pos);
    void set_field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water);


    /*
    int income{ 0 };
    std::vector<std::pair<int, int>> farms;
    std::vector<std::vector<int>> threat_levels;
    std::set<std::pair<int, int>> neighbouring_fields;
    std::vector<std::pair<int, int>> units;
    int get_cost(int field_type);
    void process_loss(std::pair<int, int> pos);
    void iterate_neighbours(std::pair<int, int> pos, const std::function<void(std::pair<int, int>)>& func);
    int& get_threat(std::pair<int, int> pos);
    int get_defense(std::pair<int, int> pos);
    void set_threat(std::pair<int, int> pos, int level);
    */
};

#endif
