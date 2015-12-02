#include "ProductionManager.h"
#include "UnitUtil.h"
#include "StrategyManager.h"

using namespace UAlbertaBot;

ProductionManager::ProductionManager()
	: _assignedWorkerForThisBuilding(false)
	, _haveLocationForThisBuilding(false)
	, _enemyCloakedDetected(false)
	, _isLastBuildOrder(false)
{
	setBuildOrder(StrategyManager::Instance().getOpeningBookBuildOrder());
}

void ProductionManager::setBuildOrder(const BuildOrder & buildOrder)
{
	_queue.clearAll();

	for (size_t i(0); i<buildOrder.size(); ++i)
	{
		_queue.queueAsLowestPriority(buildOrder[i], true);
	}
}

void ProductionManager::performBuildOrderSearch()
{
	if (!Config::Modules::UsingBuildOrderSearch || !canPlanBuildOrderNow())
	{
		return;
	}

	BuildOrder & buildOrder = BOSSManager::Instance().getBuildOrder();

	if (buildOrder.size() > 0)
	{
		setBuildOrder(buildOrder);
		BOSSManager::Instance().reset();
	}
	else
	{
		if (!BOSSManager::Instance().isSearchInProgress())
		{
			BOSSManager::Instance().startNewSearch(StrategyManager::Instance().getBuildOrderGoal());
		}
	}
}

