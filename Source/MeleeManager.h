#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroManager;

class MeleeManager : public MicroManager
{
	// NEW: toggle this on or off to change the zergling kiting algorithm
	//bool regionKiting = true;

	//NEW
	std::map<BWAPI::Unit, int>							_currentRegionVertexIndex;  // current position to flee to. must start at -1 and be reset to -1 when unit stops fleeing
	std::map<BWAPI::Unit, std::vector<BWAPI::Position>> _currentRegionVertices;	// holds border of region that each unit is currently in
	//NEW
	int                             getClosestVertexIndex(BWAPI::Unit unit);	// edge of current region closest to unit
	BWAPI::Position                 getFleePosition(BWAPI::Unit unit, BWAPI::Unit enemy);			// figure out where to run to
	void                            followPerimeter(BWAPI::Unit unit, BWAPI::Unit enemy);			// run around the current region
	void                            calculateCurrentRegionVertices(BWAPI::Unit unit);		
									// calculate edge of current region for a unit by ScoutManager's algorithm
	void                            calculateCurrentRegionVertices2(BWAPI::Unit unit);	// uses BWTA::getPolygon instead

public:
	
	//NEW
	void setUnits(const BWAPI::Unitset & u); // called by Squad.update(). overriding MicroManager. CURRENTLY THE PROBLEM: CALLED IN LOOP

	MeleeManager();
	~MeleeManager() {}
	void executeMicro(const BWAPI::Unitset & targets);

	BWAPI::Unit chooseTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets, std::map<BWAPI::Unit, int> & numTargeting);	// unused
	BWAPI::Unit closestMeleeUnit(BWAPI::Unit target, const BWAPI::Unitset & meleeUnitToAssign);	// unused
	int getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit);	// allows certain units to be focused
	BWAPI::Unit getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets);	// uses getAttackPriority and distance to choose a target
    bool meleeUnitShouldRetreat(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets);
    std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets);

    void assignTargetsNew(const BWAPI::Unitset & targets);
    void assignTargetsOld(const BWAPI::Unitset & targets);
};
}