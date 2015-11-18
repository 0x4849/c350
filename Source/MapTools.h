#pragma once

#include "Common.h"
#include <vector>
#include "BWAPI.h"
#include "DistanceMap.hpp"

namespace UAlbertaBot
{

// provides useful tools for analyzing the starcraft map
// calculates connectivity and distances using flood fills
class MapTools
{
    
    std::map<BWAPI::Position,
             DistanceMap>       _allMaps;    // a cache of already computed distance maps. get distance from a position to anywhere
    std::vector<bool>           _map;        // the map stored at TilePosition resolution, values are 0/1 for walkable or not walkable
    std::vector<bool>           _units;      // map that stores whether a unit is on this position
    std::vector<int>            _fringe;     // the fringe vector which is used as a sort of 'open list'
    int                         _rows;
    int                         _cols;

    MapTools();

    int                     getIndex(int row,int col);   // return the index of the 1D array from (row,col)
    bool                    unexplored(DistanceMap & dmap,const int index) const;
    void                    reset();                           // resets the distance and fringe vectors, call before each search    
    void                    setBWAPIMapData();                 // reads in the map data from bwapi and stores it in our map format
    void                    resetFringe();
    void                    computeDistance(DistanceMap & dmap,const BWAPI::Position p); // computes walk distance from Position P to all other points on the map
    BWAPI::TilePosition     getTilePosition(int index);

public:

    static MapTools &       Instance();

    void                    parseMap();
    void                    search(DistanceMap & dmap,const int sR,const int sC);
    void                    fill(const int index,const int region); // unused
    int                     getGroundDistance(BWAPI::Position from,BWAPI::Position to); // useful: ground distance between two positions
    int	                    getEnemyBaseDistance(BWAPI::Position p); // unused. why??? this could be useful
    int	                    getMyBaseDistance(BWAPI::Position p); // unused. why??? this could be useful
    BWAPI::Position         getEnemyBaseMoveTo(BWAPI::Position p); // unused. why??? this could be useful
    BWAPI::TilePosition     getNextExpansion(); // calls below for self()
    BWAPI::TilePosition     getNextExpansion(BWAPI::Player player); // useful: returns position of closest gas-containing base
    void                    drawHomeDistanceMap();

    const std::vector<BWAPI::TilePosition> & getClosestTilesTo(BWAPI::Position pos); // useful: returns a vector of all tiles on the
																					 // map sorted by the closest tiles to the position

};

}