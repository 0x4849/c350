

#include "Common.h"
#include "BuildingManager.h"
#include "Micro.h"
#include "ScoutManager.h"
#include "ProductionManager.h"


using namespace UAlbertaBot;

BuildingManager::BuildingManager()
: _debugMode(false)
, _reservedMinerals(0)
, _reservedGas(0)
{

}

// Get a sunken position depending on whether or not we have an expansion.
BWAPI::TilePosition BuildingManager::getSunkenPosition()
{

	BWAPI::UnitType sunk = BWAPI::UnitTypes::Zerg_Creep_Colony;
	// Always make sunkens at natural expansion if you can.
	if (createdHatcheriesSet.size() >= 1)
	{
		BWAPI::TilePosition hatchPosition = createdHatcheriesVector[0];
		BWAPI::Unit pExpansion = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(hatchPosition), BWAPI::Filter::IsResourceDepot);
		BWAPI::Unitset myUnits = pExpansion->getUnitsInRadius(200);
		BWAPI::UnitType larva = BWAPI::UnitTypes::Zerg_Larva;
		BWAPI::UnitType egg = BWAPI::UnitTypes::Zerg_Egg;

		std::set<BWAPI::TilePosition> stuffBlocking;
		for (BWAPI::Unit p : myUnits)
		{
			if (p->getType() == larva || p->getType() == egg)
			{
				stuffBlocking.insert(p->getTilePosition());
			}
		}

		while (buildableSunkenTilePositions.size() >= 1)
		{
			std::set<BWAPI::TilePosition>::iterator it = buildableSunkenTilePositions.begin();


			BWAPI::TilePosition mySunkPosition = *it;
			Building z(sunk, mySunkPosition);

			if (BWAPI::Broodwar->hasCreep(mySunkPosition) && BuildingPlacer::Instance().canBuildHere(mySunkPosition, z) && createdBuilding.find(mySunkPosition) == createdBuilding.end() && stuffBlocking.find(mySunkPosition) == stuffBlocking.end())
			{
				return *it;
			}
			else
			{
				buildableSunkenTilePositions.erase(*it);
			}

		
		}


		//BWAPI::Position hatchPositionBWP = BWAPI::Position(hatchPosition);
		BWAPI::TilePosition sunkPosition;

		const std::set<BWTA::BaseLocation*, std::less<BWTA::BaseLocation*>> locations = BWTA::getBaseLocations();
		BWTA::BaseLocation *myLocation;

		for (BWTA::BaseLocation *p : locations) {
			BWAPI::TilePosition z = p->getTilePosition();
			if (z == hatchPosition){
				// This is the BWTA::Location of the first hatchery.
				myLocation = p;

			}
		}
		// Get the set of mineral patches closest to BWTA::Location of the hatchery(it will return like 8 mineral patches usually in the set)
		const BWAPI::Unitset mineralSet = myLocation->getMinerals();
		//const std::set<BWAPI::Unit*> mineralSet = myLocation->getMinerals();

		int counter3 = 0;
		int theX = 0;
		int theY = 0;
		for (BWAPI::Unit p : mineralSet)
		{
			// Calculate the difference between LeftMostMineralPatch.x - ExpansionHatchery.x and store it in theX
			theX = p->getTilePosition().x - hatchPosition.x;
			// Calculate the difference between LeftMostMineralPatch.y - ExpansionHatchery.y and store it in theY
			theY = p->getTilePosition().y - hatchPosition.y;
			break;
		}

		int gasX = 0;
		int gasY = 0;
		int counter4 = 0;
		//Get all geysers near the expansion -- it should only return 1 for every map we play..
		const BWAPI::Unitset gasSet = myLocation->getGeysers();
		for (BWAPI::Unit p : gasSet)
		{
			// Calculate the difference between Geyser.x- ExpansionHatchery.x and store it in gasX
			gasX = p->getTilePosition().x - hatchPosition.x;
			// Calculate the difference between Geyser.y- ExpansionHatchery.y and store it in gasY
			gasY = p->getTilePosition().y - hatchPosition.y;
			break;
		}





		int newx, newy;

		int outercounter = 0;
		int counter = 0;
		newx = hatchPosition.x;
		newy = hatchPosition.y;

		int beginX = hatchPosition.x;
		int beginY = hatchPosition.y;

		
		//sunkPosition = BWAPI::TilePosition(newx, newy);

		//test4 = BuildingPlacer::Instance().canBuildHere(sunkPosition, b);

		// Form a new sunken position that starts at the hatchery positive.


		std::vector<bool> incrementDecrement(8);


		bool useGasX = false;
		bool useGasY = false;
		bool useMinX = false;
		bool useMinY = false;

		if (abs(gasX) > abs(gasY))
		{
			useGasX = true;
		}
		else
		{
			useGasY = true;
		}

		if (abs(theX) > abs(theY))
		{
			useMinX = true;
		}
		else
		{
			useMinY = true;
		}

		// Gas differences is probably more reliable than mineral differences.
		if (useGasX && useMinX)
		{
			useMinX = false;
			useMinY = true;
		}

		// Gas differences is probably more reliable than mineral differences.
		if (useGasY && useMinY)
		{
			useMinY = false;
			useMinX = true;
		}
		// This is where we decide which directions we can make sunkens in
		// It is based on X and Y differences in LeftMostMineral - Hatchery and Geyser - Hatchery
		// It is not very good right now because we only use two variables. We should use four variables for better dection : theX, theY, gasX, gasY

		// If the difference between LeftMostMineral.y - Hatchery.y is negative : It means the mierals are North of the hatchery
		// If the difference between Geyser.X - Hatchery.X is negative : It means that the geyser is Left of the Hatchery
		if (useMinY && useGasX)
		{
			if (theY < 0 && gasX < 0)
			{
				/* Allow the following directions for sunken to be built :
				Increase X & Keep Y the same (East)
				Increase X & Increase Y (Go South East)
				Decrease X & Increase Y  (Go South West)
				Keep X Same, Increase Y (Go South)

				Go NORTHEAST **Test**
				*/
				incrementDecrement = { true, false, true, true, false, false, true, false };

			}

			// If the difference between LeftMostMineral.y - Hatchery.y is positive : It means the mierals are South of the hatchery
			// If the difference between Geyser.X - Hatchery.X is negative : It means that the geyser is Left of the Hatchery
			else if (gasX < 0 && theY > 0)
			{
				/* Allow the following directions for sunken to be built :
				Increase X & Keep Y the same (East)
				Increase X & Decrease Y (Go North East)
				Decrease X & Decrease Y (Go North West)
				Keep X Same, Decrease Y (Go North)

				GO SOUTHEAST --> Test
				*/
				incrementDecrement = { false, true, true, false, true, false, false, true };
			}

			// If the difference between LeftMostMineral.y - Hatchery.y is negative : It means the mierals are North of the hatchery
			// If the difference between Geyser.X - Hatchery.X is positive : It means that the geyser is Right or East of the Hatchery
			else if (gasX > 0 && theY < 0)
			{
				/* Allow the following directions for sunken to be built :
				Decrease X & Keep Y the same (West)
				Decrease X & Increase Y (Go South West)
				Increase X & Increase Y  (Go South East)
				Keep X Same, Increase Y (Go South)

				Go Northwest
				*/
				incrementDecrement = { true, false, false, true, false, true, true, false };

			}

			// If the difference between LeftMostMineral.y - Hatchery.y is positive : It means the mierals are South of the hatchery
			// If the difference between Geyser.X - Hatchery.X is positive : It means that the geyser is Right or East of the Hatchery
			else if (theY > 0 && gasX > 0)
			{
				/* Decrease X & Keep Y the same (West)
				Decrease X & Decrease Y (Go North West)
				Increase X & Decrease Y (Go North East)
				Don't change X Decrease y (Go North)

				Go Southwest
				*/
				incrementDecrement = { false, true, false, false, true, true, false, true };

			}
		}


		else if (useMinX && useGasY)
		{
			// If the difference between LeftMostMineral.x - Hatchery.x is positive : It means the mierals are East of the hatchery
			// If the difference between Geyser.Y - Hatchery.Y is negative : It means that the geyser is North of the Hatchery
			if (gasY < 0 && theX > 0)
			{
				/* Decrease X(Go West)
				Increase Y(Go South)
				Decrease X, Increase Y(Go South West)
				Decrease X, Decrease Y(Go North West)
				I think can try SouthEast ? */
				incrementDecrement = { false, false, false, true, true, true, true, false };

			}

			// If the difference between LeftMostMineral.x - Hatchery.x is positive : It means the mierals are East of the hatchery
			// If the difference between Geyser.Y - Hatchery.Y is positive : It means that the geyser is South of the Hatchery
			else if (theX > 0 && gasY > 0)
			{
				/* Decrease X(Go West)
				Decrease Y(Go North)
				Decrease X, INcrease Y(Go SOuth West)
				Decrease X, Decrease Y(Go North West)
				I think can try NOrthEast?
				*/
				incrementDecrement = { false, false, false, true, true, true, false, true };

			}

			// If the difference between LeftMostMineral.x - Hatchery.x is negative : It means the minerals are West of the hatchery
			// If the difference between Geyser.Y - Hatchery.Y is negative : It means that the geyser is North of the Hatchery
			else if (gasY < 0 && theX < 0)
			{
				/* Increase X(Go East)
				Increase Y(Go South)
				Increase X, Increase Y(Go South East)
				Increase X, Decrease Y(Go North East)
				I think maybe Southwest is okay?.. Even NW might be okay */
				incrementDecrement = { true, true, true, false, false, false, true, false };
			}

			// If the difference between LeftMostMineral.x - Hatchery.x is negative : It means the minerals are West of the hatchery
			// If the difference between Geyser.Y - Hatchery.Y is positive : It means that the geyser is South of the Hatchery
			else if (gasY > 0 && theX < 0)
			{

				incrementDecrement = { true, true, true, false, false, false, false, true };
				/* Increase X(Go East)
				Decrease Y(Go North)
				Increase X, Increase Y(Go South East)
				Increase X, Decrease Y(Go North East)
				I think maybe Northwest is okay? */

			}
		}

		beginX = hatchPosition.x;
		beginY = hatchPosition.y;

		std::vector<std::pair<int, int> > myVec;
		std::pair<int, int> p1;

		for (int i = 0; i < 8; i++)
		{
			if (incrementDecrement[i])
			{
				if (i == 0)
				{
					p1.first = 1;
					p1.second = 1;
				}

				else if (i == 1)
				{
					p1.first = 1;
					p1.second = -1;
				}

				else if (i == 2)
				{
					p1.first = 1;
					p1.second = 0;
				}

				else if (i == 3)
				{
					p1.first = -1;
					p1.second = 1;
				}

				else if (i == 4)
				{
					p1.first = -1;
					p1.second = -1;
				}

				else if (i == 5)
				{
					p1.first = -1;
					p1.second = 0;
				}

				else if (i == 6)
				{
					p1.first = 0;
					p1.second = 1;
				}

				else if (i == 7)
				{
					p1.first = 0;
					p1.second = -1;
				}

				myVec.push_back(p1);
			}

		}


		for (int i = 0; i < 30; i++)
		for (int j = 0; j < 30; j++)
		for (int k = 0; k < 30; k++)
		for (int l = 0; l < 30; l++)
		{
			int xChange = beginX;
			int yChange = beginY;

			xChange += i * myVec[0].first;
			yChange += i * myVec[0].second;

			xChange += j * myVec[1].first;
			yChange += j * myVec[1].second;

			xChange += k * myVec[2].first;
			yChange += k * myVec[2].second;

			xChange += l * myVec[3].first;
			yChange += l * myVec[3].second;

			sunkPosition = BWAPI::TilePosition(xChange, yChange);

			Building b(sunk, sunkPosition);

			if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, b) && stuffBlocking.find(sunkPosition) == stuffBlocking.end() && createdBuilding.find(sunkPosition) == createdBuilding.end())
			{
				buildableSunkenTilePositions.insert(sunkPosition);
			}
		}


		if (buildableSunkenTilePositions.size() != 0)
		{
			std::set<BWAPI::TilePosition>::iterator it = buildableSunkenTilePositions.begin();
			return *it;
		}
		else
		{
			return BWAPI::TilePositions::None;
		}
	}
}
		/* We check the following conditions :
			Do we not have creep at the sunken position?
			Can we not build at the sunken position?
			Have we built at this position before?
			If any of these are true and counter <= 7 --> Remain in loop
			Is the counter <= 7 --> Makes sure not to go out of bounds for the incrementDecrement vector
			*/

			

		// Since counter == 8, it means we tried all 8 directions and couldnt find a sunken position, so return None(sunken will be made in main base)



	


		
