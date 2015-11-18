#pragma once

#include "Squad.h"

namespace UAlbertaBot
{
class SquadData
{
	std::map<std::string, Squad> _squads;

    void    updateAllSquads(); // calls update() of each Squad
    void    verifySquadUniqueMembership(); // prevents a unit from being in multiple squads

public:

	SquadData();

    void            clearSquadData(); // unused

    bool            canAssignUnitToSquad(BWAPI::Unit unit, const Squad & squad) const; // compares priorities of a unit's squad
																					   // to the new squad. called by squad updates
																					   // in CombatCommander to prevent moving down
																					   // squad priorities
    void            assignUnitToSquad(BWAPI::Unit unit, Squad & squad); // safe way to assign a unit to a squad
    void            addSquad(const std::string & squadName, const Squad & squad); // called by CombatCommander.initializeSquads()
																				  // as well as updateDefenseSquads() to create new
																			      // squads for defending allied regions
    void            removeSquad(const std::string & squadName);
    void            clearSquad(const std::string & squadName);
	void            drawSquadInformation(int x, int y);

    void            update(); // updateAllSquads and verifySquadUniqueMembership. called by CombatCommander.update() loop
    void            setRegroup(); // unused

    bool            squadExists(const std::string & squadName);
    bool            unitIsInSquad(BWAPI::Unit unit) const;	// useful to prevent reassigning a unit to a squad. THIS HAS BEEN HEAVILY MODIFIED
    const Squad *   getUnitSquad(BWAPI::Unit unit) const;
    Squad *         getUnitSquad(BWAPI::Unit unit);

    Squad &         getSquad(const std::string & squadName);
    const std::map<std::string, Squad> & getSquads() const;
};
}