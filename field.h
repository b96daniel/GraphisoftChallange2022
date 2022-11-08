#ifndef FIELD_H
#define FIELD_H

#include <utility>
#include <string>
#include <array>

class Field {
public:
    // Public type definitions
    enum Type {
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

    // Public variables
    std::pair<int, int> pos{ 0, 0 };
    bool water{ false };
    int value{ 0 };
    int owner{ -1 };
    bool detected{ false };
    std::array<int, 4> threats{0, 0, 0, 0};
    Type type{ EMPTY };

    // Public functions
    Field() = default;
    Field(std::pair<int, int> pos, int value, int owner, std::string& type_str, bool water);
    static Field::Type get_type_from_str(std::string& type_str);
    static std::string get_type_str(Type type);
    static int get_merged_type(int add_type, int base_type);
    static int get_income(int type);
    static int get_defense(Type type);
    int get_defense();
    static int get_offense(Type type);
    int get_offense();
    int distance(Field& field);
    int get_threat();
};

#endif