// gets called every frasme from GameCommander
void BuildingManager::update()
{
	validateWorkersAndBuildings();          // check to see if assigned workers have died en route or while constructing
	assignWorkersToUnassignedBuildings();   // assign workers to the unassigned buildings and label them 'planned'    
	constructAssignedBuildings();           // for each planned building, if the worker isn't constructing, send the command    
	checkForStartedConstruction();          // check to see if any buildings have started construction and update data structures    
	checkForDeadTerranBuilders();           // if we are terran and a building is under construction without a worker, assign a new one    
	checkForCompletedBuildings();           // check to see if any buildings have completed and update data structures
}

bool BuildingManager::isBeingBuilt(BWAPI::UnitType type)
{
	for (auto & b : _buildings)
	{
		if (b.type == type)
		{
			return true;
		}
	}

	return false;
}

// STEP 1: DO BOOK KEEPING ON WORKERS WHICH MAY HAVE DIED
void BuildingManager::validateWorkersAndBuildings()
{
	// TODO: if a terran worker dies while constructing and its building
	//       is under construction, place unit back into buildingsNeedingBuilders

	std::vector<Building> toRemove;

	// find any buildings which have become obsolete
	for (auto & b : _buildings)
	{
		if (b.status != BuildingStatus::UnderConstruction)
		{
			continue;
		}

		if (b.buildingUnit == nullptr || !b.buildingUnit->getType().isBuilding() || b.buildingUnit->getHitPoints() <= 0)
		{
			toRemove.push_back(b);
		}
	}

	removeBuildings(toRemove);
}

