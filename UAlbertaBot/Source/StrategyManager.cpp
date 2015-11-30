#include "Common.h"
#include "StrategyManager.h"
#include "UnitUtil.h"
#include "ProductionManager.h"
//TOMMY
#include "CombatSimulation.h"

using namespace UAlbertaBot;

// constructor
StrategyManager::StrategyManager() 
	: _selfRace(BWAPI::Broodwar->self()->getRace())
	, _enemyRace(BWAPI::Broodwar->enemy()->getRace())
    , _emptyBuildOrder(BWAPI::Broodwar->self()->getRace())
{
	
}

// get an instance of this
StrategyManager & StrategyManager::Instance() 
{
	static StrategyManager instance;
	return instance;
}

const int StrategyManager::getScore(BWAPI::Player player) const
{
	return player->getBuildingScore() + player->getKillScore() + player->getRazingScore() + player->getUnitScore();
}

const BuildOrder & StrategyManager::getOpeningBookBuildOrder() const
{
	if (Config::Strategy::StrategyName.empty()){
		if (_enemyRace == BWAPI::Races::Protoss){
			Config::Strategy::StrategyName = Config::Strategy::AgainstProtossStrategyName;
		} 
		else if (_enemyRace == BWAPI::Races::Terran){
			Config::Strategy::StrategyName = Config::Strategy::AgainstTerrenStrategyName;
		}
		else if (_enemyRace == BWAPI::Races::Zerg){
			Config::Strategy::StrategyName = Config::Strategy::AgainstZergStrategyName;
		}
		else {
			Config::Strategy::StrategyName = Config::Strategy::AgainstZergStrategyName;
		}
	}

	if (Config::Strategy::StrategyName == Config::Strategy::AgainstZergStrategyName){
			Config::Micro::UseSparcraftSimulation = false;
			Config::Micro::WorkersDefendRush = true;
			Config::Macro::WorkersPerRefinery = 2;
	}

    auto buildOrderIt = _strategies.find(Config::Strategy::StrategyName);

    // look for the build order in the build order map
	if (buildOrderIt != std::end(_strategies))
    {
        return (*buildOrderIt).second._buildOrder;
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Strategy not found: %s, returning empty initial build order", Config::Strategy::StrategyName.c_str());
        return _emptyBuildOrder;
    }
}

const BuildOrder & StrategyManager::getAdaptiveBuildOrder() const
{
	if (Config::Strategy::StrategyName == Config::Strategy::AgainstProtossStrategyName)
	{
		if (InformationManager::Instance().isEnemyExpand() && BWAPI::Broodwar->self()->supplyUsed() < 80)
		{
			BWAPI::Broodwar->printf("USING PROTOSS EP BUILD");
			return _strategies.find(Config::Strategy::AgainstProtossStrategyName + "_ep")->second._buildOrder;
		}
		else
		{
			BWAPI::Broodwar->printf("USING PROTOSS NE BUILD");
			return _strategies.find(Config::Strategy::AgainstProtossStrategyName + "_ne")->second._buildOrder;
		}
	}

	if (Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName && BWAPI::Broodwar->self()->supplyUsed() < 80)
	{
		if (InformationManager::Instance().isEnemyExpand())
		{
			BWAPI::Broodwar->printf("USING TERRAN EP BUILD");
			return _strategies.find(Config::Strategy::AgainstTerrenStrategyName + "_ep")->second._buildOrder;
		}
		else
		{
			BWAPI::Broodwar->printf("USING TERRAN NE BUILD");
			return _strategies.find(Config::Strategy::AgainstTerrenStrategyName + "_ne")->second._buildOrder;
		}
	}

	BWAPI::Broodwar->printf("Could not find adaptive build order!!! QQ");

	return _emptyBuildOrder;
}

