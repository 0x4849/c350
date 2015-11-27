#include "MeleeManager.h"
#include "UnitUtil.h"
//TOMMY
#include "CombatSimulation.h" 

using namespace UAlbertaBot;

MeleeManager::MeleeManager() 
{ 

}

void MeleeManager::executeMicro(const BWAPI::Unitset & targets) 
{
	assignTargetsOld(targets);
}

void MeleeManager::assignTargetsOld(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & meleeUnits = getUnits();

	// figure out targets
	BWAPI::Unitset meleeUnitTargets;
	for (auto & target : targets) 
	{
		// conditions for targeting
		if (!(target->getType().isFlyer()) && 
			!(target->isLifted()) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Larva) && 
			!(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			target->isVisible()) 
		{
			meleeUnitTargets.insert(target);
		}
	}

	// for each meleeUnit
	for (auto & meleeUnit : meleeUnits)
	{
		// if the order is to attack or defend
		if (order.getType() == SquadOrderTypes::Attack || order.getType() == SquadOrderTypes::Defend) 
        {
            // run away if we meet the retreat critereon
            if (meleeUnitShouldRetreat(meleeUnit, targets))
            {
                BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());

                Micro::SmartMove(meleeUnit, fleeTo);
            }
			// if there are targets
			else if (!meleeUnitTargets.empty())
			{
				// find the best target for this meleeUnit
				BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

				// attack it
				Micro::SmartAttackUnit(meleeUnit, target);
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (meleeUnit->getDistance(order.getPosition()) > 100)
				{
					// move to it
					Micro::SmartMove(meleeUnit, order.getPosition());
				}
			}
		}

		//NEW
		if (order.getType() == SquadOrderTypes::Confuse)
		{
			// run away if we meet the retreat critereon. this should probably not be used, but it is uncertain
			//if (meleeUnitShouldRetreat(meleeUnit, targets))
			//{
			//	BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());

			//	Micro::SmartMove(meleeUnit, fleeTo);
			//}
			// if there are targets
			// targets contains units in a radius 800 around each unit
			/*else*/ if (!meleeUnitTargets.empty())
			{
				SparCraft::ScoreType score = 0;
				CombatSimulation sim;
				sim.setCombatUnits(meleeUnit->getPosition(), 1000);
				score = sim.simulateCombat();
				bool fight = score > 0;
				if (fight) // COMBAT SIMULATOR SAYS FIGHT
				{
					// find the best target for this meleeUnit
					BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

					// attack it
					Micro::SmartAttackUnit(meleeUnit, target);

					_currentRegionVertexIndex[meleeUnit] = -1;
				}
				else // COMBAT SIMULATOR SAYS KITE.
				{
					if ((!BWTA::getRegion(meleeUnit->getTilePosition())) ||
						(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy())->getRegion()) == (BWTA::getRegion(meleeUnit->getTilePosition()))) // to do: also move out of enemy base region
					{
						BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());
						Micro::SmartMove(meleeUnit, fleeTo);
						_currentRegionVertexIndex[meleeUnit] = -1;
					}
					else
					{
						double closestZealotDist = std::numeric_limits<double>::infinity();
						BWAPI::Unit closestZealotTarget = nullptr;
						for (auto &unit : meleeUnitTargets)
						{
							if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
							{
								int distance = meleeUnit->getDistance(unit);
								if (!closestZealotTarget || (distance < closestZealotDist))
								{
									closestZealotDist = distance;
									closestZealotTarget = unit;
								}
							}
						}
						calculateCurrentRegionVertices2(meleeUnit);
						followPerimeter(meleeUnit, closestZealotTarget);
					}
				}
			}
			// if there are no targets
			else
			{
				// if we're not near the order position
				if (meleeUnit->getDistance(order.getPosition()) > 100)
				{
					// move to it
					Micro::SmartMove(meleeUnit, order.getPosition());
					_currentRegionVertexIndex[meleeUnit] = -1;
				}
			}
		}

		if (Config::Debug::DrawUnitTargetInfo)
		{
			BWAPI::Broodwar->drawLineMap(meleeUnit->getPosition().x, meleeUnit->getPosition().y, 
			meleeUnit->getTargetPosition().x, meleeUnit->getTargetPosition().y, Config::Debug::ColorLineTarget);
		}
	}
}

