#pragma once

#include "Common.h"
#include "MapGrid.h"
#include "SquadOrder.h"
#include "MapTools.h"
#include "InformationManager.h"
#include "Micro.h"

namespace UAlbertaBot
{
struct AirThreat
{
	BWAPI::Unit	unit;
	double			weight;
};

struct GroundThreat
{
	BWAPI::Unit	unit;
	double			weight;
};

class MicroManager
{

protected:

	//NEW: moved from private to protected
	BWAPI::Unitset  _units;

	SquadOrder			order;

	virtual void        executeMicro(const BWAPI::Unitset & targets) = 0; // overriden by all managers
	bool                checkPositionWalkable(BWAPI::Position pos); // unused. uses broodwar->isWalkable and evades enemy units
	bool                checkPositionWalkable2(BWAPI::Position pos); // NEW: applied to zergling kiting. modified to not go to a tile
																     // with any units
	void                drawOrderText();
	bool                unitNearEnemy(BWAPI::Unit unit); // unused
	bool                unitNearChokepoint(BWAPI::Unit unit) const; // used in meleeManager for something. could be useful for a lurker contain
	void                trainSubUnits(BWAPI::Unit unit) const; // unused. for controlling "summoners"
    

public:
						MicroManager();
    virtual				~MicroManager(){}

	const BWAPI::Unitset & getUnits() const;
	BWAPI::Position     calcCenter() const; // unused

	void				setUnits(const BWAPI::Unitset & u); // called by Squad.update()
	void				execute(const SquadOrder & order); // called by Squad.update(). note that inheriting managers don't override this
														   // it is important to call executeMicro() here. you can also manipulate
														   // actions depending on the SquadOrder
	void				regroup(const BWAPI::Position & regroupPosition) const; // called by Squad.update() when regroup is needed

	//NEW
	bool				inStorm(BWAPI::Unit unit); // TO BE IMPLEMENTED. return true if unit is in a psi storm
};
}

// for every inheriting manager, the most important functions are assignTargetsOld() and executeMicro()
// other functions can be useful but they are mainly helpers