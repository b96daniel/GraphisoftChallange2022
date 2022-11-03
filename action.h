#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <utility>

class Action {
    public:
        static constexpr float MIN_VALUE = -1000000000.0;
        float value = MIN_VALUE;
        virtual std::string str() = 0;
        virtual ~Action() = default;
};

class Buy : public Action {
public:
    std::pair<int, int> pos{ 0, 0 };
    int type{ 0 };
    std::string get_type_str();
    enum {
        FARM,
        TOWER,
        FORT,
        PEASANT,
        SPEARMAN,
        SWORDSMAN,
        KNIGHT
    };

    virtual std::string str() override;
    Buy() = default;
    Buy(std::pair<int, int> pos,int type) : pos{ pos }, type{ type } {};
};

class Move : public Action {
    std::pair<int, int> from_pos{ 0, 0 };
    std::pair<int, int> to_pos{ 0, 0 };

public:
    virtual std::string str() override;
    Move() = default;
    Move(std::pair<int, int> from_pos, std::pair<int, int> to_pos) : from_pos{ from_pos }, to_pos{ to_pos } {};
};

#endif