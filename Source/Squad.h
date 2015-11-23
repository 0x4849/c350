#pragma once

#include "Common.h"
#include "MeleeManager.h"
#include "RangedManager.h"
#include "DetectorManager.h"
#include "TransportManager.h"
#include "SquadOrder.h"
#include "DistanceMap.hpp"
#include "StrategyManager.h"
#include "CombatSimulation.h"
#include "TankManager.h"
#include "MedicManager.h"

namespace UAlbertaBot
{
    
class Squad // important features: a squad has some units, an order, a priority, and managers to control the units
{
    std::string         _name;
	BWAPI::Unitset      _units;
	std::string         _regroupStatus;
    int                 _lastRetreatSwitch;
    bool                _lastRetreatSwitchVal;
    size_t              _priority;
	
	SquadOrder          _order;
	MeleeManager        _meleeManager;
	RangedManager       _rangedManager;
	DetectorManager     _detectorManager;
	TransportManager    _transportManager;
    TankManager         _tankManager;
    MedicManager        _medicManager;

	std::map<BWAPI::Unit, bool>	_nearEnemy;	// enemies are defined as near if they are within 400 units of the unit

    
	BWAPI::Unit		getRegroupUnit(); // unused
	BWAPI::Unit		unitClosestToEnemy(); // seems to have a role with DetectorManager
    
	void                        updateUnits(); // calls the following 3 functions
	void                        addUnitsToMicroManagers(); // sorts units by type and calls the appropriate manager.setUnits()
	void                        setNearEnemyUnits(); // for every unit, calls unitNearEnemy() to see if it is near an enemy and
												     // constructs the _nearEnemy map
	void                        setAllUnits(); // cleans units (ex. remove the dead)
	
	bool                        unitNearEnemy(BWAPI::Unit unit); // used by updateUnits() to see if enemy units are near our units
	bool                        needsToRegroup(); // called by update() to see if the squad needs to regroup. only for Squads with
												  // the Attack order (thank goodness). also prevents attacking for 5 seconds after a retreat
	int                         squadUnitsNear(BWAPI::Position p); // unused

public:

	Squad(const std::string & name, SquadOrder order, size_t priority);
	Squad();
    ~Squad();

	void                update(); // calls updateUnits(). calls needsToRegroup(). if true, calls .regroup(calcRegroupPosition())
								  // on every manager. otherwise, calls .execute(_order) on every manager. may have to manually
								  // block regrouping based on the type of _order (actually no, thank goodness)
	void                setSquadOrder(const SquadOrder & so); // used by squad update functions in CombatCommander
	void                addUnit(BWAPI::Unit u); // called by SquadData.assignUnitToSquad(). avoid using this if possible
	void                removeUnit(BWAPI::Unit u); // called by SquadData.assignUnitToSquad()
    bool                containsUnit(BWAPI::Unit u) const; // unused
    bool                isEmpty() const;
    void                clear(); // calls WorkerManager.finishedWithWorker() before emptying _units. could be called in certain
								 // squad update functions in CombatCommander()
    size_t              getPriority() const; // used by SquadData.canAssignUnitToSquad()
    void                setPriority(const size_t & priority); // unused. currently once a Squad is initialized with an order, it doesn't change
    const std::string & getName() const;
    
	BWAPI::Position     calcCenter(); // unused
	BWAPI::Position     calcRegroupPosition(); // called in update() if you need to regroup
											   // regroups to the squad unit that is closest to the _order position and is
											   // not near an enemy, or to the main base if all units are near enemies

	const BWAPI::Unitset &  getUnits() const;
	const SquadOrder &  getSquadOrder()	const;
};
}