std::pair<BWAPI::Unit, BWAPI::Unit> MeleeManager::findClosestUnitPair(const BWAPI::Unitset & attackers, const BWAPI::Unitset & targets)
{
    std::pair<BWAPI::Unit, BWAPI::Unit> closestPair(nullptr, nullptr);
    double closestDistance = std::numeric_limits<double>::max();

    for (auto & attacker : attackers)
    {
        BWAPI::Unit target = getTarget(attacker, targets);
        double dist = attacker->getDistance(attacker);

        if (!closestPair.first || (dist < closestDistance))
        {
            closestPair.first = attacker;
            closestPair.second = target;
            closestDistance = dist;
        }
    }

    return closestPair;
}

// get a target for the meleeUnit to attack
BWAPI::Unit MeleeManager::getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets)
{
	int highPriority = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	BWAPI::Unit closestTarget = nullptr;

	// for each target possiblity
	for (auto & unit : targets)
	{
		int priority = getAttackPriority(meleeUnit, unit);
		int distance = meleeUnit->getDistance(unit);

		// if it's a higher priority, or it's closer, set it
		if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
		{
			closestDist = distance;
			highPriority = priority;
			closestTarget = unit;
		}
	}

	return closestTarget;
}

	// get the attack priority of a type in relation to a zergling
int MeleeManager::getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit) 
{
	BWAPI::UnitType type = unit->getType();

    if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar 
        && unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
        && (BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
    {
        return 13;
    }

	if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar && unit->getType().isWorker())
	{
		return 12;
	}

	// highest priority is something that can attack us or aid in combat
    if (type ==  BWAPI::UnitTypes::Terran_Bunker)
    {
        return 11;
    }
    else if (type == BWAPI::UnitTypes::Terran_Medic || 
		(type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) || 
		type ==  BWAPI::UnitTypes::Terran_Bunker ||
		type == BWAPI::UnitTypes::Protoss_High_Templar ||
		type == BWAPI::UnitTypes::Protoss_Reaver ||
		(type.isWorker() && unitNearChokepoint(unit))) 
	{
		return 10;
	} 
	// next priority is worker
	else if (type.isWorker()) 
	{
		return 9;
	}
    // next is special buildings
	else if (type == BWAPI::UnitTypes::Zerg_Spawning_Pool)
	{
		return 5;
	}
	// next is special buildings
	else if (type == BWAPI::UnitTypes::Protoss_Pylon)
	{
		return 5;
	}
	// next is buildings that cost gas
	else if (type.gasPrice() > 0)
	{
		return 4;
	}
	else if (type.mineralPrice() > 0)
	{
		return 3;
	}
	// then everything else
	else
	{
		return 1;
	}
}

BWAPI::Unit MeleeManager::closestMeleeUnit(BWAPI::Unit target, const BWAPI::Unitset & meleeUnitsToAssign)
{
	double minDistance = 0;
	BWAPI::Unit closest = nullptr;

	for (auto & meleeUnit : meleeUnitsToAssign)
	{
		double distance = meleeUnit->getDistance(target);
		if (!closest || distance < minDistance)
		{
			minDistance = distance;
			closest = meleeUnit;
		}
	}
	
	return closest;
}

