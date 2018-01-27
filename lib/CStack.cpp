/*
 * CStack.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "CStack.h"

#include <vstd/RNG.h>

#include "CGeneralTextHandler.h"
#include "battle/BattleInfo.h"
#include "spells/CSpellHandler.h"
#include "NetPacks.h"


///CStack
CStack::CStack(const CStackInstance * Base, PlayerColor O, int I, ui8 Side, SlotID S)
	: CBonusSystemNode(STACK_BATTLE),
	CUnitState(),
	base(Base),
	ID(I),
	type(Base->type),
	baseAmount(base->count),
	owner(O),
	slot(S),
	side(Side),
	initialPosition()
{
	health.init(); //???
}

CStack::CStack()
	: CBonusSystemNode(STACK_BATTLE),
	CUnitState()
{
	base = nullptr;
	type = nullptr;
	ID = -1;
	baseAmount = -1;
	owner = PlayerColor::NEUTRAL;
	slot = SlotID(255);
	side = 1;
	initialPosition = BattleHex();
}

CStack::CStack(const CStackBasicDescriptor * stack, PlayerColor O, int I, ui8 Side, SlotID S)
	: CBonusSystemNode(STACK_BATTLE),
	CUnitState(),
	base(nullptr),
	ID(I),
	type(stack->type),
	baseAmount(stack->count),
	owner(O),
	slot(S),
	side(Side),
	initialPosition()
{
	health.init(); //???
}

const CCreature * CStack::getCreature() const
{
	return type;
}

void CStack::localInit(BattleInfo * battleInfo)
{
	battle = battleInfo;
	assert(type);

	exportBonuses();
	if(base) //stack originating from "real" stack in garrison -> attach to it
	{
		attachTo(const_cast<CStackInstance *>(base));
	}
	else //attach directly to obj to which stack belongs and creature type
	{
		CArmedInstance * army = battle->battleGetArmyObject(side);
		attachTo(army);
		attachTo(const_cast<CCreature *>(type));
	}

	CUnitState::localInit(this);
	position = initialPosition;
}

ui32 CStack::level() const
{
	if(base)
		return base->getLevel(); //creature or commander
	else
		return std::max(1, (int)getCreature()->level); //war machine, clone etc
}

si32 CStack::magicResistance() const
{
	si32 magicResistance = IBonusBearer::magicResistance();

	si32 auraBonus = 0;

	for(auto one : battle->battleAdjacentUnits(this))
	{
		if(one->unitOwner() == owner)
			vstd::amax(auraBonus, one->valOfBonuses(Bonus::SPELL_RESISTANCE_AURA)); //max value
	}
	magicResistance += auraBonus;
	vstd::amin(magicResistance, 100);

	return magicResistance;
}

BattleHex::EDir CStack::destShiftDir() const
{
	if(doubleWide())
	{
		if(side == BattleSide::ATTACKER)
			return BattleHex::EDir::RIGHT;
		else
			return BattleHex::EDir::LEFT;
	}
	else
	{
		return BattleHex::EDir::NONE;
	}
}

std::vector<si32> CStack::activeSpells() const
{
	std::vector<si32> ret;

	std::stringstream cachingStr;
	cachingStr << "!type_" << Bonus::NONE << "source_" << Bonus::SPELL_EFFECT;
	CSelector selector = Selector::sourceType(Bonus::SPELL_EFFECT)
						 .And(CSelector([](const Bonus * b)->bool
	{
		return b->type != Bonus::NONE;
	}));

	TBonusListPtr spellEffects = getBonuses(selector, Selector::all, cachingStr.str());
	for(const std::shared_ptr<Bonus> it : *spellEffects)
	{
		if(!vstd::contains(ret, it->sid))  //do not duplicate spells with multiple effects
			ret.push_back(it->sid);
	}

	return ret;
}

CStack::~CStack()
{
	detachFromAll();
}

const CGHeroInstance * CStack::getMyHero() const
{
	if(base)
		return dynamic_cast<const CGHeroInstance *>(base->armyObj);
	else //we are attached directly?
		for(const CBonusSystemNode * n : getParentNodes())
			if(n->getNodeType() == HERO)
				return dynamic_cast<const CGHeroInstance *>(n);

	return nullptr;
}

std::string CStack::nodeName() const
{
	std::ostringstream oss;
	oss << owner.getStr();
	oss << " battle stack [" << ID << "]: " << getCount() << " of ";
	if(type)
		oss << type->namePl;
	else
		oss << "[UNDEFINED TYPE]";

	oss << " from slot " << slot;
	if(base && base->armyObj)
		oss << " of armyobj=" << base->armyObj->id.getNum();
	return oss.str();
}

void CStack::prepareAttacked(BattleStackAttacked & bsa, vstd::RNG & rand) const
{
	auto newState = asquire();
	prepareAttacked(bsa, rand, newState);
}

void CStack::prepareAttacked(BattleStackAttacked & bsa, vstd::RNG & rand, std::shared_ptr<battle::CUnitState> customState)
{
	auto initialCount = customState->getCount();

	customState->damage(bsa.damageAmount);

	bsa.killedAmount = initialCount- customState->getCount();

	if(!customState->alive() && customState->cloned)
	{
		bsa.flags |= BattleStackAttacked::CLONE_KILLED;
	}
	else if(!customState->alive()) //stack killed
	{
		bsa.flags |= BattleStackAttacked::KILLED;

		auto resurrectValue = customState->valOfBonuses(Bonus::REBIRTH);

		if(resurrectValue > 0 && customState->canCast()) //there must be casts left
		{
			double resurrectFactor = resurrectValue / 100;

			auto baseAmount = customState->unitBaseAmount();

			double resurrectedRaw = baseAmount * resurrectFactor;

			int32_t resurrectedCount = static_cast<int32_t>(floor(resurrectedRaw));

			int32_t resurrectedAdd = static_cast<int32_t>(baseAmount - (resurrectedCount/resurrectFactor));

			auto rangeGen = rand.getInt64Range(0, 99);

			for(int32_t i = 0; i < resurrectedAdd; i++)
			{
				if(resurrectValue > rangeGen())
					resurrectedCount += 1;
			}

			if(customState->hasBonusOfType(Bonus::REBIRTH, 1))
			{
				// resurrect at least one Sacred Phoenix
				vstd::amax(resurrectedCount, 1);
			}

			if(resurrectedCount > 0)
			{
				customState->casts.use();
				bsa.flags |= BattleStackAttacked::REBIRTH;
				int64_t toHeal = customState->unitMaxHealth() * resurrectedCount;
				//TODO: add one-battle rebirth?
				customState->heal(toHeal, EHealLevel::RESURRECT, EHealPower::PERMANENT);
				customState->counterAttacks.use(customState->counterAttacks.available());
			}
		}
	}

	customState->toInfo(bsa.newState);
	bsa.newState.healthDelta = -bsa.damageAmount;
}

bool CStack::isMeleeAttackPossible(const battle::Unit * attacker, const battle::Unit * defender, BattleHex attackerPos, BattleHex defenderPos)
{
	if(!attackerPos.isValid())
		attackerPos = attacker->getPosition();
	if(!defenderPos.isValid())
		defenderPos = defender->getPosition();

	return
		(BattleHex::mutualPosition(attackerPos, defenderPos) >= 0)//front <=> front
		|| (attacker->doubleWide()//back <=> front
			&& BattleHex::mutualPosition(attackerPos + (attacker->unitSide() == BattleSide::ATTACKER ? -1 : 1), defenderPos) >= 0)
		|| (defender->doubleWide()//front <=> back
			&& BattleHex::mutualPosition(attackerPos, defenderPos + (defender->unitSide() == BattleSide::ATTACKER ? -1 : 1)) >= 0)
		|| (defender->doubleWide() && attacker->doubleWide()//back <=> back
			&& BattleHex::mutualPosition(attackerPos + (attacker->unitSide() == BattleSide::ATTACKER ? -1 : 1), defenderPos + (defender->unitSide() == BattleSide::ATTACKER ? -1 : 1)) >= 0);

}

std::string CStack::getName() const
{
	return (getCount() == 1) ? type->nameSing : type->namePl; //War machines can't use base
}

bool CStack::canBeHealed() const
{
	return getFirstHPleft() < MaxHealth()
		   && isValidTarget()
		   && !hasBonusOfType(Bonus::SIEGE_WEAPON);
}

ui8 CStack::getSpellSchoolLevel(const spells::Mode mode, const spells::Spell * spell, int * outSelectedSchool) const
{
	int skill = valOfBonuses(Selector::typeSubtype(Bonus::SPELLCASTER, spell->getIndex()));
	vstd::abetween(skill, 0, 3);
	return skill;
}

int64_t CStack::getSpellBonus(const spells::Spell * spell, int64_t base, const battle::Unit * affectedStack) const
{
	//stacks does not have sorcery-like bonuses (yet?)
	return base;
}

int64_t CStack::getSpecificSpellBonus(const spells::Spell * spell, int64_t base) const
{
	return base;
}

int CStack::getEffectLevel(const spells::Mode mode, const spells::Spell * spell) const
{
	return getSpellSchoolLevel(mode, spell);
}

int CStack::getEffectPower(const spells::Mode mode, const spells::Spell * spell) const
{
	return valOfBonuses(Bonus::CREATURE_SPELL_POWER) * getCount() / 100;
}

int CStack::getEnchantPower(const spells::Mode mode, const spells::Spell * spell) const
{
	int res = valOfBonuses(Bonus::CREATURE_ENCHANT_POWER);
	if(res <= 0)
		res = 3;//default for creatures
	return res;
}

int CStack::getEffectValue(const spells::Mode mode, const spells::Spell * spell) const
{
	return valOfBonuses(Bonus::SPECIFIC_SPELL_POWER, spell->getIndex()) * getCount();
}

const PlayerColor CStack::getOwner() const
{
	return unitEffectiveOwner(this);
}

void CStack::getCasterName(MetaString & text) const
{
	//always plural name in case of spell cast.
	addNameReplacement(text, true);
}

void CStack::getCastDescription(const spells::Spell * spell, MetaString & text) const
{
	text.addTxt(MetaString::GENERAL_TXT, 565);//The %s casts %s
	//todo: use text 566 for single creature
	getCasterName(text);
	text.addReplacement(MetaString::SPELL_NAME, spell->getIndex());
}

void CStack::getCastDescription(const spells::Spell * spell, const std::vector<const battle::Unit *> & attacked, MetaString & text) const
{
	getCastDescription(spell, text);
}

void CStack::spendMana(const spells::Mode mode, const spells::Spell * spell, const spells::PacketSender * server, const int spellCost) const
{
	if(mode == spells::Mode::CREATURE_ACTIVE || mode == spells::Mode::ENCHANTER)
	{
		if(spellCost != 1)
			logGlobal->warn("Unexpected spell cost for creature %d", spellCost);

		BattleSetStackProperty ssp;
		ssp.stackID = ID;
		ssp.which = BattleSetStackProperty::CASTS;
		ssp.val = -spellCost;
		ssp.absolute = false;
		server->sendAndApply(&ssp);

		if(mode == spells::Mode::ENCHANTER)
		{
			auto bl = getBonuses(Selector::typeSubtype(Bonus::ENCHANTER, spell->getIndex()));

			int cooldown = 1;
			for(auto b : *(bl))
				if(b->additionalInfo > cooldown)
					cooldown = b->additionalInfo;

			BattleSetStackProperty ssp;
			ssp.which = BattleSetStackProperty::ENCHANTER_COUNTER;
			ssp.absolute = false;
			ssp.val = cooldown;
			ssp.stackID = ID;
			server->sendAndApply(&ssp);
		}
	}
}

const CCreature * CStack::creatureType() const
{
	return type;
}

int32_t CStack::unitBaseAmount() const
{
	return baseAmount;
}

bool CStack::unitHasAmmoCart(const battle::Unit * unit) const
{
	bool hasAmmoCart = false;

	for(const CStack * st : battle->stacks)
	{
		if(battle->battleMatchOwner(st, unit, true) && st->getCreature()->idNumber == CreatureID::AMMO_CART && st->alive())
		{
			hasAmmoCart = true;
			break;
		}
	}
	return hasAmmoCart;
}

PlayerColor CStack::unitEffectiveOwner(const battle::Unit * unit) const
{
	return battle->battleGetOwner(unit);
}

uint32_t CStack::unitId() const
{
	return ID;
}

ui8 CStack::unitSide() const
{
	return side;
}

PlayerColor CStack::unitOwner() const
{
	return owner;
}

SlotID CStack::unitSlot() const
{
	return slot;
}

std::string CStack::getDescription() const
{
	return nodeName();
}