// STEP 2: ASSIGN WORKERS TO BUILDINGS WITHOUT THEM
void BuildingManager::assignWorkersToUnassignedBuildings()
{
	// for each building that doesn't have a builder, assign one
	for (Building & b : _buildings)
	{
		if (b.status != BuildingStatus::Unassigned)
		{
			continue;
		}

		if (_debugMode) { BWAPI::Broodwar->printf("Assigning Worker To: %s", b.type.getName().c_str()); }

		// grab a worker unit from WorkerManager which is closest to this final position
		BWAPI::Unit workerToAssign = WorkerManager::Instance().getBuilder(b);

		if (workerToAssign)
		{
			//BWAPI::Broodwar->printf("VALID WORKER BEING ASSIGNED: %d", workerToAssign->getID());

			// TODO: special case of terran building whose worker died mid construction
			//       send the right click command to the buildingUnit to resume construction
			//           skip the buildingsAssigned step and push it back into buildingsUnderConstruction

			b.builderUnit = workerToAssign;

			BWAPI::TilePosition testLocation = getBuildingLocation(b);
			if (!testLocation.isValid())
			{
				continue;
			}

			b.finalPosition = testLocation;

			// reserve this building's space
			BuildingPlacer::Instance().reserveTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

			b.status = BuildingStatus::Assigned;
		}
	}
}