bool MeleeManager::meleeUnitShouldRetreat(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets)
{
    // terran don't regen so it doesn't make any sense to retreat
    if (meleeUnit->getType().getRace() == BWAPI::Races::Terran)
    {
        return false;
    }

    // we don't want to retreat the melee unit if its shields or hit points are above the threshold set in the config file
    // set those values to zero if you never want the unit to retreat from combat individually
    if (meleeUnit->getShields() > Config::Micro::RetreatMeleeUnitShields || meleeUnit->getHitPoints() > Config::Micro::RetreatMeleeUnitHP)
    {
        return false;
    }

    // if there is a ranged enemy unit within attack range of this melee unit then we shouldn't bother retreating since it could fire and kill it anyway
    for (auto & unit : targets)
    {
        int groundWeaponRange = unit->getType().groundWeapon().maxRange();
        if (groundWeaponRange >= 64 && unit->getDistance(meleeUnit) < groundWeaponRange)
        {
            return false;
        }
    }

    return true;
}


// still has bug in it somewhere, use Old version
void MeleeManager::assignTargetsNew(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & meleeUnits = getUnits();

	// figure out targets
	BWAPI::Unitset meleeUnitTargets;
	for (auto & target : targets) 
	{
		// conditions for targeting
		if (!(target->getType().isFlyer()) && 
			!(target->isLifted()) &&
			!(target->getType() == BWAPI::UnitTypes::Zerg_Larva) && 
			!(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
			target->isVisible()) 
		{
			meleeUnitTargets.insert(target);
		}
	}

    BWAPI::Unitset meleeUnitsToAssign(meleeUnits);
    std::map<BWAPI::Unit, int> attackersAssigned;

    for (auto & unit : meleeUnitTargets)
    {
        attackersAssigned[unit] = 0;
    }

    int smallThreshold = BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg ? 3 : 1;
    int bigThreshold = BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg ? 12 : 3;

    // keep assigning targets while we have attackers and targets remaining
    while (!meleeUnitsToAssign.empty() && !meleeUnitTargets.empty())
    {
        auto attackerAssignment = findClosestUnitPair(meleeUnitsToAssign, meleeUnitTargets);
        BWAPI::Unit & attacker = attackerAssignment.first;
        BWAPI::Unit & target = attackerAssignment.second;

        UAB_ASSERT_WARNING(attacker, "We should have chosen an attacker!");

        if (!attacker)
        {
            break;
        }

        if (!target)
        {
            Micro::SmartMove(attacker, order.getPosition());
            continue;
        }

        Micro::SmartAttackUnit(attacker, target);

        // update the number of units assigned to attack the target we found
        int & assigned = attackersAssigned[attackerAssignment.second];
        assigned++;

        // if it's a small / fast unit and there's more than 2 things attacking it already, don't assign more
        if ((target->getType().isWorker() || target->getType() == BWAPI::UnitTypes::Zerg_Zergling) && (assigned >= smallThreshold))
        {
            meleeUnitTargets.erase(target);
        }
        // if it's a building and there's more than 10 things assigned to it already, don't assign more
        else if (assigned > bigThreshold)
        {
            meleeUnitTargets.erase(target);
        }

        meleeUnitsToAssign.erase(attacker);
    }

    // if there's no targets left, attack move to the order destination
    if (meleeUnitTargets.empty())
    {
        for (auto & unit : meleeUnitsToAssign)    
        {
			if (unit->getDistance(order.getPosition()) > 100)
			{
				// move to it
				Micro::SmartMove(unit, order.getPosition());
                BWAPI::Broodwar->drawLineMap(unit->getPosition(), order.getPosition(), BWAPI::Colors::Yellow);
			}
        }
    }
}

//PAST THIS POINT IS TOMMY
void MeleeManager::setUnits(const BWAPI::Unitset & u)
{
	_units = u;
	for (auto &unit : _units)
	{
		if (_currentRegionVertexIndex.find(unit) == _currentRegionVertexIndex.end()) {
			_currentRegionVertexIndex[unit] = -1;
		}
	}
}

void MeleeManager::calculateCurrentRegionVertices(BWAPI::Unit unit)
{
	BWTA::Region* currentRegion = BWTA::getRegion(unit->getTilePosition());
	//UAB_ASSERT_WARNING(enemyRegion, "We should have an enemy region if we are fleeing");

	if (!currentRegion)
	{
		UAB_ASSERT_WARNING(currentRegion, "We should have an enemy region if we are fleeing"); // NEW
		return;
	}

	const BWAPI::Position basePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
	const std::vector<BWAPI::TilePosition> & closestTobase = MapTools::Instance().getClosestTilesTo(basePosition);

	std::set<BWAPI::Position> unsortedVertices;

	// check each tile position
	for (size_t i(0); i < closestTobase.size(); ++i)
	{
		const BWAPI::TilePosition & tp = closestTobase[i];

		if (BWTA::getRegion(tp) != currentRegion)
		{
			continue;
		}

		// a tile is 'surrounded' if
		// 1) in all 4 directions there's a tile position in the current region
		// 2) in all 4 directions there's a buildable tile
		bool surrounded = true;
		if (BWTA::getRegion(BWAPI::TilePosition(tp.x + 1, tp.y)) != currentRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x + 1, tp.y))
			|| BWTA::getRegion(BWAPI::TilePosition(tp.x, tp.y + 1)) != currentRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x, tp.y + 1))
			|| BWTA::getRegion(BWAPI::TilePosition(tp.x - 1, tp.y)) != currentRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x - 1, tp.y))
			|| BWTA::getRegion(BWAPI::TilePosition(tp.x, tp.y - 1)) != currentRegion || !BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tp.x, tp.y - 1)))
		{
			surrounded = false;
		}

		// push the tiles that aren't surrounded
		if (!surrounded && BWAPI::Broodwar->isBuildable(tp))
		{
			if (Config::Debug::DrawScoutInfo)
			{
				int x1 = tp.x * 32 + 2;
				int y1 = tp.y * 32 + 2;
				int x2 = (tp.x + 1) * 32 - 2;
				int y2 = (tp.y + 1) * 32 - 2;

				BWAPI::Broodwar->drawTextMap(x1 + 3, y1 + 2, "%d", MapTools::Instance().getGroundDistance(BWAPI::Position(tp), basePosition));
				BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Green, false);
			}

			unsortedVertices.insert(BWAPI::Position(tp) + BWAPI::Position(16, 16));
		}
	}


	std::vector<BWAPI::Position> sortedVertices;
	BWAPI::Position current = *unsortedVertices.begin();

	_currentRegionVertices[unit].push_back(current);
	unsortedVertices.erase(current);

	// while we still have unsorted vertices left, find the closest one remaining to current
	while (!unsortedVertices.empty())
	{
		double bestDist = 1000000;
		BWAPI::Position bestPos;

		for (const BWAPI::Position & pos : unsortedVertices)
		{
			double dist = pos.getDistance(current);

			if (dist < bestDist)
			{
				bestDist = dist;
				bestPos = pos;
			}
		}

		current = bestPos;
		sortedVertices.push_back(bestPos);
		unsortedVertices.erase(bestPos);
	}

	// let's close loops on a threshold, eliminating death grooves
	int distanceThreshold = 100;

	while (true)
	{
		// find the largest index difference whose distance is less than the threshold
		int maxFarthest = 0;
		int maxFarthestStart = 0;
		int maxFarthestEnd = 0;

		// for each starting vertex
		for (int i(0); i < (int)sortedVertices.size(); ++i)
		{
			int farthest = 0;
			int farthestIndex = 0;

			// only test half way around because we'll find the other one on the way back
			for (size_t j(1); j < sortedVertices.size() / 2; ++j)
			{
				int jindex = (i + j) % sortedVertices.size();

				if (sortedVertices[i].getDistance(sortedVertices[jindex]) < distanceThreshold)
				{
					farthest = j;
					farthestIndex = jindex;
				}
			}

			if (farthest > maxFarthest)
			{
				maxFarthest = farthest;
				maxFarthestStart = i;
				maxFarthestEnd = farthestIndex;
			}
		}

		// stop when we have no long chains within the threshold
		if (maxFarthest < 4)
		{
			break;
		}

		double dist = sortedVertices[maxFarthestStart].getDistance(sortedVertices[maxFarthestEnd]);

		std::vector<BWAPI::Position> temp;

		for (size_t s(maxFarthestEnd); s != maxFarthestStart; s = (s + 1) % sortedVertices.size())
		{
			temp.push_back(sortedVertices[s]);
		}

		sortedVertices = temp;
	}

	_currentRegionVertices[unit] = sortedVertices;
}

