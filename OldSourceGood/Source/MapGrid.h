#pragma once

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{

class GridCell // good for two functions: least explored area and units in a radius
{
public:

	int             timeLastVisited;
    int             timeLastOpponentSeen;
	BWAPI::Unitset  ourUnits;
	BWAPI::Unitset  oppUnits;
	BWAPI::Position center;

	GridCell() 
        : timeLastVisited(0)
        , timeLastOpponentSeen(0)
    {
    }
};


class MapGrid 
{
	MapGrid();
	MapGrid(int mapWidth, int mapHeight, int cellSize);

	int							cellSize;
	int							mapWidth, mapHeight; // in Tiles
	int							rows, cols;
	int							lastUpdated;

	std::vector< GridCell >		cells; // GridCells are probably tiles

	void						calculateCellCenters(); // called when MapGrid() is initialized

	void						clearGrid(); // called on every update()
	BWAPI::Position				getCellCenter(int x, int y);

	BWAPI::Position				naturalExpansion; // unused. what a shame!

public:

	// yay for singletons!
	static MapGrid &	Instance();

	void				update(); // called by gameCommander. updates the units found on every GridCell by calling getCell()
	void				GetUnits(BWAPI::Unitset & units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits); // useful function that gets allied and/or
																														    // enemy units in a radius
	BWAPI::Position		getLeastExplored(); // frequently used function to get the least explored area of the map
	BWAPI::Position		getNaturalExpansion(); // unused. what a shame! would be nice to implement this

	// called by update() to get units on cells
	GridCell & getCellByIndex(int r, int c)		{ return cells[r*cols + c]; }
	GridCell & getCell(BWAPI::Position pos)		{ return getCellByIndex(pos.y / cellSize, pos.x / cellSize); }
	GridCell & getCell(BWAPI::Unit unit)		{ return getCell(unit->getPosition()); }
};

}