bool ProductionManager::isLairCompleted()
{
	for (auto x : BWAPI::Broodwar->self()->getUnits())
	{
		if (x->getType() == BWAPI::UnitTypes::Zerg_Lair && x->isCompleted()){
			return true;
		}
	}
	return false;
}
void ProductionManager::update()
{
	/*
	if (muscBuild && BWAPI::Broodwar->getFrameCount() > muscBuildTimer)
	{
		muscBuild = false;
		_queue.queueAsHighestPriority(MetaType(BWAPI::UpgradeTypes::Grooved_Spines), true);
	}
	*/

	// check the _queue for stuff we can build
	manageBuildOrderQueue();

	// if nothing is currently building, get a new goal from the strategy manager
	//|| (BWAPI::Broodwar->self()->minerals() > 2350))
	if ((_queue.size() == 0) && (BWAPI::Broodwar->getFrameCount() > 10))
	{
		if (Config::Debug::DrawBuildOrderSearchInfo)
		{
			BWAPI::Broodwar->drawTextScreen(150, 10, "Nothing left to build, new search!");
		}
		if (_isLastBuildOrder || BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
		{
			performBuildOrderSearch();
		}
		else
		{
			setBuildOrder(StrategyManager::Instance().getAdaptiveBuildOrder());
			_isLastBuildOrder = true;
		}
	}

	// detect if there's a build order deadlock once per second
	if ((BWAPI::Broodwar->getFrameCount() % 24 == 0) && detectBuildOrderDeadlock())
	{
		if (Config::Debug::DrawBuildOrderSearchInfo)
		{
			BWAPI::Broodwar->printf("Supply deadlock detected, building supply!");
		}
		
		if (Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName && BWAPI::Broodwar->self()->supplyUsed() >= 40)
		{
			BWAPI::Broodwar->printf("Queuing one extra overlord");
			_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
		}
		else if (BWAPI::Broodwar->self()->supplyUsed() >= 50 && !StrategyManager::Instance().isSpireBuilding())
		{
			BWAPI::Broodwar->printf("Queuing one extra overlord");
			_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
		}

		if (Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName && BWAPI::Broodwar->self()->supplyUsed() >= 100)
		{
			BWAPI::Broodwar->printf("Queuing two extra overlord");
			_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
		}
		else if (BWAPI::Broodwar->self()->supplyUsed() >= 80 && !StrategyManager::Instance().isSpireBuilding())
		{
			BWAPI::Broodwar->printf("Queuing two extra overlord");
			_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
		}

		if (!(Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName))
		{
			if (BWAPI::Broodwar->self()->supplyUsed() >= 100 && !StrategyManager::Instance().isSpireBuilding())
			{
				BWAPI::Broodwar->printf("Queuing three extra overlord");
				_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
			}
			if (BWAPI::Broodwar->self()->supplyUsed() >= 125 && !StrategyManager::Instance().isSpireBuilding())
			{
				BWAPI::Broodwar->printf("Queuing four extra overlord");
				_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
			}
		}

		_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getSupplyProvider()), true);
	}


	if (!BuildingManager::Instance().shouldIExpand && BWAPI::Broodwar->getFrameCount() % 24 == 0 && StrategyManager::Instance().shouldExpandNow())
	{
		BuildingManager::Instance().shouldIExpand = true;
	}

	else if (BuildingManager::Instance().shouldIExpand && BWAPI::Broodwar->getFrameCount() % 24 == 0 && !StrategyManager::Instance().shouldExpandNow())
	{
		BuildingManager::Instance().shouldIExpand = false;
	}

	if (Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName && BWAPI::Broodwar->getFrameCount() > hatchCounter && BWAPI::Broodwar->self()->minerals() > 600 && !StrategyManager::Instance().isSpireBuilding())
	{
		BWAPI::Broodwar->printf("Entering hatchery loop\n");
		int totalMinerals = BWAPI::Broodwar->self()->minerals();
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Hatchery), true);
		hatchCounter = BWAPI::Broodwar->getFrameCount() + 360;
	}
	if (Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName && BWAPI::Broodwar->getFrameCount() > mutaCounter && BWAPI::Broodwar->self()->minerals() >= 900 && !StrategyManager::Instance().isSpireBuilding() && UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Spire) >= 1)
	{
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
		_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);

		mutaCounter = BWAPI::Broodwar->getFrameCount() + BWAPI::UnitTypes::Zerg_Mutalisk.buildTime();
	}

	//TOMMY
	if ((BWAPI::Broodwar->getFrameCount() % 24 == 0) && (checkDefenses()))
	{
		std::map<BWAPI::UnitType, int> defenses;
		// edit this and see which works better: shouldBuildSunkens or shouldBuildSunkens2
		if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
		{
			//BWAPI::Broodwar->printf("checking shouldBuildSunkens");
			defenses = StrategyManager::Instance().shouldBuildSunkens3();
		}
		if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
		{
			//BWAPI::Broodwar->printf("checking shouldBuildSunkens");
			defenses = StrategyManager::Instance().shouldBuildSunkens3();
		}
		if (!defenses.empty())
		{
			if (Config::Debug::DrawBuildOrderSearchInfo)
			{
				//BWAPI::Broodwar->printf("Need defenses");
			}
			for (int i = 0; i < defenses[BWAPI::UnitTypes::Zerg_Sunken_Colony]; i++)
			{
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
				// EVERY TIME WE MAKE A SUNKEN, SHOULD WE MAKE A DRONE TO REPLACE IT???? and at what priority? and block or no?
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
			}
			for (int j = 0; j < defenses[BWAPI::UnitTypes::Zerg_Zergling]; j++)
			{
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Zerg_Zergling), true);
			}
		}
	}
	else
	{
		if ((BWAPI::Broodwar->getFrameCount() % 24 == 0) && (!BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg))
		{
			//BWAPI::Broodwar->printf("enemy race not determined yet");
		}
	}

	// if they have cloaked units get a new goal asap
	if (!_enemyCloakedDetected && InformationManager::Instance().enemyHasCloakedUnits())
	{
		if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
		{
			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon) < 2)
			{
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon), true);
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Photon_Cannon), true);
			}

			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Forge) == 0)
			{
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Protoss_Forge), true);
			}
		}
		else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
		{
			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Missile_Turret) < 2)
			{
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Terran_Missile_Turret), true);
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Terran_Missile_Turret), true);
			}

			if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Terran_Engineering_Bay) == 0)
			{
				_queue.queueAsHighestPriority(MetaType(BWAPI::UnitTypes::Terran_Engineering_Bay), true);
			}
		}

		if (Config::Debug::DrawBuildOrderSearchInfo)
		{
			BWAPI::Broodwar->printf("Enemy Cloaked Unit Detected!");
		}

		_enemyCloakedDetected = true;
	}

	// play gas trick for 9/10Hatch strategy
	if (Config::Strategy::StrategyName == Config::Strategy::AgainstProtossStrategyName)
	{
		if (BWAPI::Broodwar->getFrameCount() == _gasTrickTimer) ProductionManager::Instance().performCommand(BWAPI::UnitCommandTypes::Cancel_Construction);
		if (!_gasTrickTimer && UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Drone) == 9 && UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Extractor) == 1){
			_gasTrickTimer = BWAPI::Broodwar->getFrameCount() + 1;
		}
	}
}

