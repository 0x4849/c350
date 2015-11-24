#pragma once

#include "Common.h"
#include "BWTA.h"

#include "UnitData.h"

#include "..\..\SparCraft\source\SparCraft.h"

//NEW
#include "UnitUtil.h"

namespace UAlbertaBot
{
struct BaseInfo; // unused. seems useful in theory though
typedef std::vector<BaseInfo> BaseInfoVector; // unused

class InformationManager 
{
    InformationManager();
    
    BWAPI::Player       _self;
    BWAPI::Player       _enemy;

    std::map<BWAPI::Player, UnitData>                   _unitData;  // UnitData carries a map of UnitInfo for every Unit
    std::map<BWAPI::Player, BWTA::BaseLocation *>       _mainBaseLocations; // just holds our base and the enemy base
    std::map<BWAPI::Player, std::set<BWTA::Region *> >  _occupiedRegions; // an occupied region is defined as a BWTA::Region with
		bool					_enemyExpand;																	  // a player's building on it. by this logic more than one
																		  // player can occupy the same region

	//NEW: estimate of number of enemy units based on production buildings. initialized with the number of currently known enemy
	// units upon seeing an enemy production building
	std::map<BWAPI::UnitType, int>						_enemyProductionEstimate;
	int													_firstProdBuildingTime;

    int                     getIndex(BWAPI::Player player) const; // unused

    void                    updateUnit(BWAPI::Unit unit); // calls updateUnit() on unitData. part of update loop
    void                    initializeRegionInformation(); // only called in constructor. sets up _mainBaseLocations for self and enemy
														   // also updateOccupiedRegions for our base	
    void                    initializeBaseInfoVector(); // unused
    void                    updateUnitInfo(); // update loop. calls updateUnit() on every unit and calls removeBadUnits() on _unitData
    void                    updateBaseLocationInfo(); // update loop.  clears _occupiedRegions. updates _mainBaseLocations for enemy
													  // calls updateOccupiedRegions() on every building each player owns 
    void                    updateOccupiedRegions(BWTA::Region * region,BWAPI::Player player); // part of update loop
																							   // used by updateBaseLocationInfo()
																							   // responsible for updating _occupiedRegions
    bool                    isValidUnit(BWAPI::Unit unit); // unused

public:

    // yay for singletons!
    static InformationManager & Instance();

    void                    update(); // called by gameCommander loop. updateUnitInfo() and updateBaseLocationInfo()

    // event driven stuff. allows InfoManager to keep track of all units
	//NEW: modified to start the production timer when an enemy prod building is seen, or increase rate when another building is seen
	void					onUnitShow(BWAPI::Unit unit);
    void					onUnitHide(BWAPI::Unit unit)        { updateUnit(unit); }
    void					onUnitCreate(BWAPI::Unit unit)		{ updateUnit(unit); }
    void					onUnitComplete(BWAPI::Unit unit)    { updateUnit(unit); }
    void					onUnitMorph(BWAPI::Unit unit)       { updateUnit(unit); }
    void					onUnitRenegade(BWAPI::Unit unit)    { updateUnit(unit); }
    void					onUnitDestroy(BWAPI::Unit unit);	//NEW: modified to adjust enemy production estimate as enemies are defeated

    bool					isEnemyBuildingInRegion(BWTA::Region * region); // could be useful. used by updateBaseLocationInfo()
    int						getNumUnits(BWAPI::UnitType type,BWAPI::Player player); // # of units of given type that a player has?
    bool					nearbyForceHasCloaked(BWAPI::Position p,BWAPI::Player player,int radius); // unused. sad, could be useful
    bool					isCombatUnit(BWAPI::UnitType type) const; // for some reason, ignores Zerg Lurkers

	// useful. adds UnitInfos of units for a player that can attack into a given radius into given vector. 
    void                    getNearbyForce(std::vector<UnitInfo> & unitInfo,BWAPI::Position p,BWAPI::Player player,int radius);

    const UIMap &           getUnitInfo(BWAPI::Player player) const; // useful: gets a map of UnitInfo to units for a player

    std::set<BWTA::Region *> &  getOccupiedRegions(BWAPI::Player player); // useful: get regions that have enemy buildings
    BWTA::BaseLocation *    getMainBaseLocation(BWAPI::Player player); // useful: get base location of enemy. 

    bool                    enemyHasCloakedUnits(); // useful. also returns true if enemy has buildings that can produce cloakers
													// works by calling getUnits() on a player's UnitData

    void                    drawExtendedInterface();
    void                    drawUnitInformation(int x,int y);
    void                    drawMapInformation();

    const UnitData &        getUnitData(BWAPI::Player player) const; // useful: returns UnitData of a player

	//NEW: once initialized, it increments enemy unit counts every time one is guessed to have been created
	// called in update() loop
	void					updateEnemyProductionEstimate();
	std::map<BWAPI::UnitType, int>		getEnemyProductionEstimate();

bool					isEnemyExpand();
};
}
