#include "CombatSimulation.h"

using namespace UAlbertaBot;

CombatSimulation::CombatSimulation()
{
	//TOMMY
	current_x = 17;
	current_y = 17;
}

// sets the starting states based on the combat units within a radius of a given position
// this center will most likely be the position of the forwardmost combat unit we control
void CombatSimulation::setCombatUnits(const BWAPI::Position & center, const int radius)
{
	SparCraft::GameState s;

	BWAPI::Broodwar->drawCircleMap(center.x, center.y, 10, BWAPI::Colors::Red, true);

	BWAPI::Unitset ourCombatUnits;
	std::vector<UnitInfo> enemyCombatUnits;

	MapGrid::Instance().GetUnits(ourCombatUnits, center, Config::Micro::CombatRegroupRadius, true, false);
	InformationManager::Instance().getNearbyForce(enemyCombatUnits, center, BWAPI::Broodwar->enemy(), Config::Micro::CombatRegroupRadius);

	for (auto & unit : ourCombatUnits)
	{
        if (unit->getType().isWorker())
        {
            continue;
        }

        if (InformationManager::Instance().isCombatUnit(unit->getType()) && SparCraft::System::isSupportedUnitType(unit->getType()))
		{
            try
            {
			    s.addUnit(getSparCraftUnit(unit));
            }
            catch (int e)
            {
                e=1;
                BWAPI::Broodwar->printf("Problem Adding Self Unit with ID: %d", unit->getID());
            }
		}
	}

	for (UnitInfo & ui : enemyCombatUnits)
	{
        if (ui.type.isWorker())
        {
            continue;
        }

        
        if (ui.type == BWAPI::UnitTypes::Terran_Bunker)
        {
            double hpRatio = static_cast<double>(ui.lastHealth) / ui.type.maxHitPoints();

            SparCraft::Unit marine( BWAPI::UnitTypes::Terran_Marine,
                            SparCraft::Position(ui.lastPosition), 
                            ui.unitID, 
                            getSparCraftPlayerID(ui.player), 
                            static_cast<int>(BWAPI::UnitTypes::Terran_Marine.maxHitPoints() * hpRatio), 
                            0,
		                    BWAPI::Broodwar->getFrameCount(), 
                            BWAPI::Broodwar->getFrameCount());	

            for (size_t i(0); i < 5; ++i)
            {
                s.addUnit(marine);
            }
            
            continue;
        }

        if (!ui.type.isFlyer() && SparCraft::System::isSupportedUnitType(ui.type) && ui.completed)
		{
            try
            {
			    s.addUnit(getSparCraftUnit(ui));
            }
            catch (int e)
            {
                BWAPI::Broodwar->printf("Problem Adding Enemy Unit with ID: %d %d", ui.unitID, e);
            }
		}
	}

	s.finishedMoving();

	state = s;
}

// Gets a SparCraft unit from a BWAPI::Unit, used for our own units since we have all their info
const SparCraft::Unit CombatSimulation::getSparCraftUnit(BWAPI::Unit unit) const
{
    return SparCraft::Unit( unit->getType(),
                            SparCraft::Position(unit->getPosition()), 
                            unit->getID(), 
                            getSparCraftPlayerID(unit->getPlayer()), 
                            unit->getHitPoints() + unit->getShields(), 
                            0,
		                    BWAPI::Broodwar->getFrameCount(), 
                            BWAPI::Broodwar->getFrameCount());	
}

// Gets a SparCraft unit from a UnitInfo struct, needed to get units of enemy behind FoW
const SparCraft::Unit CombatSimulation::getSparCraftUnit(const UnitInfo & ui) const
{
	BWAPI::UnitType type = ui.type;

    // this is a hack, treat medics as a marine for now
	if (type == BWAPI::UnitTypes::Terran_Medic)
	{
		type = BWAPI::UnitTypes::Terran_Marine;
	}

    return SparCraft::Unit( ui.type, 
                            SparCraft::Position(ui.lastPosition), 
                            ui.unitID, 
                            getSparCraftPlayerID(ui.player), 
                            ui.lastHealth, 
                            0,
		                    BWAPI::Broodwar->getFrameCount(), 
                            BWAPI::Broodwar->getFrameCount());	
}