// on unit destroy
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	// we don't care if it's not our unit
	if (!unit || unit->getPlayer() != BWAPI::Broodwar->self())
	{
		return;
	}

	if (Config::Modules::UsingBuildOrderSearch)
	{
		// if it's a worker or a building, we need to re-search for the current goal
		if ((unit->getType().isWorker() && !WorkerManager::Instance().isWorkerScout(unit)) || unit->getType().isBuilding())
		{
			if (unit->getType() != BWAPI::UnitTypes::Zerg_Drone && unit->getType() != BWAPI::UnitTypes::Zerg_Sunken_Colony
				&& unit->getType() != BWAPI::UnitTypes::Zerg_Zergling)
			{
				//performBuildOrderSearch();
				//_queue.queueAsLowestPriority(MetaType(unit->getType()), true);
			}
			if ((unit->getType() == BWAPI::UnitTypes::Zerg_Spire) || (unit->getType() == BWAPI::UnitTypes::Zerg_Hydralisk_Den)
				|| (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool))
			{
				_queue.queueAsHighestPriority(MetaType(unit->getType()), true);
			}
		}
	}
}

void ProductionManager::manageBuildOrderQueue()
	{
		// if there is nothing in the _queue, oh well
		if (_queue.isEmpty())
		{
			return;
		}

		// the current item to be used
		BuildOrderItem & currentItem = _queue.getHighestPriorityItem();


		// while there is still something left in the _queue
		while (!_queue.isEmpty())
		{


			BWAPI::Unit producer;


			/*
			if (currentItem.metaType.whatBuilds().isBuilding() && !canProduce(currentItem.metaType.whatBuilds()))
			{
			_queue.queueAsHighestPriority(currentItem.metaType.whatBuilds(), false);

			}
			*/
			producer = getProducer(currentItem.metaType);
			/*
			if (currentItem.metaType.isUpgrade())
			{
				if (currentItem.metaType.getUpgradeType() == BWAPI::UpgradeTypes::Zerg_Carapace)
				{
					BWAPI::Broodwar->printf("First evo chamber is %d, chosen Evo chamber is %d\n", BuildingManager::Instance().firstEvoChamber, producer->getID());
				}
			}
			*/
			

			// check to see if we can make it right now
			bool canMake = canMakeNow(producer, currentItem.metaType);

			// if we try to build too many refineries manually remove it
			if (currentItem.metaType.isRefinery() && (BWAPI::Broodwar->self()->allUnitCount(BWAPI::Broodwar->self()->getRace().getRefinery() >= 3)))
			{
				_queue.removeCurrentHighestPriorityItem();
				break;
			}

			// if the next item in the list is a building and we can't yet make it
			if (currentItem.metaType.isBuilding() && !(producer && canMake) && currentItem.metaType.whatBuilds().isWorker())
			{
				// construct a temporary building object
				// construct a temporary building object
				if (currentItem.metaType.getUnitType() == BWAPI::UnitTypes::Zerg_Creep_Colony)
				{
					Building b(currentItem.metaType.getUnitType(), BuildingManager::Instance().createdHatcheriesVector[0]);
					b.isGasSteal = currentItem.isGasSteal;
					b.isMacro = false;
					// set the producer as the closest worker, but do not set its job yet
					producer = WorkerManager::Instance().getBuilder(b, false);

					// predict the worker movement to that building location
					predictWorkerMovement(b);
				}
				else
				{


					Building b(currentItem.metaType.getUnitType(), BWAPI::Broodwar->self()->getStartLocation());
					b.isGasSteal = currentItem.isGasSteal;

					//TOMMY
					if ((StrategyManager::Instance().getMacroHatchCount() > 0) && (currentItem.metaType.getUnitType() == BWAPI::UnitTypes::Zerg_Hatchery))
					{
						b.isMacro = true;
						StrategyManager::Instance().removeMacroHatch();
					}

					// set the producer as the closest worker, but do not set its job yet
					producer = WorkerManager::Instance().getBuilder(b, false);

					// predict the worker movement to that building location
					predictWorkerMovement(b);
				}

			}
		// if we can make the current item
		if (producer && canMake)
		{
			// create it

			//TOMMY
			if ((StrategyManager::Instance().getMacroHatchCount() > 0) &&
				(currentItem.metaType.getUnitType() == BWAPI::UnitTypes::Zerg_Hatchery))
			{
				currentItem.isMacro = true;
				StrategyManager::Instance().removeMacroHatch();
			}

			create(producer, currentItem);
			_assignedWorkerForThisBuilding = false;
			_haveLocationForThisBuilding = false;

			// and remove it from the _queue
			_queue.removeCurrentHighestPriorityItem();

			// don't actually loop around in here
			break;
		}
		// otherwise, if we can skip the current item
		else if (_queue.canSkipItem())
		{
			// skip it
			_queue.skipItem();

			// and get the next one
			currentItem = _queue.getNextHighestPriorityItem();
		}
		else
		{
			// so break out
			break;
		}
	}
}

