/*
 * battle_UnitTest.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "../../lib/battle/Unit.h"

TEST(battle_Unit_getSurroundingHexes, oneWide)
{
	BattleHex position(77);

	auto actual = battle::Unit::getSurroundingHexes(position, false, 0);

	EXPECT_EQ(actual, position.neighbouringTiles());
}

TEST(battle_Unit_getSurroundingHexes, doubleWideAttacker)
{
	BattleHex position(77);

	auto actual = battle::Unit::getSurroundingHexes(position, true, BattleSide::ATTACKER);

	static const std::vector<BattleHex> expected =
	{
		60,
		61,
		78,
		95,
		94,
		93,
		75,
		59
	};

	EXPECT_EQ(actual, expected);
}

TEST(battle_Unit_getSurroundingHexes, doubleWideDefender)
{
	BattleHex position(77);

	auto actual = battle::Unit::getSurroundingHexes(position, true, BattleSide::DEFENDER);

	static const std::vector<BattleHex> expected =
	{
		60,
		61,
		62,
		79,
		96,
		95,
		94,
		76,
	};

	EXPECT_EQ(actual, expected);
}