int MeleeManager::getClosestVertexIndex(BWAPI::Unit unit)
{
	int closestIndex = -1;
	double closestDistance = 10000000;

	//BWAPI::Broodwar->printf((std::to_string(_currentRegionVertices[unit].size())).c_str()); // debug. seems to be OK

	for (size_t i(0); i < _currentRegionVertices[unit].size(); ++i)
	{
		double dist = unit->getDistance(_currentRegionVertices[unit][i]);
		if (dist < closestDistance)
		{
			closestDistance = dist;
			closestIndex = i;
		}
	}

	return closestIndex;
}

BWAPI::Position MeleeManager::getFleePosition(BWAPI::Unit unit, BWAPI::Unit enemy)
{
	UAB_ASSERT_WARNING(!_currentRegionVertices[unit].empty(), "We should have an enemy region vertices if we are fleeing");

	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());

	// if this is the first flee, we will not have a previous perimeter index
	if (_currentRegionVertexIndex[unit] == -1)
	{
		// so return the closest position in the polygon
		int closestPolygonIndex = getClosestVertexIndex(unit);

		//BWAPI::Broodwar->printf((std::to_string(closestPolygonIndex)).c_str());

		UAB_ASSERT_WARNING(closestPolygonIndex != -1, "Couldn't find a closest vertex"); // changed from != to ==

		if (closestPolygonIndex == -1)
		{
			//BWAPI::Broodwar->printf("wtf"); // debug
			return BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
		}
		else
		{
			// set the current index so we know how to iterate if we are still fleeing later
			_currentRegionVertexIndex[unit] = closestPolygonIndex;
			return _currentRegionVertices[unit][closestPolygonIndex];
		}
	}
	// if we are still fleeing from the previous frame, get the next location if we are close enough
	else
	{
		double distanceFromCurrentVertex = _currentRegionVertices[unit][_currentRegionVertexIndex[unit]].getDistance(unit->getPosition());

		// keep going to the next vertex in the perimeter until we get to one we're far enough from to issue another move command
		// cycle through vertices clockwise or counterclockwise depending on where the enemy is
		// THE FOLLOWING CODE SOMETIMES IMPROVES PATHING BUT ALSO MAY FAIL PATHING OR CAUSE CRASHES
		//int posDist = enemy->getDistance(_currentRegionVertices[unit][(_currentRegionVertexIndex[unit] + 1) % _currentRegionVertices[unit].size()]);
		//int negDist = enemy->getDistance(_currentRegionVertices[unit][(_currentRegionVertexIndex[unit] - 1) % _currentRegionVertices[unit].size()]);

		//if (posDist >= negDist)
		//{
		while (distanceFromCurrentVertex < 192)
		{
			_currentRegionVertexIndex[unit] = (_currentRegionVertexIndex[unit] + 1) % _currentRegionVertices[unit].size();
			distanceFromCurrentVertex = _currentRegionVertices[unit][_currentRegionVertexIndex[unit]].getDistance(unit->getPosition());
		}
		//}
		//else
		//{
		//	while (distanceFromCurrentVertex < 192)
		//	{
		//		_currentRegionVertexIndex[unit] = (_currentRegionVertexIndex[unit] - 1) % _currentRegionVertices[unit].size();
		//		distanceFromCurrentVertex = _currentRegionVertices[unit][_currentRegionVertexIndex[unit]].getDistance(unit->getPosition());
		//	}
		//}
		return _currentRegionVertices[unit][_currentRegionVertexIndex[unit]];
	}
}