bool ProductionManager::canProduce(BWAPI::UnitType producerType)
{
	for (auto x : BWAPI::Broodwar->self()->getUnits())
	{
		if (x->getType() == producerType && !x->isUpgrading())
		{
			return true;
		}
	}
	return false;

}

BWAPI::Unit ProductionManager::getProducer(MetaType t, BWAPI::Position closestTo)
{
	// get the type of unit that builds this
	BWAPI::UnitType producerType = t.whatBuilds();

	// make a set of all candidate producers
	BWAPI::Unitset candidateProducers;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		UAB_ASSERT(unit != nullptr, "Unit was null");

		// reasons a unit can not train the desired type
		if (unit->isUpgrading())                    { continue; }
		if (unit->getType() != producerType)                    { continue; }
		if (!unit->isCompleted())                               { continue; }
		if (unit->isTraining())                                 { continue; }
		if (unit->isLifted())                                   { continue; }
		if (!unit->isPowered())                                 { continue; }

		// if the type is an addon, some special cases
		if (t.getUnitType().isAddon())
		{
			// if the unit already has an addon, it can't make one
			if (unit->getAddon() != nullptr)
			{
				continue;
			}

			// if we just told this unit to build an addon, then it will not be building another one
			// this deals with the frame-delay of telling a unit to build an addon and it actually starting to build
			if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build_Addon
				&& (BWAPI::Broodwar->getFrameCount() - unit->getLastCommandFrame() < 10))
			{
				continue;
			}

			bool isBlocked = false;

			// if the unit doesn't have space to build an addon, it can't make one
			BWAPI::TilePosition addonPosition(unit->getTilePosition().x + unit->getType().tileWidth(), unit->getTilePosition().y + unit->getType().tileHeight() - t.getUnitType().tileHeight());
			BWAPI::Broodwar->drawBoxMap(addonPosition.x * 32, addonPosition.y * 32, addonPosition.x * 32 + 64, addonPosition.y * 32 + 64, BWAPI::Colors::Red);

			for (int i = 0; i<unit->getType().tileWidth() + t.getUnitType().tileWidth(); ++i)
			{
				for (int j = 0; j<unit->getType().tileHeight(); ++j)
				{
					BWAPI::TilePosition tilePos(unit->getTilePosition().x + i, unit->getTilePosition().y + j);

					// if the map won't let you build here, we can't build it
					if (!BWAPI::Broodwar->isBuildable(tilePos))
					{
						isBlocked = true;
						BWAPI::Broodwar->drawBoxMap(tilePos.x * 32, tilePos.y * 32, tilePos.x * 32 + 32, tilePos.y * 32 + 32, BWAPI::Colors::Red);
					}

					// if there are any units on the addon tile, we can't build it
					BWAPI::Unitset uot = BWAPI::Broodwar->getUnitsOnTile(tilePos.x, tilePos.y);
					if (uot.size() > 0 && !(uot.size() == 1 && *(uot.begin()) == unit))
					{
						isBlocked = true;;
						BWAPI::Broodwar->drawBoxMap(tilePos.x * 32, tilePos.y * 32, tilePos.x * 32 + 32, tilePos.y * 32 + 32, BWAPI::Colors::Red);
					}
				}
			}

			if (isBlocked)
			{
				continue;
			}
		}

		// if the type requires an addon and the producer doesn't have one
		typedef std::pair<BWAPI::UnitType, int> ReqPair;
		for (const ReqPair & pair : t.getUnitType().requiredUnits())
		{
			BWAPI::UnitType requiredType = pair.first;
			if (requiredType.isAddon())
			{
				if (!unit->getAddon() || (unit->getAddon()->getType() != requiredType))
				{
					continue;
				}
			}
		}

		// if we haven't cut it, add it to the set of candidates
		if (BuildingManager::Instance().hatchSet.find(unit) != BuildingManager::Instance().hatchSet.end())
		{
			continue;
		}
		candidateProducers.insert(unit);
	}

	return getClosestUnitToPosition(candidateProducers, closestTo);
}

