// Get a sunken position depending on whether or not we have an expansion.
BWAPI::TilePosition BuildingManager::getSunkenPosition()
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
		// we don’t want to build in the same side. And also block the direction to expand there as well.
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
		if ((gasX*gasX) >(gasY*gasY)){ if (gasX < 0){ gaspos = 1; } else{ gaspos = 3; } }// left and right
		else{ if (gasY < 0){ gaspos = 4; } else{ gaspos = 2; } }// top and bot

		// get mineral position
		int minepos = 0;
		if ((mineX*mineX) >(mineY*mineY)){ if (mineX < 0){ minepos = 1; } else{ minepos = 3; } }// left and right
		else{ if (mineY < 0){ minepos = 4; } else{ minepos = 2; } }// top and bot

		// the place position nearest gas
		int placepos = -1;
		if (place[gaspos]){ placepos = gaspos; }
		else if (place[gaspos + 1]){ placepos = gaspos + 1; }
		else if (place[gaspos - 1]){ placepos = gaspos - 1; }
		else if (place[gaspos + 2]){ placepos = gaspos + 2; }

		// give the sunken position
		int sunkenX = hatchPosition.x;
		int sunkenY = hatchPosition.y;

		switch (placepos){
		case 5:
		case 1://left
			sunkenX -= 3;
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
		if ((placepos != direcpos) && (((placepos - direcpos)*(placepos - direcpos)) != 16)){ rowpos = placepos; } // placepos == direction
		else if (((minepos + 2) != direcpos) && ((((minepos + 2) - direcpos)*((minepos + 2) - direcpos)) != 16)){ rowpos = minepos + 2; } // opposite of mineral == direction
		else { rowpos = gaspos + 2; } // opposite of gas == direction

		// to see which place is availible
		BWAPI::TilePosition sunkPosition;
		BWAPI::UnitType sunk = BWAPI::UnitTypes::Zerg_Creep_Colony;
		for (int i = 0; i < 5; i++){
			sunkPosition = BWAPI::TilePosition(sunkenX, sunkenY);
			Building b(sunk, sunkPosition);
			if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, b)) //|| createdBuilding.find(sunkPosition) != createdBuilding.end())
			{
				return sunkPosition;
			}

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
			Building c(sunk, sunkPosition);
			if (BWAPI::Broodwar->hasCreep(sunkPosition) && BuildingPlacer::Instance().canBuildHere(sunkPosition, c)) //|| createdBuilding.find(sunkPosition) != createdBuilding.end())
			{
				return sunkPosition;
			}

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
	return BWAPI::TilePositions::None;
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