const bool StrategyManager::isSpireBuilding() const
{
	for (auto x : BWAPI::Broodwar->self()->getUnits())
	{
		if (x->getType() == BWAPI::UnitTypes::Zerg_Spire && x->getHitPoints() < 600)
		{
			return true;
		}
	}
	return false;
}
const bool StrategyManager::shouldExpandNow() const
{

	// if there is no place to expand to, we can't expand
	if (MapTools::Instance().getNextExpansion() == BWAPI::TilePositions::None)
	{
		BWAPI::Broodwar->printf("No valid expansion location");
		return false;
	}

	size_t numDepots = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Command_Center)
		+ UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Nexus)
		+ UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hatchery)
		+ UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Lair)
		+ UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hive);
	int frame = BWAPI::Broodwar->getFrameCount();
	int minute = frame / (24 * 60);

	// if we have a ridiculous stockpile of minerals, expand
	if (BWAPI::Broodwar->self()->minerals() > 450)
	{
		//BuildingManager::Instance().shouldIExpand = true;
		return true;
	}

	if (InformationManager::Instance().isEnemyExpand()) //&& Config::Strategy::StrategyName == Config::Strategy::AgainstTerrenStrategyName)
	{
		return true;
	}
	// if we have a ton of idle workers then we need a new expansion
	if (WorkerManager::Instance().getNumIdleWorkers() > 6)
	{
		//BuildingManager::Instance().shouldIExpand = true;
		return true;
	}

	/*
	// we will make expansion N after array[N] minutes have passed
	std::vector<int> expansionTimes = { 5, 10, 20, 30, 40, 50 };

	for (size_t i(0); i < expansionTimes.size(); ++i)
	{
		if (numDepots < (i + 2) && minute > expansionTimes[i])
		{
			//BuildingManager::Instance().shouldIExpand = true;
			return true;
		}
	}*/

	return false;
}
void StrategyManager::addStrategy(const std::string & name, Strategy & strategy)
{
    _strategies[name] = strategy;
}

const MetaPairVector StrategyManager::getBuildOrderGoal()
{
    BWAPI::Race myRace = BWAPI::Broodwar->self()->getRace();

    if (myRace == BWAPI::Races::Protoss)
    {
        return getProtossBuildOrderGoal();
    }
    else if (myRace == BWAPI::Races::Terran)
	{
		return getTerranBuildOrderGoal();
	}
    else if (myRace == BWAPI::Races::Zerg)
	{
		return getZergBuildOrderGoal();
	}

    return MetaPairVector();
}

const MetaPairVector StrategyManager::getProtossBuildOrderGoal() const
{
	// the goal to return
	MetaPairVector goal;

	int numZealots          = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Zealot);
    int numPylons           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Pylon);
	int numDragoons         = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Dragoon);
	int numProbes           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Probe);
	int numNexusCompleted   = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numNexusAll         = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Nexus);
	int numCyber            = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Cybernetics_Core);
	int numCannon           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Photon_Cannon);
    int numScout            = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Corsair);
    int numReaver           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Reaver);
    int numDarkTeplar       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar);

    if (Config::Strategy::StrategyName == "Protoss_ZealotRush")
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 8));

        // once we have a 2nd nexus start making dragoons
        if (numNexusAll >= 2)
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 4));
        }
    }
    else if (Config::Strategy::StrategyName == "Protoss_DragoonRush")
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 6));
    }
    else if (Config::Strategy::StrategyName == "Protoss_Drop")
    {
        if (numZealots == 0)
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 4));
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Shuttle, 1));
        }
        else
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Zealot, numZealots + 8));
        }
    }
    else if (Config::Strategy::StrategyName == "Protoss_DTRush")
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dark_Templar, numDarkTeplar + 2));

        // if we have a 2nd nexus then get some goons out
        if (numNexusAll >= 2)
        {
            goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Dragoon, numDragoons + 4));
        }
    }
    else
    {
        UAB_ASSERT_WARNING(false, "Unknown Protoss Strategy Name: %s", Config::Strategy::StrategyName.c_str());
    }

    // if we have 3 nexus, make an observer
    if (numNexusCompleted >= 3)
    {
        goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
    }
    
    // add observer to the goal if the enemy has cloaked units
	if (InformationManager::Instance().enemyHasCloakedUnits())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1));
		
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observatory, 1));
		}
		if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Observatory) > 0)
		{
			goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Observer, 1));
		}
	}

    // if we want to expand, insert a nexus into the build order
	if (shouldExpandNow())
	{
		goal.push_back(MetaPair(BWAPI::UnitTypes::Protoss_Nexus, numNexusAll + 1));
	}

	return goal;
}