// STEP 3: ISSUE CONSTRUCTION ORDERS TO ASSIGN BUILDINGS AS NEEDED
void BuildingManager::constructAssignedBuildings()
{
	for (auto & b : _buildings)
	{
		if (b.status != BuildingStatus::Assigned)
		{
			continue;
		}

		// if that worker is not currently constructing
		if (!b.builderUnit->isConstructing())
		{
			// if we haven't explored the build position, go there
			if (!isBuildingPositionExplored(b))
			{
				Micro::SmartMove(b.builderUnit, BWAPI::Position(b.finalPosition));
			}
			// if this is not the first time we've sent this guy to build this
			// it must be the case that something was in the way of building
			else if (b.buildCommandGiven)
			{
				// tell worker manager the unit we had is not needed now, since we might not be able
				// to get a valid location soon enough
				WorkerManager::Instance().finishedWithWorker(b.builderUnit);

				// free the previous location in reserved
				BuildingPlacer::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

				// nullify its current builder unit
				b.builderUnit = nullptr;

				// reset the build command given flag
				b.buildCommandGiven = false;

				// add the building back to be assigned
				b.status = BuildingStatus::Unassigned;
			}
			else
			{
				// issue the build order!
				if (b.type == 131)
				{
					createdHatcheriesSet.insert(b.finalPosition);
					createdHatcheriesVector.push_back(b.finalPosition);
					firstHatcheryPosition = BWAPI::Position(b.finalPosition);
				}

				//|| b.type == 146 is sunken
				else if (b.type == 143)
				{
					
					BWAPI::TilePosition sunkPos = getSunkenPosition();
					//b.finalPosition = sunkPos;
					if (sunkPos != BWAPI::TilePositions::None)
					{
						b.finalPosition = sunkPos;
						buildableSunkenTilePositions.erase(sunkPos);
					}

					createdSunkenSet.insert(b.finalPosition);
					createdSunkenVector.push_back(b.finalPosition);

				}

				
	
					b.builderUnit->build(b.type, b.finalPosition);
					createdBuilding.insert(b.finalPosition);
			

				// set the flag to true
				b.buildCommandGiven = true;
			}
		}
	}
}