BWAPI::Unit ProductionManager::getClosestUnitToPosition(const BWAPI::Unitset & units, BWAPI::Position closestTo)
{
	if (units.size() == 0)
	{
		return nullptr;
	}

	// if we don't care where the unit is return the first one we have
	if (closestTo == BWAPI::Positions::None)
	{
		return *(units.begin());
	}

	BWAPI::Unit closestUnit = nullptr;
	double minDist(1000000);

	for (auto & unit : units)
	{
		UAB_ASSERT(unit != nullptr, "Unit was null");

		double distance = unit->getDistance(closestTo);
		if (!closestUnit || distance < minDist)
		{
			closestUnit = unit;
			minDist = distance;
		}
	}

	return closestUnit;
}

// this function will check to see if all preconditions are met and then create a unit
void ProductionManager::create(BWAPI::Unit producer, BuildOrderItem & item)
{
	if (!producer)
	{
		return;
	}

	MetaType t = item.metaType;

	// if we're dealing with a building
	if (t.isUnit() && t.getUnitType().isBuilding()
		&& t.getUnitType() != BWAPI::UnitTypes::Zerg_Lair
		&& t.getUnitType() != BWAPI::UnitTypes::Zerg_Hive
		&& t.getUnitType() != BWAPI::UnitTypes::Zerg_Greater_Spire
		&& t.getUnitType() != BWAPI::UnitTypes::Zerg_Sunken_Colony
		&& !t.getUnitType().isAddon())
	{
		// send the building task to the building manager
		//TOMMY
		BuildingManager::Instance().addBuildingTask(t.getUnitType(), BWAPI::Broodwar->self()->getStartLocation(), item.isGasSteal, item.isMacro);
	}
	else if (t.getUnitType().isAddon())
	{
		//BWAPI::TilePosition addonPosition(producer->getTilePosition().x + producer->getType().tileWidth(), producer->getTilePosition().y + producer->getType().tileHeight() - t.unitType.tileHeight());
		producer->buildAddon(t.getUnitType());
	}
	// if we're dealing with a non-building unit
	else if (t.isUnit())
	{
		// if the race is zerg, morph the unit
		if (t.getUnitType().getRace() == BWAPI::Races::Zerg)
		{
			producer->morph(t.getUnitType());
			if (t.getUnitType() == BWAPI::UnitTypes::Zerg_Overlord)
			{
				_overlordTimer = BWAPI::Broodwar->getFrameCount() + t.getUnitType().buildTime() + 35;
			}

			// if not, train the unit
		}
		else
		{
			producer->train(t.getUnitType());
		}
	}
	// if we're dealing with a tech research
	else if (t.isTech())
	{
		producer->research(t.getTechType());
	}
	else if (t.isUpgrade())
	{
		//Logger::Instance().log("Produce Upgrade: " + t.getName() + "\n");
		
		if (t.getUpgradeType() == BWAPI::UpgradeTypes::Muscular_Augments)
		{
			muscBuildTimer = BWAPI::Broodwar->getFrameCount() + t.getUpgradeType().upgradeTime();
			muscBuild = true;
		}
		producer->upgrade(t.getUpgradeType());
	}
	else
	{

	}
}