const MetaPairVector StrategyManager::getTerranBuildOrderGoal() const
{
	// the goal to return
	std::vector<MetaPair> goal;

    int numWorkers      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_SCV);
    int numCC           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Command_Center);            
    int numMarines      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Marine);
	int numMedics       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Medic);
	int numWraith       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Wraith);
    int numVultures     = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Vulture);
    int numGoliath      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Goliath);
    int numTanks        = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
    int numBay          = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Terran_Engineering_Bay);

    if (Config::Strategy::StrategyName == "Terran_MarineRush")
    {
	    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine, numMarines + 8));

        if (numMarines > 5)
        {
            goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Engineering_Bay, 1));
        }
    }
    else if (Config::Strategy::StrategyName == "Terran_4RaxMarines")
    {
	    goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Marine, numMarines + 8));
    }
    else if (Config::Strategy::StrategyName == "Terran_VultureRush")
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Vulture, numVultures + 8));

        if (numVultures > 8)
        {
            goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Tank_Siege_Mode, 1));
            goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 4));
        }
    }
    else if (Config::Strategy::StrategyName == "Terran_TankPush")
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, 6));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Goliath, numGoliath + 6));
        goal.push_back(std::pair<MetaType, int>(BWAPI::TechTypes::Tank_Siege_Mode, 1));
    }
    else
    {
        BWAPI::Broodwar->printf("Warning: No build order goal for Terran Strategy: %s", Config::Strategy::StrategyName.c_str());
    }



    if (shouldExpandNow())
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_Command_Center, numCC + 1));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Terran_SCV, numWorkers + 10));
    }

	return goal;
}

const MetaPairVector StrategyManager::getZergBuildOrderGoal() const
{
	// the goal to return
	std::vector<MetaPair> goal;
	
    int numWorkers      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Drone);
    int numCC           = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hatchery)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Lair)
                        + UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hive);
	int numMutas        = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Mutalisk);
    int numDrones       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Drone);
    int zerglings       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Zergling);
	int numHydras       = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hydralisk);
    int numScourge      = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Scourge);
    int numGuardians    = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Guardian);
	int numExtract		= UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Extractor);

	//int mutasWanted = numMutas + 20;
	//int hydrasWanted = numHydras + 6;
	int numBases = UnitUtil::GetAllUnitCount(BWAPI::UnitTypes::Zerg_Hatchery);

	while (numExtract < numBases)
	{
		numExtract++;
	}

	goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Extractor, numExtract));

	int mineralPatches = 0;
	for (auto & depot : WorkerManager::Instance().getWorkerData().getDepots())
	{
		mineralPatches += WorkerManager::Instance().getWorkerData().getMineralsNearDepot(depot);
	}
	if (numWorkers < mineralPatches)
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, mineralPatches));
	}

	if (shouldExpandNow())
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, numCC + 1));
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Extractor, numExtract + 1));
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numWorkers + 14));
	}

    if (Config::Strategy::StrategyName == "Zerg_9Pool")
    {
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, zerglings + 6));
    }
    else if (Config::Strategy::StrategyName == "Zerg_2HatchHydra")
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, numHydras + 8));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Grooved_Spines, 1));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numDrones + 4));
    }
    else if (Config::Strategy::StrategyName == "Zerg_3HatchMuta")
    {
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, numHydras + 9));
        goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numDrones + 3));
    }
	else if (Config::Strategy::StrategyName == "Zerg_3HatchScourge")
	{
		if (numScourge > 40)
		{
			goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, numHydras + 12));
		}
		else
		{
			goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Scourge, numScourge + 12));
		}


		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numDrones + 4));
	}
	else if (Config::Strategy::StrategyName == "Zerg_9/10Hatch"){
		//goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Grooved_Spines, 1));
		//goal.push_back(std::pair<MetaType, int>(BWAPI::UpgradeTypes::Muscular_Augments, 1));
		//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hydralisk, numHydras + 9));
		//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numDrones + 3));
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Mutalisk, numMutas + 8));
		//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, zerglings + 1));
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numDrones + 1));
	}

	else if (Config::Strategy::StrategyName == "Zerg_3HatchHydra")
	{
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Mutalisk, numMutas + 7));
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Zergling, zerglings + 5));
		goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numDrones + 3));
	}

	//TOMMY
	if (shouldMakeMacroHatchery())
	{
		int hatchNeeded = BWAPI::Broodwar->self()->minerals() / 600;
		while (hatchNeeded > 0)
		{
			goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Hatchery, numCC + 1));
			//goal.push_back(std::pair<MetaType, int>(BWAPI::UnitTypes::Zerg_Drone, numWorkers + 1));
			macroHatchCount++;
			hatchNeeded--;
		}
	}
	return goal;
}

