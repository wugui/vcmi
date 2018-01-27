/*
 * CUnitState.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "Unit.h"

class JsonSerializeFormat;
class UnitChanges;

namespace vstd
{
	class RNG;
}

namespace battle
{
class CUnitState;

class DLL_LINKAGE CTotalsProxy
{
public:
	CTotalsProxy(const IBonusBearer * Target, CSelector Selector, int InitialValue);
	CTotalsProxy(const CTotalsProxy & other);
	CTotalsProxy(CTotalsProxy && other) = delete;

	CTotalsProxy & operator=(const CTotalsProxy & other);
	CTotalsProxy & operator=(CTotalsProxy && other) = delete;

	int getMeleeValue() const;
	int getRangedValue() const;

private:
	const IBonusBearer * target;
	CSelector selector;
	int initialValue;

	mutable int64_t meleeCachedLast;
	mutable int meleeValue;

	mutable int64_t rangedCachedLast;
	mutable int rangedValue;
};

class DLL_LINKAGE CCheckProxy
{
public:
	CCheckProxy(const IBonusBearer * Target, CSelector Selector);
	CCheckProxy(const CCheckProxy & other);

	bool getHasBonus() const;

private:
	const IBonusBearer * target;
	CSelector selector;

	mutable int64_t cachedLast;
	mutable bool hasBonus;
};

class DLL_LINKAGE CAmmo
{
public:
	explicit CAmmo(const battle::Unit * Owner, CSelector totalSelector);

	//only copy construction is allowed for acquire(), serializeJson should be used for any other "assignment"
	CAmmo(const CAmmo & other);
	CAmmo(CAmmo && other) = delete;

	CAmmo & operator=(const CAmmo & other);
	CAmmo & operator=(CAmmo && other) = delete;

	int32_t available() const;
	bool canUse(int32_t amount = 1) const;
	virtual bool isLimited() const;
	virtual void reset();
	virtual int32_t total() const;
	virtual void use(int32_t amount = 1);

	virtual void serializeJson(JsonSerializeFormat & handler);
protected:
	int32_t used;
	const battle::Unit * owner;
	CBonusProxy totalProxy;
};

class DLL_LINKAGE CShots : public CAmmo
{
public:
	explicit CShots(const battle::Unit * Owner);
	CShots(const CShots & other);
	CShots & operator=(const CShots & other);
	bool isLimited() const override;
	int32_t total() const override;

	void setEnv(const IUnitEnvironment * env_);
private:
	const IUnitEnvironment * env;

	CCheckProxy shooter;
};

class DLL_LINKAGE CCasts : public CAmmo
{
public:
	explicit CCasts(const battle::Unit * Owner);
	CCasts(const CCasts & other);
	CCasts & operator=(const CCasts & other);
};

class DLL_LINKAGE CRetaliations : public CAmmo
{
public:
	explicit CRetaliations(const battle::Unit * Owner);
	CRetaliations(const CRetaliations & other);
	CRetaliations & operator=(const CRetaliations & other);
	bool isLimited() const override;
	int32_t total() const override;
	void reset() override;

	void serializeJson(JsonSerializeFormat & handler) override;
private:
	mutable int32_t totalCache;

	CCheckProxy noRetaliation;
	CCheckProxy unlimited;
};

class DLL_LINKAGE CHealth
{
public:
	explicit CHealth(const IUnitInfo * Owner);
	CHealth(const CHealth & other);

	CHealth & operator=(const CHealth & other);

	void init();
	void reset();

	void damage(int64_t & amount);
	void heal(int64_t & amount, EHealLevel level, EHealPower power);

	int32_t getCount() const;
	int32_t getFirstHPleft() const;
	int32_t getResurrected() const;

	int64_t available() const;
	int64_t total() const;

	void takeResurrected();

	void serializeJson(JsonSerializeFormat & handler);
private:
	void addResurrected(int32_t amount);
	void setFromTotal(const int64_t totalHealth);
	const IUnitInfo * owner;

	int32_t firstHPleft;
	int32_t fullUnits;
	int32_t resurrected;
};

class DLL_LINKAGE CUnitState : public Unit
{
public:
	bool cloned;
	bool defending;
	bool defendingAnim;
	bool drainedMana;
	bool fear;
	bool hadMorale;
	bool ghost;
	bool ghostPending;
	bool movedThisRound;
	bool summoned;
	bool waiting;

	CCasts casts;
	CRetaliations counterAttacks;
	CHealth health;
	CShots shots;

	CTotalsProxy totalAttacks;

	CTotalsProxy minDamage;
	CTotalsProxy maxDamage;

	///id of alive clone of this stack clone if any
	si32 cloneID;

	///position on battlefield; -2 - keep, -3 - lower tower, -4 - upper tower
	BattleHex position;

	CUnitState();

	CUnitState(const CUnitState & other) = delete;
	CUnitState(CUnitState && other) = delete;

	CUnitState & operator= (const CUnitState & other);
	CUnitState & operator= (CUnitState && other) = delete;

	ui8 getSpellSchoolLevel(const spells::Mode mode, const spells::Spell * spell, int * outSelectedSchool = nullptr) const override;
	int getEffectLevel(const spells::Mode mode, const spells::Spell * spell) const override;

	int64_t getSpellBonus(const spells::Spell * spell, int64_t base, const Unit * affectedStack) const override;
	int64_t getSpecificSpellBonus(const spells::Spell  * spell, int64_t base) const override;

	int getEffectPower(const spells::Mode mode, const spells::Spell * spell) const override;
	int getEnchantPower(const spells::Mode mode, const spells::Spell * spell) const override;
	int getEffectValue(const spells::Mode mode, const spells::Spell * spell) const override;

	const PlayerColor getOwner() const override;
	void getCasterName(MetaString & text) const override;
	void getCastDescription(const spells::Spell  * spell, MetaString & text) const override;
	void getCastDescription(const spells::Spell  * spell, const std::vector<const Unit *> & attacked, MetaString & text) const override;
	void spendMana(const spells::Mode mode, const spells::Spell  * spell, const spells::PacketSender * server, const int spellCost) const override;

	bool ableToRetaliate() const override;
	bool alive() const override;
	bool isGhost() const override;

	bool isClone() const override;
	bool hasClone() const override;

	bool isSummoned() const override;

	bool canCast() const override;
	bool isCaster() const override;
	bool canShoot() const override;
	bool isShooter() const override;

	int32_t getKilled() const override;
	int32_t getCount() const override;
	int32_t getFirstHPleft() const override;
	int64_t getAvailableHealth() const override;
	int64_t getTotalHealth() const override;

	BattleHex getPosition() const override;
	int32_t getInitiative(int turn = 0) const override;

	bool canMove(int turn = 0) const override;
	bool defended(int turn = 0) const override;
	bool moved(int turn = 0) const override;
	bool willMove(int turn = 0) const override;
	bool waited(int turn = 0) const override;

	int32_t unitMaxHealth() const override;

	std::shared_ptr<CUnitState> asquire() const override;

	int battleQueuePhase(int turn) const override;

	void damage(int64_t & amount);
	void heal(int64_t & amount, EHealLevel level, EHealPower power);

	void localInit(const IUnitEnvironment * env_);
	void serializeJson(JsonSerializeFormat & handler);

	void toInfo(UnitChanges & info);
	void fromInfo(const UnitChanges & info);

	int getMinDamage(bool ranged) const override;
	int getMaxDamage(bool ranged) const override;

	int getAttack(bool ranged) const override;
	int getDefence(bool ranged) const override;

	void afterAttack(bool ranged, bool counter);

	void afterNewRound();

	void afterGetsTurn();

	void makeGhost();

	void onRemoved();

private:
	const IUnitEnvironment * env;

	CTotalsProxy attack;
	CTotalsProxy defence;
	CBonusProxy inFrenzy;

	CCheckProxy cloneLifetimeMarker;

	void reset();
};

class DLL_LINKAGE CUnitStateDetached : public CUnitState
{
public:
	explicit CUnitStateDetached(const IUnitInfo * unit_, const IBonusBearer * bonus_);

	const TBonusListPtr getAllBonuses(const CSelector & selector, const CSelector & limit,
		const CBonusSystemNode * root = nullptr, const std::string & cachingStr = "") const override;

	int64_t getTreeVersion() const override;

	CUnitStateDetached & operator= (const CUnitState & other);

	uint32_t unitId() const override;
	ui8 unitSide() const override;

	const CCreature * creatureType() const override;
	PlayerColor unitOwner() const override;

	SlotID unitSlot() const override;


	int32_t unitBaseAmount() const override;

private:
	const IUnitInfo * unit;
	const IBonusBearer * bonus;
};

}
