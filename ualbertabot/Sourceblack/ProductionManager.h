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

class ProductionManager
{
    ProductionManager();
    
    BWAPI::TilePosition _predictedTilePosition;
    bool                _enemyCloakedDetected;
    bool                _assignedWorkerForThisBuilding;
    bool                _haveLocationForThisBuilding;
    

    BWAPI::Unit         selectUnitOfType(BWAPI::UnitType type,BWAPI::Position closestTo = BWAPI::Position(0,0));

    bool                hasResources(BWAPI::UnitType type);
    bool                canMake(BWAPI::UnitType type);
    bool                hasNumCompletedUnitType(BWAPI::UnitType type,int num);
    bool                meetsReservedResources(MetaType type);
    void                setBuildOrder(const BuildOrder & buildOrder);
    void                create(BWAPI::Unit producer,BuildOrderItem & item);
    void                manageBuildOrderQueue();
    void                performCommand(BWAPI::UnitCommandType t);
    bool                canMakeNow(BWAPI::Unit producer,MetaType t);
    void                predictWorkerMovement(const Building & b);

    bool                detectBuildOrderDeadlock();
	bool		        canOverlord = false;
    int                 getFreeMinerals();
    int                 getFreeGas();
    bool                canPlanBuildOrderNow() const;

public:
	BuildOrderQueue _queue;
	BWAPI::Unit         getClosestUnitToPosition(const BWAPI::Unitset & units, BWAPI::Position closestTo);
    static ProductionManager &	Instance();
	bool		canProduce(BWAPI::UnitType);
    void        drawQueueInformation(std::map<BWAPI::UnitType,int> & numUnits,int x,int y,int index);
    void        update();
    void        onUnitMorph(BWAPI::Unit unit);
    void        onUnitDestroy(BWAPI::Unit unit);
    void        performBuildOrderSearch();
    void        drawProductionInformation(int x,int y);
    void        setSearchGoal(MetaPairVector & goal);
	void        queueGasSteal();
	void		onEvoUpgradeComplete();
	BWAPI::Unit getUnusedEvo();
	int		    evoUpgradeTimer = 0;
	bool		muscBuild = false;
	int			muscBuildTimer = 0;
	
	std::set<BWAPI::UpgradeType> upgradeSearch;

	std::map<BWAPI::Unit, int> upgradeMap;
	BWAPI::Unit getEvolutionChamberProducer(BWAPI::Unit, MetaType t);
	int		    overlordBuildTimer = 0;
	std::set<BWAPI::Unit> upgradingStuff;
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
}