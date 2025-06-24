#pragma once

#include <stdint.h>


constexpr uint8_t GridSizeX = 8;
constexpr uint8_t GridSizeY = 4;


struct CellCoords {
	int8_t x;
	int8_t y;

	bool operator==(const CellCoords& other) const {
		return x == other.x && y == other.y;
	}
	
	bool operator!=(const CellCoords& other) const {
		return x != other.x || y != other.y;
	}	
};