void StrategyManager::readResults()
{
    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    std::string enemyName = BWAPI::Broodwar->enemy()->getName();
    std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

    std::string enemyResultsFile = Config::Strategy::ReadDir + enemyName + ".txt";
    
    std::string strategyName;
    int wins = 0;
    int losses = 0;

    FILE *file = fopen ( enemyResultsFile.c_str(), "r" );
    if ( file != nullptr )
    {
        char line [ 4096 ]; /* or other suitable maximum line size */
        while ( fgets ( line, sizeof line, file ) != nullptr ) /* read a line */
        {
            std::stringstream ss(line);

            ss >> strategyName;
            ss >> wins;
            ss >> losses;

            //BWAPI::Broodwar->printf("Results Found: %s %d %d", strategyName.c_str(), wins, losses);

            if (_strategies.find(strategyName) == _strategies.end())
            {
                //BWAPI::Broodwar->printf("Warning: Results file has unknown Strategy: %s", strategyName.c_str());
            }
            else
            {
                _strategies[strategyName]._wins = wins;
                _strategies[strategyName]._losses = losses;
            }
        }

        fclose ( file );
    }
    else
    {
        //BWAPI::Broodwar->printf("No results file found: %s", enemyResultsFile.c_str());
    }
}

void StrategyManager::writeResults()
{
    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    std::string enemyName = BWAPI::Broodwar->enemy()->getName();
    std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

    std::string enemyResultsFile = Config::Strategy::WriteDir + enemyName + ".txt";

    std::stringstream ss;

    for (auto & kv : _strategies)
    {
        const Strategy & strategy = kv.second;

        ss << strategy._name << " " << strategy._wins << " " << strategy._losses << "\n";
    }

    Logger::LogOverwriteToFile(enemyResultsFile, ss.str());
}

void StrategyManager::onEnd(const bool isWinner)
{
    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    if (isWinner)
    {
        _strategies[Config::Strategy::StrategyName]._wins++;
    }
    else
    {
        _strategies[Config::Strategy::StrategyName]._losses++;
    }

    writeResults();
}

