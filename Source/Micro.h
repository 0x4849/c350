#pragma once

#include <Common.h>
#include <BWAPI.h>

namespace UAlbertaBot
{
namespace Micro // contains all underlying micro actions. consider adding other micro functions
{      
    void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
    void SmartAttackMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);
    void SmartMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition); // before moving, makes sure that position is on
																				  // the map. does not check that it is walkable
																				  // perhaps this should be modified in the event
																				  // you hit a wall 
    void SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target);
    void SmartLaySpiderMine(BWAPI::Unit unit, BWAPI::Position pos);
    void SmartRepair(BWAPI::Unit unit, BWAPI::Unit target);
    void SmartKiteTarget(BWAPI::Unit rangedUnit, BWAPI::Unit target); // has bias towards Protoss Dragoon kiting. may want to improve for hydras
																	  // also contains code to move in the opposite direction of
																	  // the target, but it's simpler than muta dance
    void MutaDanceTarget(BWAPI::Unit muta, BWAPI::Unit target);
    BWAPI::Position GetKiteVector(BWAPI::Unit unit, BWAPI::Unit target); // complex way of getting a position to kite to
																		 // does actually seem to move in the opposite direction
																		 // of target
																		 // try implementing something similar for zergling kite
    void Rotate(double &x, double &y, double angle);
    void Normalize(double &x, double &y);

    void drawAPM(int x, int y);

	//NEW
	void StormDodge(BWAPI::Unit unit); // TO BE IMPLEMENTED. the given unit moves away from the nearest templar. move away for 3 seconds and come back as the storm ends
};
}