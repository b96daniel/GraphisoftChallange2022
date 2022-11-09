#include "map.h"
#include "constants.h"
#include <algorithm>
#include <array>
#include <map>
#include <deque>

// -----------------
// Private functions
// -----------------

void Map::set_threat(Field* field) {
    constexpr int depth = Constants::THREAT_DEPTH;
    std::map<Field*, int> visited;
    std::deque<Field*> not_visited;
    visited[field] = 0;
    not_visited.push_back(field);

    while (!not_visited.empty()) {
        Field* current_field = not_visited.front();
        not_visited.pop_front();

        iterate_neighbours(*current_field, [&visited, &not_visited, current_field, this](Field& neighbour) {
            if (visited.find(&neighbour) == visited.end()) {
                /* Can step on this field or can step through it */
                if (!neighbour.water)
                {
                    visited[&neighbour] = visited[current_field] + 1;
                    if (visited[&neighbour] < depth) /* Can step further */
                        not_visited.push_back(&neighbour);
                }
            }
           });
    }

    for (const auto& element : visited) {
        if (field->get_offense() > 0) ++element.first->threats[field->get_offense() - 1];
        else if (field->type == Field::CASTLE) ++element.first->threats[Constants::CASTLE_THREAT - 1];
    }
}

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
            fields[i][j].detected = false;
            fields[i][j].threats = {0, 0, 0, 0};
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
    current_field.detected = true;


    if (owner == infos.id) {
        own_fields.push_back(&current_field);
        if (current_field.type == Field::CASTLE) infos.castle = &current_field;
        if (current_field.type == Field::FARM) farms.push_back(&current_field);
        if (current_field.type >= Field::PEASANT && current_field.type <= Field::KNIGHT) units.push_back(&current_field);
        income += value + current_field.get_income(current_field.type);
    }
    else if (current_field.type >= Field::PEASANT && current_field.type <= Field::KNIGHT) set_threat(&current_field);
    else if (current_field.type == Field::CASTLE) set_threat(&current_field);
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
        iterate_neighbours(*own_field, [this, &res](Field& neighbour) {
            if (neighbour.owner == infos.id &&
                (neighbour.type == Field::FARM || neighbour.type == Field::CASTLE)) res = true;
        });
    }
    return res;
}

// Iterates through the neighbours of a field
void Map::iterate_neighbours(Field& field, const std::function<void(Field&)>& func) {
    int q = field.pos.first;
    int r = field.pos.second;

    if (q - 1 >= -infos.radius - std::min(0, r)) func(get_field({ q - 1, r }));
    if (q + 1 <= infos.radius - std::max(0, r)) func(get_field({ q + 1, r}));
    if (r - 1 >= -infos.radius - std::min(0, q)) func(get_field({ q, r - 1}));
    if (r + 1 <= infos.radius - std::max(0, q)) func(get_field({ q, r + 1}));

    if (q - 1 >= -infos.radius - std::min(0, r + 1) &&
        r + 1 <= infos.radius - std::max(0, q - 1)) func(get_field({ q - 1, r + 1}));
       
    if (q + 1 <= infos.radius - std::max(0, r - 1) &&
        r - 1 >= -infos.radius - std::min(0, q + 1)) func(get_field({ q + 1, r - 1}));
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
        // return std::max(2 + 4 * get_numof_peasants(), 10);
        return std::max(2 + 4 * static_cast<int>(units.size()), 10);
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

int Map::get_defense(Field* field) {
    int defense = field->get_defense();
    iterate_neighbours(*field, [field, this, &defense](Field& neighbour) {
        if (neighbour.owner == field->owner) {
            int n_defense = neighbour.get_defense();
            if (defense < n_defense) defense = n_defense;
        }
        });
    return defense;
}

bool Map::neighbours_detected(Field& field) {
    bool ret = true;
    iterate_neighbours(field, [&field, this, &ret](Field& neighbour) {
        ret &= neighbour.detected;
    });
    return ret;
}

int Map::get_tower_cover(Field* field, Field::Type type)
{
    float ret = 0;
    if (get_defense(field) < Field::get_defense(type)) Field::get_defense(type) - get_defense(field);
    iterate_neighbours(*field, [&ret, field, this, &type](Field& neighbour) {
        if (get_defense(&neighbour) < Field::get_defense(type)) ret += Field::get_defense(type) - get_defense(&neighbour);
    });
   
    return ret / 21;
}

bool Map::is_defended(Field* f) {
    return (get_defense(f) >= f->get_threat());
}

void Map::remove_threat(Field* field) {
    constexpr int depth = Constants::THREAT_DEPTH;
    std::map<Field*, int> visited;
    std::deque<Field*> not_visited;
    visited[field] = 0;
    not_visited.push_back(field);

    while (!not_visited.empty()) {
        Field* current_field = not_visited.front();
        not_visited.pop_front();

        iterate_neighbours(*current_field, [&visited, &not_visited, current_field, this](Field& neighbour) {
            if (visited.find(&neighbour) == visited.end()) {
                /* Can step on this field or can step through it */
                if (!neighbour.water)
                {
                    visited[&neighbour] = visited[current_field] + 1;
                    if (visited[&neighbour] < depth) /* Can step further */
                        not_visited.push_back(&neighbour);
                }
            }
            });
    }

    for (const auto& element : visited) {
        if (field->get_offense() > 0) --element.first->threats[field->get_offense() - 1];
    }
}
