#pragma once

#include "Common.h"
#include "MicroManager.h"
#include "InformationManager.h"

namespace UAlbertaBot
{
	class ScoutManager
	{
		BWAPI::Unit						_scouter1;
		BWAPI::Unit						_scouter2;
		BWAPI::Unit						_overlordScout;
		BWAPI::Unit						_workerScout;
		BWAPI::TilePosition				_ninjaBaseLocation;
		BWAPI::Position					_enemyRamp;

		void                            moveScouts();

		ScoutManager();

	public:

		static ScoutManager & Instance();

		void update();

		bool setScout(BWAPI::Unit unit);
		void setWorkerScout(BWAPI::Unit worker);
		void buildNinjaBase();
		BWAPI::TilePosition ScoutManager::getFarthestPoint();
		BWAPI::Position getEnemyRamp();

		void onSendText(std::string text);
		void onUnitShow(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitRenegade(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitMorph(BWAPI::Unit unit);
	};
}