#include "ScoutManager.h"
#include "ProductionManager.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

ScoutManager::ScoutManager() 
    : _scouter1(nullptr)
	, _scouter2(nullptr)
	, _overlordScout(nullptr)
{
}

ScoutManager & ScoutManager::Instance() 
{
	static ScoutManager instance;
	return instance;
}

void ScoutManager::update()
{
	moveScouts();
}

bool ScoutManager::setScout(BWAPI::Unit unit)
{
    // if we have a previous worker scout, release it back to the worker manager
	if (_scouter1 && _scouter2) return false;
	if (_scouter1)_scouter2 = unit;
	else _scouter1 = unit;
	return true;
}

void ScoutManager::moveScouts()
{
	if (!_overlordScout && BWAPI::Broodwar->enemy()->getRace() != BWAPI::Races::Terran)
	{
		for (auto & unit : BWAPI::Broodwar->self()->getUnits())
		{
			if (UnitUtil::IsValidUnit(unit) && unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
			{
				_overlordScout = unit;
			}
		}
	}
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	if (enemyBaseLocation)
	{
		if (_overlordScout) Micro::SmartMove(_overlordScout,BWTA::getNearestChokepoint(enemyBaseLocation->getPosition())->getCenter());
		if (_scouter1) Micro::SmartMove(_scouter1, BWAPI::Position(enemyBaseLocation->getTilePosition()));
		if (_scouter2) Micro::SmartMove(_scouter2, BWAPI::Position(enemyBaseLocation->getTilePosition()));
	}
	else 
	{
		if (_overlordScout) Micro::SmartMove(_overlordScout, BWAPI::Position(BWAPI::Broodwar->mapWidth() * 16, BWAPI::Broodwar->mapHeight() * 16));
		int count = 0;
		for (BWTA::BaseLocation * startLocation : BWTA::getStartLocations())
		{
			if (!BWAPI::Broodwar->isExplored(startLocation->getTilePosition()))
			{
				if (_scouter1 && !count) Micro::SmartMove(_scouter1, BWAPI::Position(startLocation->getTilePosition()));
				if (_scouter2 && count == 1) Micro::SmartMove(_scouter2, BWAPI::Position(startLocation->getTilePosition()));
				++count;
			}
		}
	}
}