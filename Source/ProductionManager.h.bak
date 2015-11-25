#pragma once

#include <Common.h>
#include "BuildOrderQueue.h"
#include "BuildingManager.h"
#include "StrategyManager.h"
#include "BOSSManager.h"
#include "BuildOrder.h"

namespace UAlbertaBot
{
typedef unsigned char Action;

class ProductionManager // in general, gets build orders from StratManager, sends them to BOSS to sort out, puts the items
	                    // into _queue at various priorities, and creates them. Buildings get sent to BuildingManager
{
    ProductionManager();
    
    BuildOrderQueue     _queue;
    BWAPI::TilePosition _predictedTilePosition;
    bool                _enemyCloakedDetected;
    bool                _assignedWorkerForThisBuilding;
    bool                _haveLocationForThisBuilding;
    
    BWAPI::Unit         getClosestUnitToPosition(const BWAPI::Unitset & units,BWAPI::Position closestTo);
    BWAPI::Unit         selectUnitOfType(BWAPI::UnitType type,BWAPI::Position closestTo = BWAPI::Position(0,0));

    bool                hasResources(BWAPI::UnitType type); // unused
    bool                canMake(BWAPI::UnitType type); // unused
    bool                hasNumCompletedUnitType(BWAPI::UnitType type,int num); // unused
    bool                meetsReservedResources(MetaType type); // enough resources for unit?
    void                setBuildOrder(const BuildOrder & buildOrder); // clears the entire _queue
																	  // then queues the given BuildOrder at lowest priority
																	  // called by performBuildOrderSearch()
																	  // this repeatedly queues at lowest priority the build
																	  // order given by BOSS
    void                create(BWAPI::Unit producer,BuildOrderItem & item); // if it's a building, BuildingManager.addBuildingTask()
																			// otherwise the item is morphed or researched
																			// called by manageBuildOrderQueue() in a loop
    void                manageBuildOrderQueue(); // always called in a loop. gets the highest priority item in _queue
											     // if you can make it, call create() on it
											     // if you can skip it, skips it and moves to the next item in _queue
	                                             // presumably, skipping moves the item to the back of the queue?
    void                performCommand(BWAPI::UnitCommandType t); // specifically for the extractor trick. not currently in use
    bool                canMakeNow(BWAPI::Unit producer,MetaType t); // called in manageBuildOrderQueue()
    void                predictWorkerMovement(const Building & b); // call to BuildingManager.getBuildingLocation()

    bool                detectBuildOrderDeadlock(); // suspicious function; seems to think that Zerg buildings give supply
	                                                // possibly trying to consider the fact that Zerg recovers supply by morphing
	                                                // a unit

    int                 getFreeMinerals();
    int                 getFreeGas();
    bool                canPlanBuildOrderNow() const; // performBuildOrderSearch uses this as a check

	//NEW
	bool				checkDefenses();	// checks if creep colonies are in queue or morphing

public:

    static ProductionManager &	Instance();

    void        drawQueueInformation(std::map<BWAPI::UnitType,int> & numUnits,int x,int y,int index);
    void        update(); // always calls manageBuildOrderQueue()
						  // calls performBuildOrderSearch() when _queue is empty; i.e. nothing building.
	                      // also queues supply at highest priority if detectBuildOrderDeadlock()
	                      // also queues detectors if stealth is detected, except for zerg which has overlords
						  // NEW: calls shouldBuildSunkens(). if yes, it queues at highest priority the requested items
    void        onUnitMorph(BWAPI::Unit unit);
    void        onUnitDestroy(BWAPI::Unit unit);
    void        performBuildOrderSearch(); // called in update loop. getsBuildOrder() from BOSS and setsBuildOrder()
										   // only called when _queue is empty
    void        drawProductionInformation(int x,int y);
    void        setSearchGoal(MetaPairVector & goal); // unused
    void        queueGasSteal(); // called by gasSteal() in scoutManager

    BWAPI::Unit getProducer(MetaType t,BWAPI::Position closestTo = BWAPI::Positions::None);
};


class CompareWhenStarted
{

public:

    CompareWhenStarted() {}

    // the sorting operator
    bool operator() (BWAPI::Unit u1,BWAPI::Unit u2)
    {
        int startedU1 = BWAPI::Broodwar->getFrameCount() - (u1->getType().buildTime() - u1->getRemainingBuildTime());
        int startedU2 = BWAPI::Broodwar->getFrameCount() - (u2->getType().buildTime() - u2->getRemainingBuildTime());
        return startedU1 > startedU2;
    }
};
};