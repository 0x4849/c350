#pragma once

#include <Common.h>
#include "WorkerManager.h"
#include "BuildingPlacer.h"
#include "InformationManager.h"
#include "MapTools.h"

namespace UAlbertaBot
{
class BuildingManager
{
    BuildingManager();

    std::vector<Building> _buildings; // A Building contains info about a building we intend to build

    bool            _debugMode;
    int             _reservedMinerals;				// minerals reserved for planned buildings
    int             _reservedGas;					// gas reserved for planned buildings

    bool            isEvolvedBuilding(BWAPI::UnitType type);
    bool            isBuildingPositionExplored(const Building & b) const;
    void            removeBuildings(const std::vector<Building> & toRemove);

	// called in update loop
    void            validateWorkersAndBuildings();		    // STEP 1
    void            assignWorkersToUnassignedBuildings();	// STEP 2: uses WorkerManager.getBuilder(building) and 
															// BuildingPlacer.reserveTiles(). calls getBuildingLocation()
    void            constructAssignedBuildings();			// STEP 3
    void            checkForStartedConstruction();			// STEP 4: call to BuildingPlacer.freeTiles()
    void            checkForDeadTerranBuilders();			// STEP 5: unused
    void            checkForCompletedBuildings();			// STEP 6: remove completed buildings from _buildings

    char            getBuildingWorkerCode(const Building & b) const;
    

public:
    
    static BuildingManager &	Instance();

    void                update(); // gameCommander update cycle
    void                onUnitMorph(BWAPI::Unit unit);
    void                onUnitDestroy(BWAPI::Unit unit);
    void                addBuildingTask(BWAPI::UnitType type,BWAPI::TilePosition desiredLocation,bool isGasSteal, bool isMacro); // called only by ProdManager
    void                drawBuildingInformation(int x,int y);
    BWAPI::TilePosition getBuildingLocation(const Building & b); // figures out where to put a building. considers refineries,
															     // bases, and gas steals. all other buildings are handled with a
																 // call to BuildingPlacer.getBuildLocationNear()
																 // distance padding between buildings defined in Config
																 // TO-DO: all hatcheries are considered expansions! what if
																 // we want macro hatchery?

    int                 getReservedMinerals();
    int                 getReservedGas();

    bool                isBeingBuilt(BWAPI::UnitType type); // searches _buildings and returns false if the given building isn't
														    // in _buildings. i.e. learn if we will be building something or not

    std::vector<BWAPI::UnitType> buildingsQueued();
};
}