// STEP 4: UPDATE DATA STRUCTURES FOR BUILDINGS STARTING CONSTRUCTION
void BuildingManager::checkForStartedConstruction()
{
	// for each building unit which is being constructed
	for (auto & buildingStarted : BWAPI::Broodwar->self()->getUnits())
	{
		/*if (buildingStarted->getType() == BWAPI::UnitTypes::Zerg_Creep_Colony && !buildingStarted->isBeingConstructed())
		{
			//b.buildingUnit->morph(BWAPI::UnitTypes::Zerg_Creep_Colony);
			MetaType type(BWAPI::UnitTypes::Zerg_Creep_Colony);
			ProductionManager::Instance()._queue.queueAsHighestPriority(type, true);
		}
		*/
		// filter out units which aren't buildings under construction
		if (!buildingStarted->getType().isBuilding() || !buildingStarted->isBeingConstructed())
		{
			continue;
		}

		// check all our building status objects to see if we have a match and if we do, update it
		for (auto & b : _buildings)
		{
			if (b.status != BuildingStatus::Assigned)
			{
				continue;
			}

			// check if the positions match
			if (b.finalPosition == buildingStarted->getTilePosition())
			{
				// the resources should now be spent, so unreserve them
				_reservedMinerals -= buildingStarted->getType().mineralPrice();
				_reservedGas -= buildingStarted->getType().gasPrice();

				// flag it as started and set the buildingUnit
				b.underConstruction = true;
				b.buildingUnit = buildingStarted;

				// if we are zerg, the buildingUnit now becomes nullptr since it's destroyed
				if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
				{
					b.builderUnit = nullptr;
					// if we are protoss, give the worker back to worker manager
				}
				else if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
				{
					// if this was the gas steal unit then it's the scout worker so give it back to the scout manager
					if (b.isGasSteal)
					{
						ScoutManager::Instance().setWorkerScout(b.builderUnit);
					}
					// otherwise tell the worker manager we're finished with this unit
					else
					{
						WorkerManager::Instance().finishedWithWorker(b.builderUnit);
					}

					b.builderUnit = nullptr;
				}

				// put it in the under construction vector
				b.status = BuildingStatus::UnderConstruction;

				// free this space
				BuildingPlacer::Instance().freeTiles(b.finalPosition, b.type.tileWidth(), b.type.tileHeight());

				// only one building will match
				break;
			}
		}
	}
}

