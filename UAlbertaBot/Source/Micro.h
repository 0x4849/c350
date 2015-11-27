#pragma once

#include <Common.h>
#include <BWAPI.h>

namespace UAlbertaBot
{
namespace Micro
{      
    void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
    void SmartAttackMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);
    void SmartMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);
    void SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target);
    void SmartLaySpiderMine(BWAPI::Unit unit, BWAPI::Position pos);
    void SmartRepair(BWAPI::Unit unit, BWAPI::Unit target);
    void SmartKiteTarget(BWAPI::Unit rangedUnit, BWAPI::Unit target);
    void MutaDanceTarget(BWAPI::Unit muta, BWAPI::Unit target);
    BWAPI::Position GetKiteVector(BWAPI::Unit unit, BWAPI::Unit target);

    void Rotate(double &x, double &y, double angle);
    void Normalize(double &x, double &y);

    void drawAPM(int x, int y);

	//TOMMY
	void StormDodge(BWAPI::Unit unit); // TO BE IMPLEMENTED. the given unit moves away from the nearest templar. move away for 2 seconds and come back as the storm ends

	BWAPI::Position GetNearbyPos(BWAPI::Position pos); //TO BE IMPLEMENTED. helper function in the case that you move to an unwalkable tile
	int CalcDistance(BWAPI::Unit unit, int time); // TO BE IMPLEMENTED. helper function that gets a distance a unit will travel
	// in the given time
	BWAPI::Position GetFleeVector(BWAPI::Unit unit, BWAPI::Unit enemy); // TO BE IMPLEMENTED. helper function that gets the direction
	// of movement to flee directly away a given enemy
};
}