void MeleeManager::followPerimeter(BWAPI::Unit unit, BWAPI::Unit enemy)
{
	BWAPI::Position fleeTo = getFleePosition(unit, enemy);

	if (Config::Debug::DrawScoutInfo)
	{
		BWAPI::Broodwar->drawCircleMap(fleeTo, 5, BWAPI::Colors::Red, true);
	}

	Micro::SmartMove(unit, fleeTo);
}

// 2ND VERSION OF REGION KITE

void MeleeManager::calculateCurrentRegionVertices2(BWAPI::Unit unit)
{
	BWTA::Region* currentRegion = BWTA::getRegion(unit->getTilePosition());
	//UAB_ASSERT_WARNING(enemyRegion, "We should have an enemy region if we are fleeing");

	if (!currentRegion)
	{
		UAB_ASSERT_WARNING(currentRegion, "We should have an enemy region if we are fleeing"); // NEW
		return;
	}

	_currentRegionVertices[unit] = currentRegion->getPolygon();
}

/* CURRENT BEST ITERATION OF ZERG KITING
if (!meleeUnitTargets.empty())
{
SparCraft::ScoreType score = 0;
CombatSimulation sim;
sim.setCombatUnits(meleeUnit->getPosition(), 1000);
score = sim.simulateCombat();
bool fight = score > 0;
if (fight) // COMBAT SIMULATOR SAYS FIGHT
{
// find the best target for this meleeUnit
BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

// attack it
Micro::SmartAttackUnit(meleeUnit, target);
}
else // COMBAT SIMULATOR SAYS KITE
{
// attempt to prevent issuing too many move commands
if (BWAPI::Broodwar->getFrameCount() % 30 != 0)
{
return;
}
double closestZealotDist = std::numeric_limits<double>::infinity();
BWAPI::Unit closestZealotTarget = nullptr;
for (auto &unit : targets)
{
if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
{
int distance = meleeUnit->getDistance(unit);
if (!closestZealotTarget || (distance < closestZealotDist))
{
closestZealotDist = distance;
closestZealotTarget = unit;
}
}
}
// generate random vector that: moves 800u away from the zealot,
// does not come within 500 units of our base, and is walkable
// TO-DO: prevent reissuing a movement command until the prior is complete
BWAPI::Position kiteTo;
int baseDist = meleeUnit->getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition());
int distX = closestZealotTarget->getPosition().x - meleeUnit->getPosition().x;
int distY = closestZealotTarget->getPosition().y - meleeUnit->getPosition().y;
double distAngle = (acos(distX / (sqrt((distX*distX) + (distY*distY))))) * 180 / 3.141593;
bool inMainBase = true;
double distFromBase;
int randAngle;
int newX;
int newY;
while (inMainBase)
{
do
{
randAngle = (rand() % 180) + 90 + distAngle;
newY = 800 * sin(randAngle * 3.141593 / 180);
newX = 800 * cos(randAngle * 3.141593 / 180);
newY += meleeUnit->getPosition().y;
newX += meleeUnit->getPosition().x;
kiteTo = BWAPI::Point<int, 1>::Point(static_cast<int>(newX), static_cast<int>(newY));
} while (!checkPositionWalkable2(kiteTo));
distFromBase = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition().getApproxDistance(kiteTo);
if (distFromBase > 500) { inMainBase = false; }
}
Micro::SmartMove(meleeUnit, kiteTo);
}
*/


