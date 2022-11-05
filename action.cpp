#include "action.h"
#include <sstream>

// -----------------------
// Public functions -> Buy
// -----------------------
Buy::Buy(std::pair<int, int> pos, Field::Type type) : pos{ pos }, type{ type} {};

std::string Buy::str() {
	std::stringstream ss;
	ss << "BUY " << pos.first << " " << pos.second << " " << Field::get_type_str(type);
	return ss.str();
}

// ------------------------
// Public functions -> Move
// ------------------------
Move::Move(std::pair<int, int> from_pos, std::pair<int, int> to_pos) : from_pos{ from_pos }, to_pos{ to_pos } {};

std::string Move::str() {
	std::stringstream ss;
	ss << "MOVE " << from_pos.first << " " << from_pos.second << " " << to_pos.first << " " << to_pos.second;
	return ss.str();
}