void StrategyManager::setLearnedStrategy()
{
    // we are currently not using this functionality for the competition so turn it off 
    return;

    if (!Config::Modules::UsingStrategyIO)
    {
        return;
    }

    const std::string & strategyName = Config::Strategy::StrategyName;
    Strategy & currentStrategy = _strategies[strategyName];

    int totalGamesPlayed = 0;
    int strategyGamesPlayed = currentStrategy._wins + currentStrategy._losses;
    double winRate = strategyGamesPlayed > 0 ? currentStrategy._wins / static_cast<double>(strategyGamesPlayed) : 0;

    // if we are using an enemy specific strategy
    if (Config::Strategy::FoundEnemySpecificStrategy)
    {        
        return;
    }

    // if our win rate with the current strategy is super high don't explore at all
    // also we're pretty confident in our base strategies so don't change if insufficient games have been played
    if (strategyGamesPlayed < 5 || (strategyGamesPlayed > 0 && winRate > 0.49))
    {
        BWAPI::Broodwar->printf("Still using default strategy");
        return;
    }

    // get the total number of games played so far with this race
    for (auto & kv : _strategies)
    {
        Strategy & strategy = kv.second;
        if (strategy._race == BWAPI::Broodwar->self()->getRace())
        {
            totalGamesPlayed += strategy._wins + strategy._losses;
        }
    }

    // calculate the UCB value and store the highest
    double C = 0.5;
    std::string bestUCBStrategy;
    double bestUCBStrategyVal = std::numeric_limits<double>::lowest();
    for (auto & kv : _strategies)
    {
        Strategy & strategy = kv.second;
        if (strategy._race != BWAPI::Broodwar->self()->getRace())
        {
            continue;
        }

        int sGamesPlayed = strategy._wins + strategy._losses;
        double sWinRate = sGamesPlayed > 0 ? currentStrategy._wins / static_cast<double>(strategyGamesPlayed) : 0;
        double ucbVal = C * sqrt( log( (double)totalGamesPlayed / sGamesPlayed ) );
        double val = sWinRate + ucbVal;

        if (val > bestUCBStrategyVal)
        {
            bestUCBStrategy = strategy._name;
            bestUCBStrategyVal = val;
        }
    }

    Config::Strategy::StrategyName = bestUCBStrategy;
}

//PAST THIS POINT IS TOMMY
//note: modify this function by changing order of items, max batch of items, and different items in response to different enemy units
std::map<BWAPI::UnitType, int> StrategyManager::shouldBuildSunkens() const
{
	std::map<BWAPI::UnitType, int> defenses;

	SparCraft::ScoreType score = 0;
	CombatSimulation sim;
	//sim.generateMap(); THIS IS NOT WORKING LOL

	sim.generateCurrentSituation();

	CombatSimulation test(sim);

	test.finishMoving();
	score = test.simulateCombat();

	if (score >= 0)
	{
		//BWAPI::Broodwar->printf("Don't need defenses");
		return defenses;
	}

	if (score < 0)
	{
		BWAPI::Broodwar->printf("Defenses required");
	}

	int newSunkens = 0;
	int newLings = 0;
	// if we lose, add one sunken and resimulate
	if (Config::Strategy::StrategyName == Config::Strategy::AgainstProtossStrategyName)
	{
		if ((BWAPI::Broodwar->getFrameCount() > BuildingManager::Instance().sunkenBuildTimer) && (score < 0))
		{
			sim.addAllySunken();
			newSunkens++;

			CombatSimulation test(sim);
			test.finishMoving();
			score = test.simulateCombat();
		}
		else if (score < 0)
		{
			BWAPI::Broodwar->printf("Only making lings; expansion not ready yet!");
		}
	}
	else if ((score < 0))
	{
		sim.addAllySunken();
		newSunkens++;

		CombatSimulation test(sim);
		test.finishMoving();
		score = test.simulateCombat();
	}
	// if we stil lose, keep adding zerglings until we hit 6 (larva cap)
	while ((score < 0) && (newLings < 6))
	{
		sim.addAllyZergling();
		newLings++;

		CombatSimulation test(sim);
		test.finishMoving();
		score = test.simulateCombat();
	}
	// if we still lose, keep adding sunkens until we win
	if (Config::Strategy::StrategyName == Config::Strategy::AgainstProtossStrategyName)
	{
		if (BWAPI::Broodwar->getFrameCount() > BuildingManager::Instance().sunkenBuildTimer)
		{
			while ((score < 0) && (newSunkens < 4))
			{
				sim.addAllySunken();
				newSunkens++;

				CombatSimulation test(sim);
				test.finishMoving();
				score = test.simulateCombat();
			}
		}
	}
	else
	{
		while ((score < 0) && (newSunkens < 4))
		{
			sim.addAllySunken();
			newSunkens++;

			CombatSimulation test(sim);
			test.finishMoving();
			score = test.simulateCombat();
		}
	}

	// return a list of units to be made. production manager should queue these at highest priority
	defenses[BWAPI::UnitTypes::Zerg_Sunken_Colony] = newSunkens;
	defenses[BWAPI::UnitTypes::Zerg_Zergling] = newLings;
	BWAPI::Broodwar->printf("Sunkens for defense: %d\n", newSunkens);
	BWAPI::Broodwar->printf("Zerglings for defense: %d\n", newLings);
	return defenses;
}

