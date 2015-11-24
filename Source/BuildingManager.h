#pragma once

#include <Common.h>
#include "WorkerManager.h"
#include "BuildingPlacer.h"
#include "InformationManager.h"
#include "MapTools.h"
#include "ScoutManager.h"

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

double				Euclidean_Distance(int x1, int x2, int y1, int y2);
		bool				buildable(int x, int y, BWAPI::TilePosition mySunkPosition) const;
		bool				buildable2(int x, int y, BWAPI::TilePosition mySunkPosition) const;
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

	bool				sentFirstDroneForSunken = false;
		bool				madeFirstSunken = false;
		bool				isBaseLocation(BWAPI::TilePosition);
		int					sunkenBuildTimer = 9999999;
		bool				canBuild = false;
		bool				canSunken = false;
		bool				canBuildTrigger = true;
		double				mainToRampDistance;
		BWAPI::Position		ourRampPosition;
		BWAPI::Unit			naturalGas;
		BWAPI::Position		firstHatcheryPosition;
		BWAPI::Unit			hatcheryUnit;
		BWAPI::TilePosition getExtractorPosition(BWAPI::TilePosition);
		bool				sunkenIntersection(BWAPI::TilePosition) const;
		int					firstEvoChamber;


		std::set<BWAPI::Unit> evoCompleted;
		std::set<int> sentSunkenCommand;
		int baseCount = 0;
		BWAPI::TilePosition getSunkenPosition(void);
		std::set<BWAPI::TilePosition> createdHatcheriesSet;
		std::set<BWAPI::TilePosition> createdSunkenSet;
		std::vector<BWAPI::TilePosition> createdSunkenVector;
		std::vector<BWAPI::TilePosition> createdBaseVector;
		std::vector<BWAPI::TilePosition> createdHatcheriesVector;
		std::set<BWAPI::TilePosition> createdBuilding;
		std::set<BWAPI::TilePosition> buildableSunkenTilePositions;
		std::map<int, std::pair<int, int> > knownBuildableLocations;

		std::set<BWAPI::UpgradeType> upgradeEvo;


};
}