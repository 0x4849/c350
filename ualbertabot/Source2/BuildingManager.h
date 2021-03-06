

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

		std::vector<Building> _buildings;

		bool            _debugMode;
		int             _reservedMinerals;                          // minerals reserved for planned buildings
		int             _reservedGas;                                       // gas reserved for planned buildings

		bool            isEvolvedBuilding(BWAPI::UnitType type);
		bool            isBuildingPositionExplored(const Building & b) const;
		void            removeBuildings(const std::vector<Building> & toRemove);

		void            validateWorkersAndBuildings();                  // STEP 1
		void            assignWorkersToUnassignedBuildings();       // STEP 2
		void            constructAssignedBuildings();                       // STEP 3
		void            checkForStartedConstruction();                      // STEP 4
		void            checkForDeadTerranBuilders();                       // STEP 5
		void            checkForCompletedBuildings();                       // STEP 6

		char            getBuildingWorkerCode(const Building & b) const;


	public:

		static BuildingManager &    Instance();

		double				Euclidean_Distance(int x1, int x2, int y1, int y2);
		bool				buildable(int x, int y, BWAPI::TilePosition mySunkPosition) const;
		bool				buildable2(int x, int y, BWAPI::TilePosition mySunkPosition) const;
		void                update();
		void                onUnitMorph(BWAPI::Unit unit);
		void                onUnitDestroy(BWAPI::Unit unit);
		void                addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, bool isGasSteal);
		void                drawBuildingInformation(int x, int y);
		BWAPI::TilePosition getBuildingLocation(const Building & b);

		int                 getReservedMinerals();
		int                 getReservedGas();
		void				checkSunkenUpgrade();
		bool                isBeingBuilt(BWAPI::UnitType type);
		bool				sentFirstDroneForSunken = false;
		bool				madeFirstSunken = false;
		bool				isBaseLocation(BWAPI::TilePosition);
		int					sunkenBuildTimer = 9999999;
		bool				canBuild = false;
		bool				canSunken = false;
		bool				canBuildTrigger = true;
		double				mainToRampDistance;
		double				expansionToChokeDistance;
		BWAPI::Position		ourRampPosition;
		BWAPI::Position		ourChokePointPosition;

		BWAPI::Unit			naturalGas;
		BWAPI::Position		firstHatcheryPosition;
		BWAPI::Unit			hatcheryUnit;
		BWAPI::TilePosition getExtractorPosition(BWAPI::TilePosition);
		bool				sunkenIntersection(BWAPI::TilePosition) const;
		int					firstEvoChamber;
		void				checkForBuildingProblems();

		std::set<BWAPI::Unit> evoCompleted;
		std::set<int> sentSunkenCommand;
		int baseCount = 0;
		BWAPI::TilePosition getSunkenPosition(BWAPI::Unit myBuilder);
		std::vector<BWAPI::UnitType> buildingsQueued();
		std::set<BWAPI::TilePosition> createdHatcheriesSet;
		std::set<BWAPI::TilePosition> createdSunkenSet;
		std::vector<BWAPI::TilePosition> createdSunkenVector;
		std::vector<BWAPI::TilePosition> createdBaseVector;
		std::vector<BWAPI::TilePosition> createdHatcheriesVector;
		std::set<BWAPI::TilePosition> createdBuilding;
		std::set<BWAPI::TilePosition> buildableSunkenTilePositions;
		std::map<int, std::pair<int, int> > knownBuildableLocations;

		std::map<BWAPI::UnitType, int> expectedBuildingNumber;
		std::map<BWAPI::UnitType, int> expectedBuildingCheck;


		std::map<BWAPI::Unit, int> sentBuildingCommandFrame;
		std::map<BWAPI::Unit, MetaType> sentBuildingCommandBuilding;
		std::set<BWAPI::UpgradeType> upgradeEvo;
		std::set<BWAPI::UpgradeType> upgradeHydra;
		std::set<BWAPI::Unit> hydraCompleted;

		std::map<BWAPI::Unit, int> createdCreeps;

		void removeBuildingExternal(BWAPI::TilePosition);

		BWAPI::TilePosition firstExtractorPosition = BWAPI::TilePositions::None;

		int didGasTrickFrames = 9999999;
		std::set<BWAPI::TilePosition> completedBuilding;

		bool shouldIExpand = false;
		
	};
}