// STEP 5: IF WE ARE TERRAN, THIS MATTERS, SO: LOL
void BuildingManager::checkForDeadTerranBuilders() {}

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
				/*
				if (b.buildingUnit->getType() == BWAPI::UnitTypes::Zerg_Hatchery)
				{
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
			// if we are terran, give the worker back to worker manager
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
		}
		else
		{
			/*
			if (b.type == 131 && b.buildingUnit->getHitPoints() >= 1050 && !sentFirstDroneForSunken)
			{
				BWAPI::Broodwar->printf("Send Drone");
				BWAPI::Unitset candidateProducers;

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
				
				//BWAPI::Unit myUnit = ProductionManager::Instance().getClosestUnitToPosition(candidateProducers, BWAPI::Position(b.finalPosition));
				/*
				BWAPI::Unit closestUnit = nullptr;
				double minDist(1000000);

				for (auto & unit : candidateProducers)
				{
					UAB_ASSERT(unit != nullptr, "Unit was null");

					double distance = unit->getDistance(BWAPI::Position(b.finalPosition));
					if (!closestUnit || distance < minDist)
					{
						closestUnit = unit;
						minDist = distance;
					}
				}
				

				//MetaType m = MetaType(b.buildingUnit->getType());
				//MetaType m = MetaType(BWAPI::UnitTypes::Zerg_Drone);
			
				//BWAPI::Unit myUnit  = .getProducer(m, BWAPI::Position(b.finalPosition));
				closestUnit->move(BWAPI::Position(b.finalPosition));
				
				sentFirstDroneForSunken = true;
				
			}
			int x = 0;
			*/
		}
	}

	removeBuildings(toRemove);
}

// COMPLETED
bool BuildingManager::isEvolvedBuilding(BWAPI::UnitType type)
{
	if (type == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
		type == BWAPI::UnitTypes::Zerg_Spore_Colony ||
		type == BWAPI::UnitTypes::Zerg_Lair ||
		type == BWAPI::UnitTypes::Zerg_Hive ||
		type == BWAPI::UnitTypes::Zerg_Greater_Spire)
	{
		return true;
	}

	return false;
}

// add a new building to be constructed
void BuildingManager::addBuildingTask(BWAPI::UnitType type, BWAPI::TilePosition desiredLocation, bool isGasSteal)
{
	_reservedMinerals += type.mineralPrice();
	_reservedGas += type.gasPrice();

	Building b(type, desiredLocation);
	b.isGasSteal = isGasSteal;
	b.status = BuildingStatus::Unassigned;

	_buildings.push_back(b);
}

bool BuildingManager::isBuildingPositionExplored(const Building & b) const
{
	BWAPI::TilePosition tile = b.finalPosition;

	// for each tile where the building will be built
	for (int x = 0; x<b.type.tileWidth(); ++x)
	{
		for (int y = 0; y<b.type.tileHeight(); ++y)
		{
			if (!BWAPI::Broodwar->isExplored(tile.x + x, tile.y + y))
			{
				return false;
			}
		}
	}

	return true;
}


char BuildingManager::getBuildingWorkerCode(const Building & b) const
{
	return b.builderUnit == nullptr ? 'X' : 'W';
}

int BuildingManager::getReservedMinerals()
{
	return _reservedMinerals;
}

int BuildingManager::getReservedGas()
{
	return _reservedGas;
}