bool ProductionManager::canMakeNow(BWAPI::Unit producer, MetaType t)
{
	//UAB_ASSERT(producer != nullptr, "Producer was null");

	bool canMake = meetsReservedResources(t);
	if (canMake)
	{
		if (t.isUnit())
		{
			canMake = BWAPI::Broodwar->canMake(t.getUnitType(), producer);
		}
		else if (t.isTech())
		{
			canMake = BWAPI::Broodwar->canResearch(t.getTechType(), producer);
		}
		else if (t.isUpgrade())
		{
			canMake = BWAPI::Broodwar->canUpgrade(t.getUpgradeType(), producer);
		}
		else
		{
			UAB_ASSERT(false, "Unknown type");
		}
	}

	return canMake;
}

bool ProductionManager::detectBuildOrderDeadlock()
{
	// if the _queue is empty there is no deadlock
	if (_queue.size() == 0 || BWAPI::Broodwar->self()->supplyTotal() >= 390)
	{
		return false;
	}

	//TOMMY
	// hacky fix to make sure gas trick always works
	if ((BWAPI::Broodwar->getFrameCount() < 2000) && (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss || BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran))
	{
		return false;
	}

	// are any supply providers being built currently
	bool supplyInProgress = BuildingManager::Instance().isBeingBuilt(BWAPI::Broodwar->self()->getRace().getSupplyProvider());

	if (_overlordTimer)
	{
		if (BWAPI::Broodwar->getFrameCount() <= _overlordTimer) supplyInProgress = true;
	}

	// does the current item being built require more supply

	int supplyCost = _queue.getHighestPriorityItem().metaType.supplyRequired();
	int supplyAvailable = std::max(0, BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed());

	//TOMMY: try to avoid midgame deadlocks
	/*
	int totalSupply = 0;
	int totalProduced = 0;
	for (auto &item : _queue.getQueue())
	{
		if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg && item.metaType.getUnitType().isBuilding())
		{
			totalProduced++;
		}
		totalSupply += item.metaType.getUnitType().supplyRequired();
		totalProduced += item.metaType.getUnitType().supplyProvided();
	}
	if ((totalProduced < (totalSupply + 4)) && (BWAPI::Broodwar->self()->minerals() > 950))
	{
		BWAPI::Broodwar->printf("Staying way ahead of the supply game");
		return true;
	}
	if ((totalProduced < (totalSupply + 2)) && (BWAPI::Broodwar->self()->minerals() > 580))
	{
		BWAPI::Broodwar->printf("Staying a little ahead of the supply game");
		return true;
	}*/

	// if we don't have enough supply and none is being built, there's a deadlock
	if ((supplyAvailable < supplyCost) && !supplyInProgress)
	{
		// if we're zerg, check to see if a building is planned to be built
		if ((BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg) && 
			(BuildingManager::Instance().buildingsQueued().size() > 0) &&
			(BWAPI::Broodwar->getFrameCount() < 6000) &&
			((UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Spire) < 1) && (UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) < 1)))
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

// When the next item in the _queue is a building, this checks to see if we should move to it
// This function is here as it needs to access prodction manager's reserved resources info
void ProductionManager::predictWorkerMovement(const Building & b)
{
	if (b.isGasSteal)
	{
		return;
	}

	// get a possible building location for the building
	if (!_haveLocationForThisBuilding)
	{
		_predictedTilePosition = BuildingManager::Instance().getBuildingLocation(b);
	}

	if (_predictedTilePosition != BWAPI::TilePositions::None)
	{
		_haveLocationForThisBuilding = true;
	}
	else
	{
		return;
	}

	// draw a box where the building will be placed
	int x1 = _predictedTilePosition.x * 32;
	int x2 = x1 + (b.type.tileWidth()) * 32;
	int y1 = _predictedTilePosition.y * 32;
	int y2 = y1 + (b.type.tileHeight()) * 32;
	if (Config::Debug::DrawWorkerInfo)
	{
		BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Blue, false);
	}

	// where we want the worker to walk to
	BWAPI::Position walkToPosition = BWAPI::Position(x1 + (b.type.tileWidth() / 2) * 32, y1 + (b.type.tileHeight() / 2) * 32);

	// compute how many resources we need to construct this building
	int mineralsRequired = std::max(0, b.type.mineralPrice() - getFreeMinerals());
	int gasRequired = std::max(0, b.type.gasPrice() - getFreeGas());

	// get a candidate worker to move to this location
	BWAPI::Unit moveWorker = WorkerManager::Instance().getMoveWorker(walkToPosition);

	// Conditions under which to move the worker: 
	//		- there's a valid worker to move
	//		- we haven't yet assigned a worker to move to this location
	//		- the build position is valid
	//		- we will have the required resources by the time the worker gets there
	if (moveWorker && _haveLocationForThisBuilding && !_assignedWorkerForThisBuilding && (_predictedTilePosition != BWAPI::TilePositions::None) &&
		WorkerManager::Instance().willHaveResources(mineralsRequired, gasRequired, moveWorker->getDistance(walkToPosition)))
	{
		// we have assigned a worker
		_assignedWorkerForThisBuilding = true;

		// tell the worker manager to move this worker
		WorkerManager::Instance().setMoveWorker(mineralsRequired, gasRequired, walkToPosition);
	}
}