SparCraft::ScoreType CombatSimulation::simulateCombat()
{
    try
    {
	    SparCraft::GameState s1(state);

        SparCraft::PlayerPtr selfNOK(new SparCraft::Player_NOKDPS(getSparCraftPlayerID(BWAPI::Broodwar->self())));

	    SparCraft::PlayerPtr enemyNOK(new SparCraft::Player_NOKDPS(getSparCraftPlayerID(BWAPI::Broodwar->enemy())));

	    SparCraft::Game g (s1, selfNOK, enemyNOK, 2000);

	    g.play();
	
	    SparCraft::ScoreType eval =  g.getState().eval(SparCraft::Players::Player_One, SparCraft::EvaluationMethods::LTD2).val();

        if (Config::Debug::DrawCombatSimulationInfo)
        {
            std::stringstream ss1;
            ss1 << "Initial State:\n";
            ss1 << s1.toStringCompact() << "\n\n";

            std::stringstream ss2;

            ss2 << "Predicted Outcome: " << eval << "\n";
            ss2 << g.getState().toStringCompact() << "\n";

            BWAPI::Broodwar->drawTextScreen(150,200,"%s", ss1.str().c_str());
            BWAPI::Broodwar->drawTextScreen(300,200,"%s", ss2.str().c_str());

	        BWAPI::Broodwar->drawTextScreen(240, 280, "Combat Sim : %d", eval);
        }
        
	    return eval;
    }
    catch (int e)
    {
        BWAPI::Broodwar->printf("SparCraft FatalError, simulateCombat() threw");

        return e;
    }
}

const SparCraft::GameState & CombatSimulation::getSparCraftState() const
{
	return state;
}

const SparCraft::IDType CombatSimulation::getSparCraftPlayerID(BWAPI::Player player) const
{
	if (player == BWAPI::Broodwar->self())
	{
		return SparCraft::Players::Player_One;
	}
	else if (player == BWAPI::Broodwar->enemy())
	{
		return SparCraft::Players::Player_Two;
	}

	return SparCraft::Players::Player_None;
}

//PAST THIS POINT IS TOMMY
void CombatSimulation::addToState(BWAPI::Unit unit)
{
	// if (!SparCraft::System::isSupportedUnitType(unit->getType()))

	try
	{
		state.addUnit(getSparCraftUnit(unit));
	}
	catch (int e)
	{
		e = 1;
		BWAPI::Broodwar->printf("Problem Adding Self Unit with ID: %d", unit->getID());
	}
}

//NEW
void CombatSimulation::addToState(const UnitInfo &ui)
{
	// if (!SparCraft::System::isSupportedUnitType(unit->getType()))

	try
	{
		state.addUnit(getSparCraftUnit(ui));
	}
	catch (int e)
	{
		e = 1;
		BWAPI::Broodwar->printf("Problem Adding Self Unit");
	}
}

void CombatSimulation::addToState(SparCraft::Unit unit)
{
	// if (!SparCraft::System::isSupportedUnitType(unit->getType()))

	try
	{
		state.addUnit(unit);
	}
	catch (int e)
	{
		e = 1;
		BWAPI::Broodwar->printf("Problem Adding Self Unit");
	}
}

//NEW
void CombatSimulation::finishMoving()
{
	state.finishedMoving();
}

const SparCraft::Unit CombatSimulation::getSparCraftUnit(BWAPI::UnitType unit, int player, int x, int y) const
{
	SparCraft::IDType p;
	if (player == 1)
	{
		p = SparCraft::Players::Player_One;
	}
	else
	{
		p = SparCraft::Players::Player_Two;
	}
	SparCraft::Position position(x, y);
	SparCraft::Unit sparUnit(unit, p, position);
	return sparUnit;
}

void CombatSimulation::generateMap()
{
	SparCraft::Map smallMap(32, 32);
	state.setMap(&smallMap);
}