void BuildingManager::drawBuildingInformation(int x, int y)
{
	if (!Config::Debug::DrawBuildingInfo)
	{
		return;
	}

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		BWAPI::Broodwar->drawTextMap(unit->getPosition().x, unit->getPosition().y + 5, "\x07%d", unit->getID());
	}

	BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Building Information:");
	BWAPI::Broodwar->drawTextScreen(x, y + 20, "\x04 Name");
	BWAPI::Broodwar->drawTextScreen(x + 150, y + 20, "\x04 State");

	int yspace = 0;

	for (const auto & b : _buildings)
	{
		if (b.status == BuildingStatus::Unassigned)
		{
			BWAPI::Broodwar->drawTextScreen(x, y + 40 + ((yspace)* 10), "\x03 %s", b.type.getName().c_str());
			BWAPI::Broodwar->drawTextScreen(x + 150, y + 40 + ((yspace++) * 10), "\x03 Need %c", getBuildingWorkerCode(b));
		}
		else if (b.status == BuildingStatus::Assigned)
		{
			BWAPI::Broodwar->drawTextScreen(x, y + 40 + ((yspace)* 10), "\x03 %s %d", b.type.getName().c_str(), b.builderUnit->getID());
			BWAPI::Broodwar->drawTextScreen(x + 150, y + 40 + ((yspace++) * 10), "\x03 A %c (%d,%d)", getBuildingWorkerCode(b), b.finalPosition.x, b.finalPosition.y);

			int x1 = b.finalPosition.x * 32;
			int y1 = b.finalPosition.y * 32;
			int x2 = (b.finalPosition.x + b.type.tileWidth()) * 32;
			int y2 = (b.finalPosition.y + b.type.tileHeight()) * 32;

			BWAPI::Broodwar->drawLineMap(b.builderUnit->getPosition().x, b.builderUnit->getPosition().y, (x1 + x2) / 2, (y1 + y2) / 2, BWAPI::Colors::Orange);
			BWAPI::Broodwar->drawBoxMap(x1, y1, x2, y2, BWAPI::Colors::Red, false);
		}
		else if (b.status == BuildingStatus::UnderConstruction)
		{
			BWAPI::Broodwar->drawTextScreen(x, y + 40 + ((yspace)* 10), "\x03 %s %d", b.type.getName().c_str(), b.buildingUnit->getID());
			BWAPI::Broodwar->drawTextScreen(x + 150, y + 40 + ((yspace++) * 10), "\x03 Const %c", getBuildingWorkerCode(b));
		}
	}
}

BuildingManager & BuildingManager::Instance()
{
	static BuildingManager instance;
	return instance;
}

std::vector<BWAPI::UnitType> BuildingManager::buildingsQueued()
{
	std::vector<BWAPI::UnitType> buildingsQueued;

	for (const auto & b : _buildings)
	{
		if (b.status == BuildingStatus::Unassigned || b.status == BuildingStatus::Assigned)
		{
			buildingsQueued.push_back(b.type);
		}
	}

	return buildingsQueued;
}


BWAPI::TilePosition BuildingManager::getBuildingLocation(const Building & b)
{
	int numPylons = BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Pylon);

	if (b.isGasSteal)
	{
		BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
		UAB_ASSERT(enemyBaseLocation, "Should have enemy base location before attempting gas steal");
		UAB_ASSERT(enemyBaseLocation->getGeysers().size() > 0, "Should have spotted an enemy geyser");

		for (auto & unit : enemyBaseLocation->getGeysers())
		{
			BWAPI::TilePosition tp(unit->getInitialTilePosition());
			return tp;
		}
	}

	if (b.type.requiresPsi() && numPylons == 0)
	{
		return BWAPI::TilePositions::None;
	}

	if (b.type.isRefinery())
	{
		return BuildingPlacer::Instance().getRefineryPosition();
	}

	if (b.type.isResourceDepot())
	{
		// get the location
		BWAPI::TilePosition tile = MapTools::Instance().getNextExpansion();

		return tile;
	}

	// set the building padding specifically
	int distance = b.type == BWAPI::UnitTypes::Protoss_Photon_Cannon ? 0 : Config::Macro::BuildingSpacing;
	if (b.type == BWAPI::UnitTypes::Protoss_Pylon && (numPylons < 3))
	{
		distance = Config::Macro::PylonSpacing;
	}

	// get a position within our region
	return BuildingPlacer::Instance().getBuildLocationNear(b, distance, false);
}

void BuildingManager::removeBuildings(const std::vector<Building> & toRemove)
{
	for (auto & b : toRemove)
	{
		auto & it = std::find(_buildings.begin(), _buildings.end(), b);

		if (it != _buildings.end())
		{
			_buildings.erase(it);
		}
	}
}