void ProductionManager::performCommand(BWAPI::UnitCommandType t)
{
	// if it is a cancel construction, it is probably the extractor trick
	if (t == BWAPI::UnitCommandTypes::Cancel_Construction)
	{
		BWAPI::Unit extractor = nullptr;
		for (auto & unit : BWAPI::Broodwar->self()->getUnits())
		{
			if (unit->getType() == BWAPI::UnitTypes::Zerg_Extractor)
			{
				extractor = unit;
			}
		}


		if (extractor)
		{
			BuildingManager::Instance().firstExtractorPosition = extractor->getTilePosition();
			extractor->cancelMorph();
			//BWAPI::Broodwar->printf("Freeing tiles\n");
			BuildingManager::Instance().removeBuildingExternal(extractor->getTilePosition());
			BuildingPlacer::Instance().freeTiles(extractor->getTilePosition(), 4, 2);
			BuildingManager::Instance().didGasTrickFrames = BWAPI::Broodwar->getFrameCount();
		}
	}
}

int ProductionManager::getFreeMinerals()
{
	return BWAPI::Broodwar->self()->minerals() - BuildingManager::Instance().getReservedMinerals();
}

int ProductionManager::getFreeGas()
{
	return BWAPI::Broodwar->self()->gas() - BuildingManager::Instance().getReservedGas();
}

// return whether or not we meet resources, including building reserves
bool ProductionManager::meetsReservedResources(MetaType type)
{
	// return whether or not we meet the resources
	return (type.mineralPrice() <= getFreeMinerals()) && (type.gasPrice() <= getFreeGas());
}


// selects a unit of a given type
BWAPI::Unit ProductionManager::selectUnitOfType(BWAPI::UnitType type, BWAPI::Position closestTo)
{
	// if we have none of the unit type, return nullptr right away
	if (BWAPI::Broodwar->self()->completedUnitCount(type) == 0)
	{
		return nullptr;
	}

	BWAPI::Unit unit = nullptr;

	// if we are concerned about the position of the unit, that takes priority
	if (closestTo != BWAPI::Positions::None)
	{
		double minDist(1000000);

		for (auto & u : BWAPI::Broodwar->self()->getUnits())
		{
			if (u->getType() == type)
			{
				double distance = u->getDistance(closestTo);
				if (!unit || distance < minDist) {
					unit = u;
					minDist = distance;
				}
			}
		}

		// if it is a building and we are worried about selecting the unit with the least
		// amount of training time remaining
	}
	else if (type.isBuilding())
	{
		for (auto & u : BWAPI::Broodwar->self()->getUnits())
		{
			UAB_ASSERT(u != nullptr, "Unit was null");

			if (u->getType() == type && u->isCompleted() && !u->isTraining() && !u->isLifted() && u->isPowered()) {

				return u;
			}
		}
		// otherwise just return the first unit we come across
	}
	else
	{
		for (auto & u : BWAPI::Broodwar->self()->getUnits())
		{
			UAB_ASSERT(u != nullptr, "Unit was null");

			if (u->getType() == type && u->isCompleted() && u->getHitPoints() > 0 && !u->isLifted() && u->isPowered())
			{
				return u;
			}
		}
	}

	// return what we've found so far
	return nullptr;
}

