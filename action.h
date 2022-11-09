#ifndef ACTION_H
#define ACTION_H

#include "field.h"

#include <string>
#include <utility>

class Action {
    public:
        // Public variables
        static constexpr float MIN_VALUE = -1000000000.0;
        static constexpr float MAX_VALUE = 1000000000.0;
        float value = MIN_VALUE;

        // Public functions
        virtual std::string str() = 0;
        virtual ~Action() = default;
};

class Buy : public Action {
public:
    // Public variables
    std::pair<int, int> pos{ 0, 0 };
    Field::Type type{ Field::FARM };

    // Public functions
    Buy() = default;
    Buy(std::pair<int, int> pos, Field::Type type);
    virtual std::string str() override;
};

class Move : public Action {
public:
    // Public variables
    std::pair<int, int> from_pos{ 0, 0 };
    std::pair<int, int> to_pos{ 0, 0 };

    // Public functions
    Move() = default;
    Move(std::pair<int, int> from_pos, std::pair<int, int> to_pos);
    virtual std::string str() override;
};

#endif