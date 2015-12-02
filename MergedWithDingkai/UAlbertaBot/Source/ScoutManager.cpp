#include "ScoutManager.h"
#include "ProductionManager.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;

ScoutManager::ScoutManager()
	: _scouter1(nullptr)
	, _scouter2(nullptr)
	, _overlordScout(nullptr)
	, _workerScout(nullptr)
	, _enemyRamp(BWAPI::Position(0, 0))
	, _scoutLocation1(BWAPI::Position(0, 0))
	, _scoutLocation2(BWAPI::Position(0, 0))
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

void ScoutManager::setWorkerScout(BWAPI::Unit unit)
{
	// if we have a previous worker scout, release it back to the worker manager
	if (_workerScout)
	{
		WorkerManager::Instance().finishedWithWorker(_workerScout);
	}

	_workerScout = unit;
	WorkerManager::Instance().setScoutWorker(_workerScout);
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
		if (_enemyRamp == BWAPI::Position(0, 0)) _enemyRamp = getEnemyRamp();
		if (_overlordScout) Micro::SmartMove(_overlordScout, _enemyRamp);
		if (_scouter1) {
			Micro::SmartMove(_scouter1, BWAPI::Position(enemyBaseLocation->getTilePosition()));
		}
		if (_scouter2) Micro::SmartMove(_scouter2, _enemyRamp);
		if (_workerScout)
		{
			WorkerManager::Instance().finishedWithWorker(_workerScout);
			_workerScout = nullptr;
		}
	}
	else
	{
		if (_scoutLocation1 == BWAPI::Position(0, 0)||_scoutLocation2 == BWAPI::Position(0,0)){
			for (BWTA::BaseLocation * startLocation : BWTA::getStartLocations())
			{
				if (!BWAPI::Broodwar->isExplored(startLocation->getTilePosition()))
				{
					if (_scoutLocation1 == BWAPI::Position(0, 0) && startLocation->getPosition()!=_scoutLocation2) { _scoutLocation1 = startLocation->getPosition(); }
					else if (_scoutLocation2 == BWAPI::Position(0, 0) && startLocation->getPosition() != _scoutLocation1){ _scoutLocation2 = startLocation->getPosition(); }
					else if (startLocation->getPosition() != _scoutLocation1 && startLocation->getPosition() != _scoutLocation2) { _scoutLocation2 = startLocation->getPosition(); }
				}
			}
			if (_scoutLocation2 == BWAPI::Position(0, 0))
			{
				_scoutLocation2 = _scoutLocation1;
			}
			if (_scoutLocation1 == BWAPI::Position(0, 0))
			{
				_scoutLocation1 = _scoutLocation2;
			}
		}
		if (_scouter1) 
		{
			Micro::SmartMove(_scouter1, _scoutLocation1);
			if (BWAPI::Broodwar->isExplored(BWAPI::TilePosition(_scoutLocation1))){
				_scoutLocation1 = BWAPI::Position(0, 0);
			}
		}
		if (_scouter2) 
		{
			Micro::SmartMove(_scouter2, _scoutLocation2);
			if (BWAPI::Broodwar->isExplored(BWAPI::TilePosition(_scoutLocation2))){
				_scoutLocation2 = BWAPI::Position(0, 0);
			}
		}
		if (_workerScout)
		{
			Micro::SmartMove(_workerScout, _scoutLocation1);
			if (BWAPI::Broodwar->isExplored(BWAPI::TilePosition(_scoutLocation1))){
				_scoutLocation1 = BWAPI::Position(0, 0);
			}

		}
		if (_overlordScout)
		{
			if (Config::Strategy::StrategyName == Config::Strategy::AgainstZergStrategyName)
			{
				Micro::SmartMove(_overlordScout, _scoutLocation2);
				if (BWAPI::Broodwar->isExplored(BWAPI::TilePosition(_scoutLocation2))){
					_scoutLocation2 = BWAPI::Position(0, 0);
				}
			}
			else 
			{
				Micro::SmartMove(_overlordScout, BWAPI::Position(BWAPI::Broodwar->mapWidth() * 16, BWAPI::Broodwar->mapHeight() * 16));
			}
		}
	}
}

BWAPI::TilePosition ScoutManager::getFarthestPoint()
{
	double distance = 1;
	double expDist = 999999.0;
	BWTA::BaseLocation * farthestLocation;
	BWTA::BaseLocation * myExpansion;
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	const std::set<BWTA::BaseLocation*, std::less<BWTA::BaseLocation*>> locations = BWTA::getBaseLocations();

	for (auto x : locations)
	{
		double myDist = BWTA::getGroundDistance(BWTA::getStartLocation(BWAPI::Broodwar->self())->getTilePosition(), x->getTilePosition());
		if (myDist >= 1 && myDist < expDist && !x->isMineralOnly() && x != BWTA::getStartLocation(BWAPI::Broodwar->self()))
		{
			myExpansion = x;
			expDist = myDist;

		}
	}

	for (auto x : locations)
	{
		double myDist = BWTA::getGroundDistance(enemyBaseLocation->getTilePosition(), x->getTilePosition());
		if (myDist >= 1 && !x->isMineralOnly() && x != enemyBaseLocation && x != BWTA::getStartLocation(BWAPI::Broodwar->self()) && x != myExpansion && myDist > distance)
		{
			distance = myDist;
			farthestLocation = x;

		}
	}
	BuildingManager::Instance().createdHatcheriesVector[1] = farthestLocation->getTilePosition();
	return farthestLocation->getTilePosition();
}

BWAPI::Position ScoutManager::getEnemyRamp(){
	std::set<BWTA::Chokepoint *> chokePoints = BWTA::getChokepoints();
	double lowestDistance = 999999.0;
	double lowestDistance2 = 999999.0;
	double expDist = 999999.0;
	BWAPI::Position rampPosition;
	BWTA::BaseLocation * expansion;
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	const std::set<BWTA::BaseLocation*, std::less<BWTA::BaseLocation*>> locations = BWTA::getBaseLocations();
	for (auto x : locations)
	{
		double myDist = BWTA::getGroundDistance(enemyBaseLocation->getTilePosition(), x->getTilePosition());
		if (myDist >= 1 && myDist < expDist && !x->isMineralOnly() && x != enemyBaseLocation)
		{
			expansion = x;
			expDist = myDist;
		}
	}

	for (auto x : chokePoints)
	{
		double distance1 = BWTA::getGroundDistance(expansion->getTilePosition(), BWAPI::TilePosition(x->getCenter()));
		double distance2 = BWTA::getGroundDistance(enemyBaseLocation->getTilePosition(), BWAPI::TilePosition(x->getCenter()));
		double sum = distance1 + distance2;

		if (sum < lowestDistance)
		{
			lowestDistance = sum;
			rampPosition = x->getCenter();
		}
	}
	return rampPosition;
}