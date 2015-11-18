#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroManager;

class TransportManager : public MicroManager
{
	std::vector<BWAPI::Position>    _mapEdgeVertices; 
	BWAPI::UnitInterface*	        _transportShip;
	int                             _currentRegionVertexIndex;
	BWAPI::Position					_minCorner;
	BWAPI::Position					_maxCorner;
	std::vector<BWAPI::Position>	_waypoints;	// along with to and from, does not seem to be used
	BWAPI::Position					_to;		// to and from are both -1,-1 and don't seem to be used
	BWAPI::Position					_from;

	void							calculateMapEdgeVertices(); // calculates _mapEdgeVertices
	void							drawTransportInformation(int x, int y);
	void							moveTransport();	// calls followPerimeter() to retreat the transport
	void							moveTroops();	// unloads troops
	BWAPI::Position                 getFleePosition(int clockwise=1);	// gets the closest position on the edge of the map to the transport
																		// and moves along the perimeter
	void                            followPerimeter(int clockwise=1);	// uses getFleePosition to move the transport to the edge of the map
	void							followPerimeter(BWAPI::Position to, BWAPI::Position from);
	int                             getClosestVertexIndex(BWAPI::UnitInterface * unit);	// gets the closest vertex on the edge of the map
	int								getClosestVertexIndex(BWAPI::Position p);	// gets the closest vertex on the edge of the map
	std::pair<int, int>				findSafePath(BWAPI::Position from, BWAPI::Position to);
	
public:

	TransportManager();		// CombatCommander fills up a drop ship, then sends it to attack the enemy base

	void							executeMicro(const BWAPI::Unitset & targets);	// seems to have no function
	void							update();	// called by Squad.update(). calls calculateMapEdgeVertices, moveTroops and moveTransport
	void							setTransportShip(BWAPI::UnitInterface * unit); // unused?
	void							setFrom(BWAPI::Position from); // unused?
	void							setTo(BWAPI::Position to);  // unused?
};
}