/*
 * Summon.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "Summon.h"
#include "Registry.h"

#include "../ISpellMechanics.h"
#include "../../battle/CBattleInfoCallback.h"
#include "../../battle/Unit.h"
#include "../../NetPacks.h"
#include "../../serializer/JsonSerializeFormat.h"

#include "../../CHeroHandler.h"
#include "../../mapObjects/CGHeroInstance.h"

static const std::string EFFECT_NAME = "core:summon";

namespace spells
{
namespace effects
{

VCMI_REGISTER_SPELL_EFFECT(Summon, EFFECT_NAME);

Summon::Summon(const int level)
	: GlobalEffect(level),
	creature(),
	permanent(false),
	exclusive(true)
{
}

Summon::~Summon() = default;

bool Summon::applicable(Problem & problem, const Mechanics * m) const
{
	auto mode = m->mode;
	if(mode == Mode::AFTER_ATTACK || mode == Mode::BEFORE_ATTACK || mode == Mode::SPELL_LIKE_ATTACK || mode == Mode::MAGIC_MIRROR)
	{
		//should not even try to do it
		MetaString text;
		text.addReplacement("Invalid spell cast attempt: spell %s, mode %d");
		text.addReplacement(MetaString::SPELL_NAME, m->getSpellIndex());
		text.addReplacement2(int(mode));
		problem.add(std::move(text), Problem::CRITICAL);
		return false;
	}

	if(!exclusive)
		return true;

	//check if there are summoned elementals of other type

	auto otherSummoned = m->cb->battleGetUnitsIf([m, this](const battle::Unit * unit)
	{
		return (unit->unitOwner() == m->caster->getOwner())
			&& (unit->isSummoned())
			&& (!unit->isClone())
			&& (unit->creatureId() != creature);
	});

	if(!otherSummoned.empty())
	{
		auto elemental = otherSummoned.front();

		MetaString text;
		text.addTxt(MetaString::GENERAL_TXT, 538);

		auto caster = dynamic_cast<const CGHeroInstance *>(m->caster);
		if(caster)
		{
			text.addReplacement(caster->name);

			text.addReplacement(MetaString::CRE_PL_NAMES, elemental->creatureIndex());

			if(caster->type->sex)
				text.addReplacement(MetaString::GENERAL_TXT, 540);
			else
				text.addReplacement(MetaString::GENERAL_TXT, 539);

		}
		problem.add(std::move(text), Problem::NORMAL);
		return false;
	}

	return true;
}

void Summon::apply(BattleStateProxy * battleState, RNG & rng, const Mechanics * m, const EffectTarget & target) const
{
	//new feature - percentage bonus
	auto amount = m->applySpecificSpellBonus(m->calculateRawEffectValue(0, m->getEffectPower()));
	if(amount < 1)
	{
		battleState->complain("Summoning didn't summon any!");
		return;
	}

	for(auto & dest : target)
	{
		battle::UnitInfo info;
		info.id = m->cb->battleNextUnitId();
		info.count = amount;
		info.type = creature;
		info.side = m->casterSide;
		info.position = dest.hexValue;
		info.summoned = !permanent;

		BattleUnitsChanged pack;
		pack.changedStacks.emplace_back();
		info.toInfo(pack.changedStacks.back());
		battleState->apply(&pack);
	}
}

void Summon::serializeJsonEffect(JsonSerializeFormat & handler)
{
	handler.serializeId("id", creature, CreatureID());
	handler.serializeBool("permanent", permanent, false);
	handler.serializeBool("exclusive", exclusive, true);
}

EffectTarget Summon::transformTarget(const Mechanics * m, const Target & aimPoint, const Target & spellTarget) const
{
	EffectTarget effectTarget;
	BattleHex hex = m->cb->getAvaliableHex(creature, m->casterSide);
	if(!hex.isValid())
		logGlobal->error("No free space to summon creature!");
	else
		effectTarget.push_back(Destination(hex));

	return effectTarget;
}


}
}
