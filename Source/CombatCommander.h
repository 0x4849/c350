#pragma once

#include "Common.h"
#include "Squad.h"
#include "SquadData.h"
#include "InformationManager.h"
#include "StrategyManager.h"

namespace UAlbertaBot
{
class CombatCommander
{
	SquadData       _squadData;
    BWAPI::Unitset  _combatUnits;	// all ally combat units; constantly updated
    bool            _initialized;	// only call initializeSquads() once

	// squad update functions generally collect units to assign to squads and give them an order. many variations
    void            updateScoutDefenseSquad();
	void            updateDefenseSquads();
	void            updateAttackSquads();
    void            updateDropSquads();
	void            updateIdleSquad();
	void			updateDistractSquad();
	void			updateHarassSquad();
	void			updateHydraSquad();
	bool            isSquadUpdateFrame();	// only update the various squads every 10th frame, to make units move efficiently
										  // examine this code for an example of how to control update periodicity
										  // uses BWAPI::Broodwar->getFrameCount() for elapsed total logical FPS
	int             getNumType(BWAPI::Unitset & units, BWAPI::UnitType type);

	// both used by updateDefenseSquads()
	BWAPI::Unit     findClosestDefender(const Squad & defenseSquad, BWAPI::Position pos, bool flyingDefender);
    BWAPI::Unit     findClosestWorkerToTarget(BWAPI::Unitset & unitsToAssign, BWAPI::Unit target);

	BWAPI::Position getDefendLocation();	// unused
    BWAPI::Position getMainAttackLocation();	// gets a position for Attack squad

    void            initializeSquads();	 // called once in constructor to set up squads. does not assign any units
    void            verifySquadUniqueMembership();	 // called by SquadData.update()
    void            assignFlyingDefender(Squad & squad);	// unused
    void            emptySquad(Squad & squad, BWAPI::Unitset & unitsToAssign);	// unused
    int             getNumGroundDefendersInSquad(Squad & squad);	// unused
    int             getNumAirDefendersInSquad(Squad & squad);	// unused

    void            updateDefenseSquadUnits(Squad & defenseSquad, const size_t & flyingDefendersNeeded, const size_t & groundDefendersNeeded);	// used by updateDefenseSquads()
    int             defendWithWorkers();	// return number of enemy units near workers. seems like a poorly named function. unused

    int             numZerglingsInOurBase();	// called by findClosestDefender()
    bool            beingBuildingRushed();	// called by findClosestDefender(). doesn't seem very useful; just returns true if you
										   // see an enemy building

public:

	CombatCommander();

	void update(const BWAPI::Unitset & combatUnits);	// initializes squads once, gets all ally combat units,
													 // updates squads, calls SquadData.update(). net result is to call update()
													 // for each Squad, which in turn calls execute() for each manager in the squad
    
	void drawSquadInformation(int x, int y);
};
}
