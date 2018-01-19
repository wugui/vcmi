/*
 * EffectFixture.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "../../../lib/spells/effects/Effect.h"

#include "mock/mock_spells_Mechanics.h"
#include "mock/mock_spells_Problem.h"

#include "mock/mock_battle_IBattleState.h"
#include "mock/mock_battle_Unit.h"
#include "mock/mock_vstd_RNG.h"

#include "../../../lib/JsonNode.h"
#include "../../../lib/NetPacksBase.h"
#include "../../../lib/battle/CBattleInfoCallback.h"

namespace test
{

class EffectFixture
{
public:
	class BattleFake : public CBattleInfoCallback, public BattleStateMock
	{
	public:
		BattleFake()
			: CBattleInfoCallback(),
			BattleStateMock()
		{
		}

		void setUp()
		{
			CBattleInfoCallback::setBattle(this);
		}
	};

	std::shared_ptr<::spells::effects::Effect> subject;
	::spells::ProblemMock problemMock;
	::spells::MechanicsMock mechanicsMock;
	vstd::RNGMock rngMock;

	BattleFake battleFake;

	std::string effectName;

	EffectFixture(std::string effectName_);
	virtual ~EffectFixture();

	void setupEffect(const JsonNode & effectConfig);

	void setupDefaultRNG();

protected:
	void setUp();

private:
};

}
