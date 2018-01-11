/*
 * GlobalEffect.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "Effect.h"

namespace spells
{
namespace effects
{

class GlobalEffect : public Effect
{
public:
	GlobalEffect(const int level);
	virtual ~GlobalEffect();

	void adjustTargetTypes(std::vector<TargetType> & types) const override;

	void adjustAffectedHexes(std::set<BattleHex> & hexes, const Mechanics * m, const Target & spellTarget) const override;

	EffectTarget filterTarget(const Mechanics * m, const EffectTarget & target) const override;

	EffectTarget transformTarget(const Mechanics * m,  const Target & aimPoint, const Target & spellTarget) const override;
protected:

private:
};

}
}
