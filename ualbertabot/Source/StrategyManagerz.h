#pragma once

#include "Common.h"
#include "BWTA.h"
#include "BuildOrderQueue.h"
#include "InformationManager.h"
#include "WorkerManager.h"
#include "BuildOrder.h"

namespace UAlbertaBot
{
typedef std::pair<MetaType, size_t> MetaPair;
typedef std::vector<MetaPair> MetaPairVector;

struct Strategy
{
    std::string _name;
    BWAPI::Race _race;
    int         _wins;
    int         _losses;
    BuildOrder  _buildOrder;

    Strategy()
        : _name("None")
        , _race(BWAPI::Races::None)
        , _wins(0)
        , _losses(0)
    {
    
    }

    Strategy(const std::string & name, const BWAPI::Race & race, const BuildOrder & buildOrder)
        : _name(name)
        , _race(race)
        , _wins(0)
        , _losses(0)
        , _buildOrder(buildOrder)
    {
    
    }
};

class StrategyManager 
{
	StrategyManager();

	BWAPI::Race					    _selfRace;
	BWAPI::Race					    _enemyRace;
    std::map<std::string, Strategy> _strategies; // contains strategies parsed from Config.txt. seems only to be used
												 // for getOpeningBookBuildOrder()
    int                             _totalGamesPlayed;
    const BuildOrder                _emptyBuildOrder;

	        void	                writeResults();
	const	int					    getScore(BWAPI::Player player) const;
	const	double				    getUCBValue(const size_t & strategy) const;
	const	bool				    shouldExpandNow() const; // could be modified to give more precise expansions
    const	MetaPairVector		    getProtossBuildOrderGoal() const;
	const	MetaPairVector		    getTerranBuildOrderGoal() const;
	const	MetaPairVector		    getZergBuildOrderGoal() const; // uses the strategy in Config.cpp to decide what units to make
																   // independent of Config.txt. this build order seems to be looped

	//NEW


public:
    
	static	StrategyManager &	    Instance();

			void				    onEnd(const bool isWinner);
            void                    addStrategy(const std::string & name, Strategy & strategy); // currently only used by the parser
            void                    setLearnedStrategy();
            void	                readResults();
	const	bool				    regroup(int numInRadius); // not implemented
	const	bool				    rushDetected(); // not implemented
	const	int				        defendWithWorkers(); // not implemented
	const	MetaPairVector		    getBuildOrderGoal(); // called by performBuildOrderSearch in ProdManager
														 // passes build order goals to BOSS through ProdManager
														 // called in an update loop
	const	BuildOrder &            getOpeningBookBuildOrder() const; // use strat in Config.cpp. only called once
																	  // theory: since ProdManager's _queue is not updated until
																	  // its current units are completed, openingBookBO builds all 
																	  // its items, then you use getBOGoal to continue production
	const	bool					shouldBuildSunkens() const;		// TO-DO: needs to account for enemy unit production
	const	bool					shouldMakeMacroHatchery() const;	// TO-DO: needs to replace shouldExpand() at >3000 minerals

};
}