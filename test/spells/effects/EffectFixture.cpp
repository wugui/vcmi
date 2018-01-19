/*
 * EffectFixture.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "EffectFixture.h"

#include <vstd/RNG.h>

#include "../../../lib/serializer/JsonDeserializer.h"

namespace test
{

using namespace ::spells;
using namespace ::spells::effects;
using namespace ::testing;

EffectFixture::EffectFixture(std::string effectName_)
	:subject(),
	problemMock(),
	mechanicsMock(),
	battleFake(),
	effectName(effectName_)
{

}

EffectFixture::~EffectFixture() = default;

void EffectFixture::setupEffect(const JsonNode & effectConfig)
{
	JsonDeserializer deser(nullptr, effectConfig);
	subject->serializeJson(deser);
}

void EffectFixture::setUp()
{
	subject = Effect::create(effectName);
	ASSERT_TRUE(subject);
	battleFake.setUp();
	mechanicsMock.cb = &battleFake;
}

static vstd::TRandI64 getInt64RangeDef(int64_t lower, int64_t upper)
{
	return [=]()->int64_t
	{
		return (lower + upper)/2;
	};
}

static vstd::TRand getDoubleRangeDef(double lower, double upper)
{
	return [=]()->double
	{
		return (lower + upper)/2;
	};
}

void EffectFixture::setupDefaultRNG()
{
	EXPECT_CALL(rngMock, getInt64Range(_,_)).WillRepeatedly(Invoke(&getInt64RangeDef));
	EXPECT_CALL(rngMock, getDoubleRange(_,_)).WillRepeatedly(Invoke(&getDoubleRangeDef));
}

}
