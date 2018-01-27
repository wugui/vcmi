/*
 * StartInfo.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "StartInfo.h"


PlayerSettings::PlayerSettings()
	: bonus(RANDOM), castle(NONE), hero(RANDOM), heroPortrait(RANDOM), color(0), handicap(NO_HANDICAP), team(0), compOnly(false)
{

}

bool PlayerSettings::isControlledByAI() const
{
	return !connectedPlayerIDs.size();
}

bool PlayerSettings::isControlledByHuman() const
{
	return connectedPlayerIDs.size();
}

PlayerSettings & StartInfo::getIthPlayersSettings(PlayerColor no)
{
	if(playerInfos.find(no) != playerInfos.end())
		return playerInfos[no];
	logGlobal->error("Cannot find info about player %s. Throwing...", no.getStr());
	throw std::runtime_error("Cannot find info about player");
}
const PlayerSettings & StartInfo::getIthPlayersSettings(PlayerColor no) const
{
	return const_cast<StartInfo&>(*this).getIthPlayersSettings(no);
}

PlayerSettings * StartInfo::getPlayersSettings(const ui8 connectedPlayerId)
{
	for(auto & elem : playerInfos)
	{
		if(vstd::contains(elem.second.connectedPlayerIDs, connectedPlayerId))
			return &elem.second;
	}

	return nullptr;
}


bool LobbyInfo::isClientHost(int clientId) const
{
	return clientId == hostClientId;
}

std::set<PlayerColor> LobbyInfo::getAllClientPlayers(int clientId) //MPTODO: this function has dupe i suppose
{
	std::set<PlayerColor> players;
	for(auto & elem : si->playerInfos)
	{
		if(isClientHost(clientId) && elem.second.isControlledByAI())
			players.insert(elem.first);

		for(ui8 id : elem.second.connectedPlayerIDs)
		{
			if(vstd::contains(getConnectedPlayerIdsForClient(clientId), id))
				players.insert(elem.first);
		}
	}
	if(isClientHost(clientId))
		players.insert(PlayerColor::NEUTRAL);

	return players;
}

std::vector<ui8> LobbyInfo::getConnectedPlayerIdsForClient(int clientId) const
{
	std::vector<ui8> ids;

	for(auto & pair : playerNames)
	{
		if(pair.second.connection == clientId)
		{
			for(auto & elem : si->playerInfos)
			{
				if(vstd::contains(elem.second.connectedPlayerIDs, pair.first))
					ids.push_back(pair.first);
			}
		}
	}
	return ids;
}