std::map<BWAPI::UnitType, int> StrategyManager::shouldBuildSunkens2() const
{
	std::map<BWAPI::UnitType, int> defenses;

	SparCraft::ScoreType score = 0;
	CombatSimulation sim;
	//sim.generateMap(); THIS IS NOT WORKING LOL

	sim.generateCurrentSituation2();

	CombatSimulation test(sim);

	test.finishMoving();
	score = test.simulateCombat();

	if (score >= 0)
	{
		BWAPI::Broodwar->printf("Don't need defenses");
		return defenses;
	}

	if (score < 0)
	{
		BWAPI::Broodwar->printf("Defenses required");
	}

	int newSunkens = 0;
	int newLings = 0;
	// if we lose, add one sunken and resimulate
	if ((score < 0) && (BuildingManager::Instance().canBuild))
	{
		sim.addAllySunken();
		newSunkens++;

		CombatSimulation test(sim);
		test.finishMoving();
		score = test.simulateCombat();
	}
	else if (!(BuildingManager::Instance().canBuild))
	{
		BWAPI::Broodwar->printf("Only making lings; expansion not ready!");
	}
	// if we stil lose, keep adding zerglings until we hit 6 (larva cap)
	while ((score < 0) && (newLings < 6) && (BuildingManager::Instance().canBuild))
	{
		sim.addAllyZergling();
		newLings++;

		CombatSimulation test(sim);
		test.finishMoving();
		score = test.simulateCombat();
	}
	// if we still lose, keep adding sunkens until we win
	while ((score < 0) && (newSunkens < 4))
	{
		sim.addAllySunken();
		newSunkens++;

		CombatSimulation test(sim);
		test.finishMoving();
		score = test.simulateCombat();
	}
	// possibly may need to cap the number of sunkens (3-6)

	// build 1 sunken at a time. actually, this code may be useless
	/*if (score < 0)
	{
	for (auto &unit : BWAPI::Broodwar->enemy()->getUnits())
	{
	if ( ( (unit->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony) && (unit->isBeingConstructed()) ) ||
	((unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony) && (unit->isMorphing())) )
	{
	BWAPI::Broodwar->printf("Already making a sunken");
	return false;
	}
	}
	}*/

	for (auto &unit : InformationManager::Instance().getEnemyProductionEstimate())
	{
		if (unit.first == BWAPI::UnitTypes::Terran_Barracks)
		{
			//BWAPI::Broodwar->printf("Estimated marines: %d\n", unit.second);
		}
		if (unit.first == BWAPI::UnitTypes::Protoss_Gateway)
		{
			//BWAPI::Broodwar->printf("Estimated zealots: %d\n", unit.second);
		}
	}

	// return a list of units to be made. production manager should queue these at highest priority
	defenses[BWAPI::UnitTypes::Zerg_Sunken_Colony] = newSunkens;
	defenses[BWAPI::UnitTypes::Zerg_Zergling] = newLings;
	BWAPI::Broodwar->printf("Sunkens for defense: %d\n", newSunkens);
	BWAPI::Broodwar->printf("Zerglings for defense: %d\n", newLings);
	return defenses;
}

//NEW
const bool StrategyManager::shouldMakeMacroHatchery() const
{
	if (BWAPI::Broodwar->self()->minerals() > 600)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int StrategyManager::getMacroHatchCount()
{
	return macroHatchCount;
}

void StrategyManager::removeMacroHatch()
{
	macroHatchCount--;
	BWAPI::Broodwar->printf("Minerals > 800; building macro hatchery");
}