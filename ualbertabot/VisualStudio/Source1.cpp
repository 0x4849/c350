// STEP 6: CHECK FOR COMPLETED BUILDINGS
void BuildingManager::checkForCompletedBuildings()
{
	std::vector<Building> toRemove;

	// for each of our buildings under construction
	for (auto & b : _buildings)
	{
		if (b.status != BuildingStatus::UnderConstruction)
		{
			continue;
		}

		// if the unit has completed
		if (b.buildingUnit->isCompleted())
		{
			if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
			{
				if (b.buildingUnit->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony)
				{
					//b.buildingUnit->upgrade(b.buildingUnit->getUpgrade());
					//b.buildingUnit->buildAddon(BWAPI::UnitTypes::Zerg_Sunken_Colony);

					// MAKE SUNKEN
					//b.buildingUnit->morph(BWAPI::UnitTypes::Zerg_Sunken_Colony);
					MetaType type(BWAPI::UnitTypes::Zerg_Sunken_Colony);
					ProductionManager::Instance()._queue.queueAsHighestPriority(type, true);

				}

				if (b.buildingUnit->getType() == BWAPI::UnitTypes::Zerg_Hatchery)
				{
					createdHatchery = true;

				}
			}
			if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Terran)
			{
				if (b.isGasSteal)
				{
					ScoutManager::Instance().setWorkerScout(b.builderUnit);
				}
				// otherwise tell the worker manager we're finished with this unit
				else
				{
					WorkerManager::Instance().finishedWithWorker(b.builderUnit);
				}
			}



			// remove this unit from the under construction vector
			toRemove.push_back(b);
			/*
			for (BWAPI::Unit p : BWAPI::Broodwar->self()->getUnits())
			{
			if (!madeFirstSunken && p->getType().isWorker() && p->getPosition() == firstHatcheryPosition)
			{
			//BWAPI::Position tempPosition;
			//tempPosition.x = b.finalPosition.x + 3;
			//tempPosition.y = b.finalPosition.y + 3;
			MetaType type(BWAPI::UnitTypes::Zerg_Creep_Colony);
			ProductionManager::Instance()._queue.queueAsHighestPriority(type, true);
			/*
			BWAPI::TilePosition sunkPos = getSunkenPosition();


			createdSunkenSet.insert(sunkPos);
			createdSunkenVector.push_back(sunkPos);
			createdBuilding.insert(sunkPos);
			p->build(BWAPI::UnitTypes::Zerg_Hatchery, sunkPos);

			madeFirstSunken = true;


			BWAPI::TilePosition sunkPos2 = getSunkenPosition();


			createdSunkenSet.insert(sunkPos2);
			createdSunkenVector.push_back(sunkPos2);
			createdBuilding.insert(sunkPos2);
			p->build(BWAPI::UnitTypes::Zerg_Hatchery, sunkPos2);
			//b.builderUnit->build(b.type, b.finalPosition);

			break;

			//candidateProducers.insert(p);
			}
			}
			}
			*/
		}
		else
		{

			if (b.type == 131 && b.buildingUnit->getHitPoints() >= 1050 && !sentFirstDroneForSunken)
			{
				//BWAPI::Broodwar->printf("Send Drone");
				//BWAPI::Unitset candidateProducers;

				for (BWAPI::Unit p : BWAPI::Broodwar->self()->getUnits())
				{
					if (p->getType().isWorker())
					{
						//BWAPI::Position tempPosition;
						//tempPosition.x = b.finalPosition.x + 3;
						//tempPosition.y = b.finalPosition.y + 3;
						p->move(BWAPI::Position(b.finalPosition));
						break;
						//candidateProducers.insert(p);
					}
				}
				sentFirstDroneForSunken = true;
			}
		}
		// if we are terran, give the worker back to worker manager

	}


}
	}

	removeBuildings(toRemove);
}