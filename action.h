#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <utility>

class Action {
	float value = 0;

    public:
        virtual std::string str() = 0;
};

class Buy : public Action {
    std::pair<int, int> pos{0, 0};
    int type{ 0 };
    std::string get_type_str();

public:
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
    Buy(std::pair<int, int> pos,int type) : pos{ pos }, type{ type } {};
};

class Move : public Action {
    std::pair<int, int> from_pos{ 0, 0 };
    std::pair<int, int> to_pos{ 0, 0 };

public:
    virtual std::string str() override;
    Move(std::pair<int, int> from_pos, std::pair<int, int> to_pos) : from_pos{ from_pos }, to_pos{ to_pos } {};
};

#endif