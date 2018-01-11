/*
 * Destination.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "BattleHex.h"

namespace battle
{

class Unit;

class DLL_LINKAGE Destination
{
public:
	Destination();
	~Destination();
	explicit Destination(const Unit * destination);
	explicit Destination(const BattleHex & destination);

	Destination(const Destination & other);

	Destination & operator=(const Destination & other);

	const Unit * unitValue;
	BattleHex hexValue;
};

using Target = std::vector<Destination>;

}
