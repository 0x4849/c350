#pragma once

#include "Common.h"
#include "BuildingData.h"
#include "MetaType.h"
#include "InformationManager.h"

namespace UAlbertaBot
{


class BuildingPlacer // in general seems to build buildings at random locations. ProductionManager only passes in startLocation
					 // as the target location for every building, so most buildings are made in the main base
{
    BuildingPlacer();

    std::vector< std::vector<bool> > _reserveMap; // covers the whole map to reserve building space

    int     _boxTop;  // marks the mineral area, but seemingly only for the main base
    int	    _boxBottom;
    int	    _boxLeft;
    int	    _boxRight;

    void    computeBuildableTileDistance(BWAPI::TilePosition tp); // unused

public:

    static BuildingPlacer & Instance();

    // queries for various BuildingPlacer data
    bool					buildable(const Building & b,int x,int y) const; // called by canBuildHereWithSpace()
    bool					isReserved(int x,int y) const; // unused
    bool					isInResourceBox(int x,int y) const; // called by canBuildHereWithSpace()
    bool					tileOverlapsBaseLocation(BWAPI::TilePosition tile,BWAPI::UnitType type) const; // called by canBuildHere()
    bool                    tileBlocksAddon(BWAPI::TilePosition position) const;

    BWAPI::TilePosition		GetBuildLocation(const Building & b,int padding) const; // unused

    // determines whether we can build at a given location
    bool					canBuildHere(BWAPI::TilePosition position,const Building & b) const;
	// canBuildHereWithSpace is the main one; it calls canBuildHere() and is called by getBuildLocationNear()
    bool					canBuildHereWithSpace(BWAPI::TilePosition position,const Building & b,int buildDist,bool horizontalOnly = false) const;

    // returns a build location near a building's desired location
	// uses MapTools.getClosestTilesTo to find tiles near b.desiredPosition
	// searches tiles for one where you canBuildHereWithSpace()
	// BuildingManager.getBuildingLocation() calls this and uses the distance padding in Config
    BWAPI::TilePosition		getBuildLocationNear(const Building & b,int buildDist,bool horizontalOnly = false) const;

    void					reserveTiles(BWAPI::TilePosition position,int width,int height);
    void					freeTiles(BWAPI::TilePosition position,int width,int height);

    void					drawReservedTiles();
    void					computeResourceBox(); // called in constructor. sets _box ints

    BWAPI::TilePosition		getRefineryPosition(); // gets the closest geyser to the main base. called by BuildingManager.getBuildingLocation()

};
}