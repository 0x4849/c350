#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroManager;

class MeleeManager : public MicroManager
{
	//TOMMY
	//bool regionKiting = true;
	
	//TOMMY
	std::map<BWAPI::Unit, int>							_currentRegionVertexIndex;  // current position to flee to. must start at -1 and be reset to -1 when unit stops fleeing
	std::map<BWAPI::Unit, std::vector<BWAPI::Position>> _currentRegionVertices;	// holds border of region that each unit is currently in
	
	//TOMMY
	int                             getClosestVertexIndex(BWAPI::Unit unit);	// edge of current region closest to unit
	BWAPI::Position                 getFleePosition(BWAPI::Unit unit, BWAPI::Unit enemy);			// figure out where to run to
	void                            followPerimeter(BWAPI::Unit unit, BWAPI::Unit enemy);			// run around the current region
	void                            calculateCurrentRegionVertices(BWAPI::Unit unit);	// calculate edge of current region for a unit by ScoutManager's algorithm
	void                            calculateCurrentRegionVertices2(BWAPI::Unit unit);	// uses BWTA::getPolygon instead

public:

	MeleeManager();
	~MeleeManager() {}
	void executeMicro(const BWAPI::Unitset & targets);

	BWAPI::Unit chooseTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets, std::map<BWAPI::Unit, int> & numTargeting);
	BWAPI::Unit closestMeleeUnit(BWAPI::Unit target, const BWAPI::Unitset & meleeUnitToAssign);
	int getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit);
	BWAPI::Unit getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets);
    bool meleeUnitShouldRetreat(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets);
    std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets);

    void assignTargetsNew(const BWAPI::Unitset & targets);
    void assignTargetsOld(const BWAPI::Unitset & targets);

	//TOMMY
	void setUnits(const BWAPI::Unitset & u);
};
}