void ProductionManager::drawProductionInformation(int x, int y)
{
	if (!Config::Debug::DrawProductionInfo)
	{
		return;
	}

	// fill prod with each unit which is under construction
	std::vector<BWAPI::Unit> prod;
	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		UAB_ASSERT(unit != nullptr, "Unit was null");

		if (unit->isBeingConstructed())
		{
			prod.push_back(unit);
		}
	}

	// sort it based on the time it was started
	std::sort(prod.begin(), prod.end(), CompareWhenStarted());

	BWAPI::Broodwar->drawTextScreen(x - 30, y + 20, "\x04 TIME");
	BWAPI::Broodwar->drawTextScreen(x, y + 20, "\x04 UNIT NAME");

	size_t reps = prod.size() < 10 ? prod.size() : 10;

	y += 30;
	int yy = y;

	// for each unit in the _queue
	for (auto & unit : prod)
	{
		std::string prefix = "\x07";

		yy += 10;

		BWAPI::UnitType t = unit->getType();
		if (t == BWAPI::UnitTypes::Zerg_Egg)
		{
			t = unit->getBuildType();
		}

		BWAPI::Broodwar->drawTextScreen(x, yy, " %s%s", prefix.c_str(), t.getName().c_str());
		BWAPI::Broodwar->drawTextScreen(x - 35, yy, "%s%6d", prefix.c_str(), unit->getRemainingBuildTime());
	}

	_queue.drawQueueInformation(x, yy + 10);
}

ProductionManager & ProductionManager::Instance()
{
	static ProductionManager instance;
	return instance;
}

void ProductionManager::queueGasSteal()
{
	_queue.queueAsHighestPriority(MetaType(BWAPI::Broodwar->self()->getRace().getRefinery()), true, true);
}

// this will return true if any unit is on the first frame if it's training time remaining
// this can cause issues for the build order search system so don't plan a search on these frames
bool ProductionManager::canPlanBuildOrderNow() const
{
	for (const auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getRemainingTrainTime() == 0)
		{
			continue;
		}

		BWAPI::UnitType trainType = unit->getLastCommand().getUnitType();

		if (unit->getRemainingTrainTime() == trainType.buildTime())
		{
			return false;
		}
	}

	return true;
}

//TOMMY
bool ProductionManager::checkDefenses()
{
	if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
	{
		//BWAPI::Broodwar->printf("fighting Zerg; don't check defenses");
		return false;
	}

	//if more than 6 sunkens, stop checking
	/*
	if (UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony) >= 8)
	{
		return false;
	}
	*/

	//if more than X sunkens, stop checking
	if ((UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony) >= 6) && (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran))
	{
		return false;
	}
	if ((UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony) >= 7) && (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss))
	{
		return false;
	}

	if (Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName)
	{
		if (!(!BuildingManager::Instance().canBuild && BWAPI::Broodwar->getFrameCount() > BuildingManager::Instance().sunkenBuildTimer))
		{
			return false;
		}
	}


	std::deque< BuildOrderItem > items = _queue.getQueue();

	for (auto it = items.begin(); it != items.end(); it++)
	{
		if (it->metaType.getUnitType() == BWAPI::UnitTypes::Zerg_Creep_Colony)
		{
			return false;
		}
		
		if (it->metaType.getUnitType() == BWAPI::UnitTypes::Zerg_Zergling)
		{
			return false;
		}		
	}

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony)
		{
			return false;
		}

		if (unit->getType() == BWAPI::UnitTypes::Zerg_Larva)
		{
			if (unit->isMorphing())
			{
				if (unit->getBuildType() == BWAPI::UnitTypes::Zerg_Zergling)
				{
					BWAPI::Broodwar->printf("Larvae are morphing into zerglings");
					return false;
				}
			}
		}
	}

	if (!InformationManager::Instance().isEnemyMovedOut())
	{
		//BWAPI::Broodwar->printf("Enemy hasnt moved out; dont check defenses");
		return false;
	}

	if (UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Spire) >= 1)
	{
		return false;
	}
	if (UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den) >= 1)
	{
		return false;
	}

	for (auto x : BWAPI::Broodwar->self()->getUnits())
	{
		if (x->getType() == BWAPI::UnitTypes::Zerg_Spire && x->getHitPoints() == 600)
		{
			return false;
		}
	}

	return true;
}