void CombatSimulation::addAllyZergling()
{
	SparCraft::Unit sparUnit = getSparCraftUnit(BWAPI::UnitTypes::Zerg_Zergling, 1, current_x, current_y);
	addToState(sparUnit);
	current_x += 16;
	if (current_x >= 129)
	{
		current_x = 17;
		current_y += 32;
	}
}

void CombatSimulation::addAllySunken()
{
	SparCraft::Unit sparUnit = getSparCraftUnit(BWAPI::UnitTypes::Zerg_Sunken_Colony, 1, current_x, current_y);
	addToState(sparUnit);
	current_x += 16;
	if (current_x >= 129)
	{
		current_x = 17;
		current_y += 32;
	}
}

// currently need to adjust this to make all units hypothetical
void CombatSimulation::generateCurrentSituation()
{
	// add our units along one side of the map
	// space them half a tile apart
	// when you have 7 units in a row start a new row one tile under
	int xPos = 17;
	int yPos = 17;
	SparCraft::Unit sparUnit;
	int ourCombatUnits = 0;
	for (auto &unit : BWAPI::Broodwar->self()->getUnits())
	{
		if ((UnitUtil::IsCombatUnit(unit)) || (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony))	// will this code add our sunkens to the sim? (I think so)
		{
			sparUnit = getSparCraftUnit(unit->getType(), 1, xPos, yPos);
			addToState(sparUnit);
			xPos += 16;
			if (xPos >= 129)
			{
				xPos = 17;
				yPos += 32;
			}
			ourCombatUnits++;
		}
	}
	BWAPI::Broodwar->printf("We have this many units: %d\n", ourCombatUnits);

	current_x = xPos;
	current_y = yPos;

	// add enemy units along the other side of the map and let them start at the range of a marine
	int range = BWAPI::UnitTypes::Terran_Marine.groundWeapon().maxRange();
	yPos += range;
	xPos = 17;
	int theirCombatUnits = 0;
	//int theirProducingUnits = 0;
	// using InfoManager to see former enemy units
	// apparently InfoManager can recognize previously seen enemy units, so this is always more accurate than counting
	// currently visible enemy units
	for (auto &unit : InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
	{
		// consider enemy combat units and not static defenses
		if ((UnitUtil::IsCombatUnit(unit.second.unit)) && (!unit.second.type.isBuilding()))
		{
			sparUnit = getSparCraftUnit(unit.second.type, 2, xPos, yPos);
			addToState(sparUnit);
			xPos += 16;
			if (xPos >= 129)
			{
				xPos = 17;
				yPos += 32;
			}
			theirCombatUnits++;
		}

		// if unit is building and can produce a combat unit seen in the enemy's roster
		// add to state all the units of that type the building can produce in the time to build a sunken
		/*
		if (unit.second.type.isBuilding())
		{
			BWAPI::UnitType::set thingsUnitCanMake(unit.second.type.buildsWhat());
			int buildTime;
			int sunkenTime = BWAPI::UnitTypes::Zerg_Sunken_Colony.buildTime() + BWAPI::UnitTypes::Zerg_Creep_Colony.buildTime();
			int unitCount = 0;
			// only consider potential unit production of enemy units we know they have made. is this bad?
			for (auto &seenUnit : InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
			{
				// potential issue: if enemy has been making two types of units from the same building, it will overestimate
				// production. however it is quite unlikely for the enemy to do this early in the game
				if (thingsUnitCanMake.find(seenUnit.second.type) != thingsUnitCanMake.end())
				{
					buildTime = seenUnit.second.type.buildTime();
					if (!(buildTime > 0))
					{
						continue;
					}
					unitCount = sunkenTime / buildTime;
					while (unitCount > 0)
					{
						sparUnit = getSparCraftUnit(seenUnit.second.type, 2, xPos, yPos);
						addToState(sparUnit);
						xPos += 16;
						if (xPos >= 129)
						{
							xPos = 17;
							yPos += 32;
						}
						unitCount--;
						theirProducingUnits++;
					}
				}
			}
		}*/
	}
	BWAPI::Broodwar->printf("They currently have this many units: %d\n", theirCombatUnits);
	//BWAPI::Broodwar->printf("They might make this many more units: %d\n", theirProducingUnits);
}

void CombatSimulation::generateCurrentSituation2()
{
	// add our units along one side of the map
	// space them half a tile apart
	// when you have 7 units in a row start a new row one tile under
	int xPos = 17;
	int yPos = 17;
	SparCraft::Unit sparUnit;
	int ourCombatUnits = 0;
	for (auto &unit : BWAPI::Broodwar->self()->getUnits())
	{
		if ((UnitUtil::IsCombatUnit(unit)) || (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony))	// will this code add our sunkens to the sim? (I think so)
		{
			sparUnit = getSparCraftUnit(unit->getType(), 1, xPos, yPos);
			addToState(sparUnit);
			xPos += 16;
			if (xPos >= 129)
			{
				xPos = 17;
				yPos += 32;
			}
			ourCombatUnits++;
		}
	}
	BWAPI::Broodwar->printf("We have this many units: %d\n", ourCombatUnits);

	current_x = xPos;
	current_y = yPos;

	// add enemy units along the other side of the map and let them start at the range of a marine
	int range = BWAPI::UnitTypes::Terran_Marine.groundWeapon().maxRange();
	yPos += range;
	xPos = 17;
	int theirCombatUnits = 0;
	for (auto &unit : InformationManager::Instance().getEnemyProductionEstimate())
	{
		if (unit.first == BWAPI::UnitTypes::Terran_Barracks)
		{
			for (int i = 0; i < unit.second; i++)
			{
				sparUnit = getSparCraftUnit(BWAPI::UnitTypes::Terran_Marine, 2, xPos, yPos);
				addToState(sparUnit);
				xPos += 16;
				if (xPos >= 129)
				{
					xPos = 17;
					yPos += 32;
				}
				theirCombatUnits++;
			}
		}
		if (unit.first == BWAPI::UnitTypes::Protoss_Gateway)
		{
			for (int j = 0; j < unit.second; j++)
			{
				sparUnit = getSparCraftUnit(BWAPI::UnitTypes::Protoss_Zealot, 2, xPos, yPos);
				addToState(sparUnit);
				xPos += 16;
				if (xPos >= 129)
				{
					xPos = 17;
					yPos += 32;
				}
				theirCombatUnits++;
			}
		}
	}
	BWAPI::Broodwar->printf("They could have this many units: %d\n", theirCombatUnits);
	/*
	//int theirProducingUnits = 0;
	for (auto &unit : InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
	{
		// if unit is building and can produce a combat unit seen in the enemy's roster
		// add to state all the units of that type the building can produce in the time to build a sunken

		if (unit.second.type.isBuilding())
		{
			BWAPI::UnitType::set thingsUnitCanMake(unit.second.type.buildsWhat());
			int buildTime;
			int sunkenTime = BWAPI::UnitTypes::Zerg_Sunken_Colony.buildTime() + BWAPI::UnitTypes::Zerg_Creep_Colony.buildTime();
			int unitCount = 0;
			// only consider potential unit production of enemy units we know they have made. is this bad?
			for (auto &seenUnit : InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
			{
				// potential issue: if enemy has been making two types of units from the same building, it will overestimate
				// production. however it is quite unlikely for the enemy to do this early in the game
				if (thingsUnitCanMake.find(seenUnit.second.type) != thingsUnitCanMake.end())
				{
					buildTime = seenUnit.second.type.buildTime();
					if (!(buildTime > 0))
					{
						continue;
					}
					unitCount = sunkenTime / buildTime;
					while (unitCount > 0)
					{
						sparUnit = getSparCraftUnit(seenUnit.second.type, 2, xPos, yPos);
						addToState(sparUnit);
						xPos += 16;
						if (xPos >= 129)
						{
							xPos = 17;
							yPos += 32;
						}
						unitCount--;
						theirProducingUnits++;
					}
				}
			}
		}
	}
	BWAPI::Broodwar->printf("They could be making this many units: %d\n", theirProducingUnits);
	*/
}