#include "action.h"
#include <sstream>

std::string Buy::str() {
	std::stringstream ss;
	ss << "BUY " << pos.first << " " << pos.second << " " << get_type_str();
	return ss.str();
}

std::string Buy::get_type_str() {
	switch (type)
	{
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
	default:
		return "FARM";
	}
}

std::string Move::str() {
	std::stringstream ss;
	ss << "MOVE " << from_pos.first << " " << from_pos.second << " " << to_pos.first << " " << to_pos.second;
	return ss.str();
}