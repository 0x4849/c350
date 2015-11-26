#pragma once

#include "Common.h"
#include "MapGrid.h"

#ifdef USING_VISUALIZATION_LIBRARIES
	#include "Visualizer.h"
#endif

#include "..\..\SparCraft\source\GameState.h"
#include "..\..\SparCraft\source\Game.h"
#include "..\..\SparCraft\source\Unit.h"
#include "..\..\SparCraft\source\AllPlayers.h"
#include "InformationManager.h"

//TOMMY
#include "..\..\SparCraft\source\SparCraft.h"
#include "UnitUtil.h"

namespace UAlbertaBot
{
class CombatSimulation
{
	SparCraft::GameState		state;

	//TOMMY
	int current_x;
	int current_y;

public:

	CombatSimulation();

	void setCombatUnits(const BWAPI::Position & center, const int radius);

	SparCraft::ScoreType simulateCombat();

	const SparCraft::Unit			getSparCraftUnit(const UnitInfo & ui) const;
    const SparCraft::Unit			getSparCraftUnit(BWAPI::Unit unit) const;
	const SparCraft::GameState &	getSparCraftState() const;

	const SparCraft::IDType getSparCraftPlayerID(BWAPI::Player player) const;

	void logState(const SparCraft::GameState & state);

	//PAST THIS POINT IS TOMMY
	void addToState(BWAPI::Unit unit); // adds to a simulation a single unit
	void addToState(const UnitInfo &ui);
	void addToState(SparCraft::Unit unit);
	void finishMoving(); // call right before simulating combat. actually, may not need to!
	const SparCraft::Unit getSparCraftUnit(BWAPI::UnitType unit, int player, int x, int y) const;	// if player is 1, it is us. if player is 2, enemy. pass in x and y coordinates
	// based on half-tile positions
	void generateMap();	// sets the map for the state
	void addAllyZergling();
	void addAllySunken();
	void generateCurrentSituation();	// if copy constructing fails, use these. this is based on all seen enemies
	void generateCurrentSituation2();
};
}