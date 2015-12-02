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
    std::map<std::string, Strategy> _strategies;
    int                             _totalGamesPlayed;
    const BuildOrder                _emptyBuildOrder;
	bool							_isLastBuildOrder;

	        void	                writeResults();
	const	int					    getScore(BWAPI::Player player) const;
	const	double				    getUCBValue(const size_t & strategy) const;
	//const	bool				    shouldExpandNow() const;
    const	MetaPairVector		    getProtossBuildOrderGoal() const;
	const	MetaPairVector		    getTerranBuildOrderGoal() const;
	const	MetaPairVector		    getZergBuildOrderGoal() const;

	//TOMMY
	mutable int						macroHatchCount;
	const	bool					shouldMakeMacroHatchery() const;
	const	bool					shouldTotallyMakeMacroHatchery() const;
	mutable int						_mineralToBuildMacroHatchery;

public:
    
	static	StrategyManager &	    Instance();

			void				    onEnd(const bool isWinner);
            void                    addStrategy(const std::string & name, Strategy & strategy);
            void                    setLearnedStrategy();
            void	                readResults();
	const	bool				    regroup(int numInRadius);
	const	bool				    rushDetected();
	const	int				        defendWithWorkers();
	const	MetaPairVector		    getBuildOrderGoal();
	const	BuildOrder &            getOpeningBookBuildOrder() const;
	const   BuildOrder &			getAdaptiveBuildOrder() const;

	//TOMMY
	int								getMacroHatchCount();
	void							removeMacroHatch();
	// returns map corresponding to units that need to be made and how many
	// current control: in prod manager, if creep colonies or sunkens are currently in the queue, don't call this
	// when testing, try each version and see which works better
	std::map<BWAPI::UnitType, int>	shouldBuildSunkens() const;
	std::map<BWAPI::UnitType, int>	shouldBuildSunkens2() const;
	std::map<BWAPI::UnitType, int>  shouldBuildSunkens3() const;

	bool							timeToAttack = true;
	const  bool						shouldExpandNow() const;
	const  bool						isSpireBuilding() const;
};
}