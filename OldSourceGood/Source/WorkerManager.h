#pragma once

#include <Common.h>
#include "BuildingManager.h"
#include "WorkerData.h"

namespace UAlbertaBot
{
class Building;

class WorkerManager	// manipulates a WorkerData to control all workers
{
    WorkerData  workerData;
    BWAPI::Unit previousClosestWorker;

    void        setMineralWorker(BWAPI::Unit unit);		// calls getClosestDepot and setWorkerJob
    bool        isGasStealRefinery(BWAPI::Unit unit);
    
    void        handleIdleWorkers();	// calls setMineralWorker on all idle workers
    void        handleGasWorkers();		// fills refinery up to 3 by calling getGasWorkers
    void        handleMoveWorkers();
    void        handleCombatWorkers();
    void        handleRepairWorkers();

    WorkerManager();

public:

    void        update();		// updateWorkerStatus and handleXWorkers in main loop
    void        onUnitDestroy(BWAPI::Unit unit);
    void        onUnitMorph(BWAPI::Unit unit);
    void        onUnitShow(BWAPI::Unit unit);
    void        onUnitRenegade(BWAPI::Unit unit);
    void        finishedWithWorker(BWAPI::Unit unit);

    void        finishedWithCombatWorkers();

    void        drawResourceDebugInfo();
    void        updateWorkerStatus();
    void        drawWorkerInformation(int x,int y);

    int         getNumMineralWorkers();
    int         getNumGasWorkers();
    int         getNumIdleWorkers();
    void        setScoutWorker(BWAPI::Unit worker);

    bool        isWorkerScout(BWAPI::Unit worker);
    bool        isFree(BWAPI::Unit worker);
    bool        isBuilder(BWAPI::Unit worker);

    BWAPI::Unit getBuilder(Building & b,bool setJobAsBuilder = true);	// only uses a move or mineral worker. called by 
																		// building and production managers
    BWAPI::Unit getMoveWorker(BWAPI::Position p);	// for moving a worker to a location. only uses mineral workers
    BWAPI::Unit getClosestDepot(BWAPI::Unit worker);
    BWAPI::Unit getGasWorker(BWAPI::Unit refinery);		// prioritizes gas over minerals
    BWAPI::Unit getClosestEnemyUnit(BWAPI::Unit worker);
    BWAPI::Unit getClosestMineralWorkerTo(BWAPI::Unit enemyUnit);
    BWAPI::Unit getWorkerScout();

    void        setBuildingWorker(BWAPI::Unit worker,Building & b);	// unused. use getBuilder
    void        setRepairWorker(BWAPI::Unit worker,BWAPI::Unit unitToRepair);
    void        stopRepairing(BWAPI::Unit worker);
    void        setMoveWorker(int m,int g,BWAPI::Position p);
    void        setCombatWorker(BWAPI::Unit worker);

    bool        willHaveResources(int mineralsRequired,int gasRequired,double distance);
    void        rebalanceWorkers();		// optimizes workers to minerals. NEW: called in update loop. HOPEFULLY NOT A FAIL

    static WorkerManager &  Instance();
};
}