/* RANDOM CODE FOR ZERG KITING
else // COMBAT SIMULATOR SAYS KITE
{
if (regionKiting) // kiting type 1: new
{
double closestZealotDist = std::numeric_limits<double>::infinity();
BWAPI::Unit closestZealotTarget = nullptr;
for (auto &unit : targets)
{
if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
{
int distance = meleeUnit->getDistance(unit);
if (!closestZealotTarget || (distance < closestZealotDist))
{
closestZealotDist = distance;
closestZealotTarget = unit;
}
}
}
BWAPI::Position unitPos = meleeUnit->getPosition();
BWAPI::TilePosition unitTilePos = BWAPI::Point<int, 32>::Point(static_cast<int>(unitPos.x / 32), static_cast<int>(unitPos.y / 32));
BWTA::Region* unitRegion = BWTA::getRegion(unitTilePos);
// if you aren't that close to the zealot (1300 units away), move 50 units in a random direction away from base towards the zealot
if ((closestZealotTarget) && (closestZealotDist > 1300))
{
// this block has code for running away from our base
BWAPI::Position kiteTo;
int baseDist = meleeUnit->getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition());
int zealotDist = meleeUnit->getDistance(closestZealotTarget);
bool toMainBase = true;
bool toZealot = false;
int randAngle;
int newX;
int newY;
int newBaseDist;
int newZealotDist;
while (toMainBase || (!toZealot))
{
do
{
randAngle = rand() % 360;
newY = 50 * sin(randAngle * 3.141593 / 180);
newX = 50 * cos(randAngle * 3.141593 / 180);
newY += unitPos.y;
newX += unitPos.x;
kiteTo = BWAPI::Point<int, 1>::Point(static_cast<int>(newX), static_cast<int>(newY));
} while (!checkPositionWalkable2(kiteTo));
newBaseDist = kiteTo.getApproxDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition());
newZealotDist = closestZealotTarget->getDistance(kiteTo);
if (newBaseDist > baseDist) { toMainBase = false; }
if (newZealotDist < zealotDist) { toZealot = true; }
}
Micro::SmartMove(meleeUnit, kiteTo);
}
else if (!unitRegion) // standing in chokepoint
{
Micro::SmartMove(meleeUnit, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
}
// if Zealot is found, move to the nearest region polygon position that is away from the zealot
// then just run around the polygon
else if (closestZealotTarget)
{
// this block has code for just not going in the BWTA::Region of our base
BWAPI::Position kiteTo;
BWAPI::Position basePosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition();
int zealotDist = meleeUnit->getDistance(closestZealotTarget);
bool inMainBase = true;
bool toZealot = true;
int randAngle;
int newX;
int newY;
int newBaseDist;
int newZealotDist;
BWTA::Region* baseRegion;
BWTA::Region* kiteToRegion;
BWAPI::TilePosition kiteToTile;
while (inMainBase || toZealot)
{
do
{
randAngle = rand() % 360;
newY = 950 * sin(randAngle * 3.141593 / 180);
newX = 950 * cos(randAngle * 3.141593 / 180);
newY += unitPos.y;
newX += unitPos.x;
kiteTo = BWAPI::Point<int, 1>::Point(static_cast<int>(newX), static_cast<int>(newY));
kiteToTile = BWAPI::Point<int, 32>::Point(static_cast<int>(kiteTo.x / 32), static_cast<int>(kiteTo.y / 32));
} while ((!checkPositionWalkable2(kiteTo)) ||
((BWTA::getGroundDistance(unitTilePos, kiteToTile)) > (kiteTo.getApproxDistance(unitPos) + 250))); // trying to make sure that you aren't walking around terrain
// use of MapTools getGroundDistance so frequently is iffy
// try BWTA getGroundDistance
baseRegion = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getRegion();
kiteToRegion = BWTA::getRegion(kiteToTile);
newZealotDist = closestZealotTarget->getDistance(kiteTo);
if (baseRegion != kiteToRegion) { inMainBase = false; }
if (newZealotDist > zealotDist) { toZealot = false; }
}
Micro::SmartMove(meleeUnit, kiteTo);
}
// if no Zealots, behave like an Attack unit
else
{
// find the best target for this meleeUnit
BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

// attack it
Micro::SmartAttackUnit(meleeUnit, target);
}
}
else // kiting type 2
{
// assuming that Confuse strategy is not used if non-Zealot combat units are seen
// search meleeUnitTargets for nearest Zealot
double closestZealotDist = std::numeric_limits<double>::infinity();
BWAPI::Unit closestZealotTarget = nullptr;
for (auto &unit : targets)
{
if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
{
int distance = meleeUnit->getDistance(unit);
if (!closestZealotTarget || (distance < closestZealotDist))
{
closestZealotDist = distance;
closestZealotTarget = unit;
}
}
}
// if you aren't that close to the zealot (1300 units away), move 50 units in a random direction away from base towards the zealot
if ((closestZealotTarget) && (closestZealotDist > 1300))
{
// this block has code for running away from our base
BWAPI::Position unitPos = meleeUnit->getPosition();
BWAPI::Position kiteTo;
int baseDist = meleeUnit->getDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition());
int zealotDist = meleeUnit->getDistance(closestZealotTarget);
bool toMainBase = true;
bool toZealot = false;
int randAngle;
int newX;
int newY;
int newBaseDist;
int newZealotDist;
while (toMainBase || (!toZealot))
{
do
{
randAngle = rand() % 360;
newY = 50 * sin(randAngle * 3.141593 / 180);
newX = 50 * cos(randAngle * 3.141593 / 180);
newY += unitPos.y;
newX += unitPos.x;
kiteTo = BWAPI::Point<int, 1>::Point(static_cast<int>(newX), static_cast<int>(newY));
} while (!checkPositionWalkable2(kiteTo));
newBaseDist = kiteTo.getApproxDistance(InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition());
newZealotDist = closestZealotTarget->getDistance(kiteTo);
if (newBaseDist > baseDist) { toMainBase = false; }
if (newZealotDist < zealotDist) { toZealot = true; }
}
Micro::SmartMove(meleeUnit, kiteTo);
}
// if Zealot is found, get a position to flee to that increases distance to the main base and the nearest zealot
// IMPORTANT: fleeing distance must be <= simulator radius == order radius. current flee = 950
else if (closestZealotTarget)
{
// this block has code for just not going in the BWTA::Region of our base
BWAPI::Position unitPos = meleeUnit->getPosition();
BWAPI::Position kiteTo;
BWAPI::Position basePosition = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getPosition();
int zealotDist = meleeUnit->getDistance(closestZealotTarget);
bool inMainBase = true;
bool toZealot = true;
int randAngle;
int newX;
int newY;
int newBaseDist;
int newZealotDist;
BWTA::Region* baseRegion;
BWTA::Region* kiteToRegion;
BWAPI::TilePosition unitTilePos = BWAPI::Point<int, 32>::Point(static_cast<int>(unitPos.x/32), static_cast<int>(unitPos.y/32));
BWAPI::TilePosition kiteToTile;
while (inMainBase || toZealot)
{
do
{
randAngle = rand() % 360;
newY = 950 * sin(randAngle * 3.141593 / 180);
newX = 950 * cos(randAngle * 3.141593 / 180);
newY += unitPos.y;
newX += unitPos.x;
kiteTo = BWAPI::Point<int, 1>::Point(static_cast<int>(newX), static_cast<int>(newY));
kiteToTile = BWAPI::Point<int, 32>::Point(static_cast<int>(kiteTo.x/32), static_cast<int>(kiteTo.y/32));
} while ((!checkPositionWalkable2(kiteTo)) ||
((BWTA::getGroundDistance(unitTilePos, kiteToTile)) > (kiteTo.getApproxDistance(unitPos) + 250))); // trying to make sure that you aren't walking around terrain
// use of MapTools getGroundDistance so frequently is iffy
// try BWTA getGroundDistance
baseRegion = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->self())->getRegion();
kiteToRegion = BWTA::getRegion(kiteToTile);
newZealotDist = closestZealotTarget->getDistance(kiteTo);
if (baseRegion != kiteToRegion) { inMainBase = false; }
if (newZealotDist > zealotDist) { toZealot = false; }
}
Micro::SmartMove(meleeUnit, kiteTo);
}
// if no Zealots, behave like an Attack unit
else
{
// find the best target for this meleeUnit
BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

// attack it
Micro::SmartAttackUnit(meleeUnit, target);
}
}
}
*/