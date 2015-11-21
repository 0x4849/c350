

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

BWAPI::TilePosition BuildingManager::simcity()
{
	if (!simcity_planned){ simcity_init(); simcity_planned = true; }
	BWAPI::UnitType sunk = BWAPI::UnitTypes::Zerg_Creep_Colony;
	for (unsigned i = 0; i < simcity_sunken.size(); i++)
	{
		BWAPI::TilePosition sunkPosition = simcity_sunken[i];
		Building b(sunk, sunkPosition);
		if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, b))
		{ 
			simcity_sunken.erase(simcity_sunken.begin() + i);
			return sunkPosition;
		}
	}	
	return BWAPI::TilePositions::None;
}

// Get a sunken position depending on whether or not we have an expansion.
void BuildingManager::simcity_init()
{

	// Always make sunkens at natural expansion if you can.
	if (createdHatcheriesSet.size() >= 1)
	{

		BWAPI::TilePosition hatchPosition = createdHatcheriesVector[0];

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

		
		std::vector<bool> place(8);
		std::vector<bool> direction(8);
		place = { true, true, true, true, true, true, true, true }; // means[up, left, bot, right,up, left, bot, right]
		direction = { true, true, true, true, true, true, true, true }; //means[up,left,bot,right,up,left,bot,right]
		int mineX = 0;
		int mineY = 0;
		int mindis = 999999;
		for (BWAPI::Unit p : mineralSet)
		{
			// Calculate the difference between LeftMostMineralPatch.x - ExpansionHatchery.x and store it in theX
			int theX = p->getTilePosition().x - hatchPosition.x;
			// Calculate the difference between LeftMostMineralPatch.y - ExpansionHatchery.y and store it in theY
			int theY = p->getTilePosition().y - hatchPosition.y;
			if ((theX*theX + theY*theY) < mindis){ mineX = theX; mineY = theY; mindis = (theX*theX + theY*theY); }
			//break;
		}



		int gasX;
		int gasY;
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

		// define which side is it
		// we don¡¯t want to build in the same side. And also block the direction to expand there as well.
		if ((mineX*mineX) > (mineY*mineY)){
			if (mineX < 0){
				// mineral at left
				place[1] = false; place[5] = false; direction[1] = false; direction[5] = false;
				// gas up or bot
				if (gasX >= 0){ if (gasY < 0){ place[0] = false; place[4] = false; } else{ place[2] = false; place[6] = false; } }// for place
				if (gasX > 0){ if (gasY < 0){ direction[0] = false; direction[4] = false; } else{ direction[2] = false; direction[6] = false; } } // for direction
			}
			else{
				// mineral at right
				place[3] = false; place[7] = false; direction[3] = false; direction[0] = false;
				if (gasX <= 0){ if (gasY < 0){ place[0] = false; place[4] = false; } else{ place[2] = false; place[6] = false; } }
				if (gasX < 0){ if (gasY < 0){ direction[0] = false; direction[4] = false; } else{ direction[2] = false; direction[6] = false; } }
			}
		}
		else{
			if (mineY < 0){
				// mineral at up
				place[0] = false; place[4] = false; direction[0] = false; direction[4] = false;
				if (gasY >= 0){ if (gasX < 0){ place[1] = false; place[5] = false; } else{ place[3] = false; place[7] = false; } }// for place
				if (gasY > 0){ if (gasX < 0){ direction[1] = false; direction[5] = false; } else{ direction[3] = false; direction[7] = false; } } // for direction
			}
			else{
				// mineral at bot
				place[2] = false; place[6] = false; direction[2] = false; direction[7] = false;
				if (gasY <= 0){ if (gasX < 0){ place[1] = false; place[5] = false; } else{ place[3] = false; place[7] = false; } }// for place
				if (gasY < 0){ if (gasX < 0){ direction[1] = false; direction[5] = false; } else{ direction[3] = false; direction[7] = false; } } // for direction
			}
		}

		// get gas position
		int gaspos = 0;
		if ((gasX*gasX) > (gasY*gasY)){ if (gasX < 0){ gaspos = 1; } else{ gaspos = 3; } }// left and right
		else{ if (gasY < 0){ gaspos = 4; } else{ gaspos = 2; } }// top and bot

		// get mineral position
		int minepos = 0;
		if ((mineX*mineX) >(mineY*mineY)){ if (mineX < 0){ minepos = 1; } else{ minepos = 3; } }// left and right
		else{ if (mineY < 0){ minepos = 4; } else{ minepos = 2; } }// top and bot

		// the place position nearest gas
		int placepos = -1;
		if (place[gaspos]){ placepos = gaspos; }
		else if (place[gaspos+1]){ placepos = gaspos+1; }
		else if (place[gaspos - 1]){ placepos = gaspos-1; }
		else if (place[gaspos + 2]){ placepos = gaspos+2; }

		// give the sunken position
		int sunkenX = hatchPosition.x;
		int sunkenY = hatchPosition.y;

		switch (placepos){
		case 5:
		case 1://left
			sunkenX -= 2;
			direction[3] = false; direction[7] = false;
			break;
		case 6:
		case 2://bot
			sunkenY += 4;
			direction[0] = false; direction[4] = false;
			break;
		case 7:
		case 3://right
			sunkenX += 3;
			direction[1] = false; direction[5] = false;
			break;
		case 0:
		case 4://top
			sunkenY -= 3;
			direction[2] = false; direction[6] = false;
			break;
		}

		// the direction position nearest gas
		int direcpos = -1;
		if (direction[gaspos]){ direcpos = gaspos; }
		else if (direction[gaspos + 1]){ direcpos = gaspos + 1; }
		else if (direction[gaspos - 1]){ direcpos = gaspos - 1; }
		else if (direction[gaspos + 2]){ direcpos = gaspos + 2; }

		// use for second row of 
		int sunkenX2;
		int sunkenY2;
		int rowpos = -1;
		if ((placepos != direcpos) && (((placepos - direcpos)*(placepos - direcpos))!=16)){ rowpos = placepos; } // placepos == direction
		else if (((minepos + 2) != direcpos) && ((((minepos + 2) - direcpos)*((minepos + 2) - direcpos)) != 16)){ rowpos = minepos + 2; } // opposite of mineral == direction
		else { rowpos = gaspos + 2; } // opposite of gas == direction

		// to see which place is availible
		BWAPI::TilePosition sunkPosition;
		//BWAPI::UnitType sunk = BWAPI::UnitTypes::Zerg_Creep_Colony;
		for (int i = 0; i < 3; i++){
			sunkPosition = BWAPI::TilePosition(sunkenX, sunkenY);
			//Building b(sunk, sunkPosition);
			//if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, b)) {return sunkPosition;}
			simcity_sunken.push_back(sunkPosition);
			// look at the second row
			switch (rowpos){
			case 5:
			case 1://left
				sunkenX2 = sunkenX - 2;
				sunkenY2 = sunkenY;
				break;
			case 6:
			case 2://bot
				sunkenY2 = sunkenY + 2;
				sunkenX2 = sunkenX;
				break;
			case 7:
			case 3://right
				sunkenX2 = sunkenX + 2;
				sunkenY2 = sunkenY;
				break;
			case 0:
			case 4://top
				sunkenY2 = sunkenY - 2;
				sunkenX2 = sunkenX;
				break;
			}
			sunkPosition = BWAPI::TilePosition(sunkenX2, sunkenY2);
			//Building c(sunk, sunkPosition);
			//if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, c)){return sunkPosition;}
			simcity_sunken.push_back(sunkPosition);


			// look at the third row
			switch (rowpos){
			case 5:
			case 1://left
				sunkenX2 = sunkenX - 4;
				sunkenY2 = sunkenY;
				break;
			case 6:
			case 2://bot
				sunkenY2 = sunkenY + 4;
				sunkenX2 = sunkenX;
				break;
			case 7:
			case 3://right
				sunkenX2 = sunkenX + 4;
				sunkenY2 = sunkenY;
				break;
			case 0:
			case 4://top
				sunkenY2 = sunkenY - 4;
				sunkenX2 = sunkenX;
				break;
			}
			sunkPosition = BWAPI::TilePosition(sunkenX2, sunkenY2);
			//Building c(sunk, sunkPosition);
			//if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, c)){return sunkPosition;}
			simcity_wall.push_back(sunkPosition);


			// next one of the row
			switch (direcpos){
			case 5:
			case 1://left
				sunkenX -= 2;
				break;
			case 6:
			case 2://bot
				sunkenY += 2;
				break;
			case 7:
			case 3://right
				sunkenX += 2;
				break;
			case 0:
			case 4://top
				sunkenY -= 2;
				break;
			}
		}
		//return BWAPI::TilePositions::(sunkenX, sunkenY);


	}
	//return BWAPI::TilePositions::None;
}

		
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


				else if (b.type == 143 || b.type == 146)
				{
					BWAPI::TilePosition sunkPos = simcity();

					if (sunkPos != BWAPI::TilePositions::None)
					{
						b.finalPosition = sunkPos;
					}
				

					createdSunkenSet.insert(b.finalPosition);
					createdSunkenVector.push_back(b.finalPosition);

				}

				createdBuilding.insert(b.finalPosition);
				b.builderUnit->build(b.type, b.finalPosition);

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
					b.buildingUnit->morph(BWAPI::UnitTypes::Zerg_Sunken_Colony);
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

