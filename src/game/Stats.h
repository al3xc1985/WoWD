// Copyright (C) 2004 WoW Daemon
// Copyright (C) 2005 Oxide
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef __STATS_H
#define __STATS_H

#include "Unit.h"
#include "UpdateMask.h"
#include "ItemInterface.h"

enum Stats
{
    STAT_STRENGTH,
    STAT_AGILITY,
    STAT_STAMINA,
    STAT_INTELLECT,
    STAT_SPIRIT,
};

/*ZD = 5, when Char Level = 1 - 7
ZD = 6, when Char Level = 8 - 9
ZD = 7, when Char Level = 10 - 11
ZD = 8, when Char Level = 12 - 14
ZD = 9, when Char Level = 16 - 19 (15-19?)
ZD = 11, when Char Level = 20 - 29
ZD = 12, when Char Level = 30 - 39
ZD = 13, when Char Level = 40 - 44
ZD = 14, when Char Level = 45 - 49
ZD = 15, when Char Level = 50 - 53 (50-54?)
ZD = 16, when Char Level = 55 - 59 (guessed) 
ZD = 17, when Char Level = 60 (guessed) */

/////////////////////////////////////////////////////////////////////////          30
//  CalculateXpToGive
//
//  Calculates XP given out by pVictim upon death.
//  XP=(MOB_LEVEL*5+45)*(1+0.05*(MOB_LEVEL-PLAYER_LEVEL)) 
//  from http://wowwow.game-host.org/viewtopic.php?t=857&sid=07e3a117e26e43358dd23cf260c0c7ad
//
//
//old formula xp = (SUM(health,power1,power2,power3,power4) / 5) * (lvl_of_monster*2)
////////////////////////////////////////////////////////////////////////
// http://www.wowwiki.com/Formulas:Mob_XP latest formula
////////////////////////////////////////////////////////////////////////
//    Old Formula
/*    CreatureInfo *victimI = pVictim->GetCreatureName();
if(victimI)
{
if(victimI->Type == CRITTER)
return 0;
}
if(pVictim->GetTypeId() == TYPEID_PLAYER)
return 0;
if(pAttacker->getLevel() >= 60)
return 0;
uint16 VictimLvl = pVictim->GetUInt32Value(UNIT_FIELD_LEVEL);
uint16 AttackerLvl = pAttacker->GetUInt32Value(UNIT_FIELD_LEVEL);
int xp = 0;
int greylvl = 0;
int ZD[61] = {1,5,5,5,5,5,5,5,6,6,7,7,8,8,8,9,9,9,9,9,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,14,14,14,14,14,15,15,15,15,15,16,16,16,16,16,17};    
if( VictimLvl > AttackerLvl )
xp = (AttackerLvl*5+45)*(1+0.05*(VictimLvl-AttackerLvl));
else if( VictimLvl == AttackerLvl )
xp = AttackerLvl*5+45;
else
{
if( AttackerLvl < 6 )
greylvl = 0;
else if( AttackerLvl > 5 && AttackerLvl < 40 )
greylvl = AttackerLvl-5-(AttackerLvl/10);
else
greylvl = AttackerLvl-1-(AttackerLvl/5);

if( VictimLvl > greylvl )
xp = (AttackerLvl*5+45)*(1-((AttackerLvl-VictimLvl)/ZD[AttackerLvl]));
else
xp = 0;
}

if( xp < 0 )
xp = 0;
else
xp *= sWorld.getRate(RATE_XP);

if(victimI->Rank > 1)  xp *= victimI->Rank; //Elite + Boss Extra XP

return xp;*/
//////////////////////////////////////////////////////////////////
// Mob XP Functions (including Con Colors)
// Colors will be numbers:
//  {grey = 0, green = 1, yellow = 2, orange = 3, red = 4, skull = 5}
// NOTE: skull = red when working with anything OTHER than mobs!

inline uint32 getConColor(uint16 AttackerLvl, uint16 VictimLvl)
{
    const uint32 grayLevel[PLAYER_LEVEL_CAP+1] = {0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,13,14,15,16,17,18,19,20,21,22,22,23,24,25,26,27,28,29,30,31,31,32,33,34,35,35,36,37,38,39,39,40,41,42,43,43,44,45,46,47,47,48,49,50,51,51,52,53,54,55,55};
    if(AttackerLvl + 5 <= VictimLvl) 
    {
        if(AttackerLvl + 10 <= VictimLvl) 
        {
            return 5;
        }
        return 4;
    }
    else 
    {
        switch(VictimLvl - AttackerLvl)
        {
        case 4:
        case 3:
            return 3;
            break;
        case 2:
        case 1:
        case 0:
        case -1:
        case -2:
            return 2;
            break;
        default:
            // More adv formula for grey/green lvls:
            if(AttackerLvl <= 6) 
            {
                return 1; //All others are green.
            }
            else 
            {
                if(VictimLvl <= grayLevel[AttackerLvl])
                    return 0;
                else
                    return 1;
            }
        }
    }
}

inline uint32 CalculateXpToGive(Unit *pVictim, Unit *pAttacker)
{
    CreatureInfo *victimI = pVictim->GetCreatureName();
    if(victimI)
    {
        if(victimI->Type == CRITTER)
            return 0;
    }
    if(pVictim->GetTypeId() == TYPEID_PLAYER)
        return 0;
    if(pAttacker->getLevel() >= PLAYER_LEVEL_CAP)
        return 0;
    uint16 VictimLvl = pVictim->GetUInt32Value(UNIT_FIELD_LEVEL);
    uint16 AttackerLvl = pAttacker->GetUInt32Value(UNIT_FIELD_LEVEL);

/*    float zd = 5;
    float g = 5;

    // get zero diff
    if(AttackerLvl >= 70)
        zd = 19;
    else if(AttackerLvl >= 65)
        zd = 18;
    else if(AttackerLvl >= 60)
        zd = 17;
    else if(AttackerLvl >= 55)
        zd = 16;
    else if(AttackerLvl >= 50)
        zd = 15;
    else if(AttackerLvl >= 45)
        zd = 14;
    else if(AttackerLvl >= 40)
        zd = 13;
    else if(AttackerLvl >= 30)
        zd = 12;
    else if(AttackerLvl >= 20)
        zd = 11;
    else if(AttackerLvl >= 16)
        zd = 9;
    else if(AttackerLvl >= 12)
        zd = 8;
    else if(AttackerLvl >= 10)
        zd = 7;
    else if(AttackerLvl >= 8)
        zd = 6;
    else
        zd = 5;

    // get grey diff

    if(AttackerLvl >= 70)
        g = 15;
    else if(AttackerLvl >= 65)
        g = 14;
    else if(AttackerLvl >= 60)
        g = 13;
    else if(AttackerLvl >= 55)
        g = 12;
    else if(AttackerLvl >= 50)
        g = 11;
    else if(AttackerLvl >= 45)
        g = 10;
    else if(AttackerLvl >= 40)
        g = 9;
    else if(AttackerLvl >= 30)
        g = 8;
    else if(AttackerLvl >= 20)
        g = 7;
    else if(AttackerLvl >= 10)
        g = 6;
    else
        g = 5;

    float xp = 0.0f;
    float fVictim = VictimLvl;
    float fAttacker = AttackerLvl;

    if(VictimLvl == AttackerLvl)
        xp = float( ((fVictim * 5.0f) + 45.0f) );
    else if(VictimLvl > AttackerLvl)
    {
        float j = 1.0f + (0.25f * (fVictim - fAttacker));
        xp = float( ((AttackerLvl * 5.0f) + 45.0f) * j );
    }
    else
    {
        if((AttackerLvl - VictimLvl) < g)
        {
            float j = (1.0f - float((fAttacker - fVictim) / zd));
            xp = (AttackerLvl * 5.0f + 45.0f) * j;
        }
    }

    // multiply by global XP rate
    if(xp == 0.0f)
        return 0;

    xp *= sWorld.getRate(RATE_XP);

    // elite boss multiplier
    if(victimI)
    {
        switch(victimI->Rank)
        {
        case 0: // normal mob
            break;
        case 1: // elite
            xp *= 1.5f;
            break;
        case 2: // rare elite
            xp *= 3.0f;
            break;
        case 3: // world boss
            xp *= 10.0f;
            break;
        default:    // rare or higher
            xp *= 7.0f;
            break;
        }
    }
    return (uint32)xp;*/
    const float ZD[PLAYER_LEVEL_CAP+1] = {1,5,5,5,5,5,5,5,6,6,7,7,8,8,8,9,9,9,9,9,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,14,14,14,14,14,15,15,15,15,15,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17};
    float temp = 0;
    float tempcap = 0;
    float xp = 0;

    if(VictimLvl >= AttackerLvl) 
    {
        temp = ((AttackerLvl * 5) + 45) * (1 + 0.05 * (VictimLvl - AttackerLvl));
        tempcap = ((AttackerLvl * 5) + 45) * 1.2;
        if(temp > tempcap)
        {
            if( tempcap < 0 )
                tempcap = 0;
            else
                tempcap *= sWorld.getRate(RATE_XP);

            xp = tempcap;
        }
        else 
        {
            if( temp < 0 )
                temp = 0;
            else
                temp *= sWorld.getRate(RATE_XP); 

            xp = temp;
        }
    }
    else 
    {
        if(getConColor(AttackerLvl, VictimLvl) == 0) 
        {
            return (uint32)0;
        }
        else 
        {
            temp = (((AttackerLvl * 5) + 45) * (1 - (AttackerLvl - VictimLvl)/ZD[AttackerLvl]));
            if( temp < 0 )
                temp = 0;
            else
                temp *= sWorld.getRate(RATE_XP);

            xp = temp;
        }
    }

    if(victimI)
    {
        switch(victimI->Rank)
        {
        case 0: // normal mob
            break;
        case 1: // elite
            xp *= 1.5f;
            break;
        case 2: // rare elite
            xp *= 3.0f;
            break;
        case 3: // world boss
            xp *= 10.0f;
            break;
        default:    // rare or higher
            xp *= 7.0f;
            break;
        }
    }

    return static_cast<uint32>(xp);
}

//Taken from WoWWoW Source
/*
Author: pionere

Calculate the stat increase. Using 3rd grade polynome.

Parameter level The level the character reached.
Parameter a3 The factor for x^3.
Parameter a2 The factor for x^2.
Parameter a1 The factor for x^1.
Parameter a0 The constant factor for the polynome.
Return stat gain
*/
inline uint32 CalculateStat(uint16 level,double a3, double a2, double a1, double a0)
{
    int result1;
    int result2;
    int diff;

    result1 =    (int)(a3*level*level*level +
        a2*level*level +
        a1*level +
        a0);

    result2 =    (int)(a3*(level-1)*(level-1)*(level-1) +
        a2*(level-1)*(level-1) +
        a1*(level-1) +
        a0);

    //get diffrence
    diff = result1-result2;
    return diff;
}
//Partialy taken from WoWWoW Source
inline uint32 GainStat(uint16 level, uint8 playerclass,uint8 Stat)
{
    uint32 gain = 0;
    switch(playerclass)
    {
    case WARRIOR:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000039, 0.006902, 1.080040, -1.051701); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000022, 0.004600, 0.655333, -0.600356); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000059, 0.004044, 1.040000, -1.488504); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000002, 0.001003, 0.100890, -0.076055); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000006, 0.002031, 0.278360, -0.340077); }break;
            }

        }break;

    case PALADIN:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000037, 0.005455, 0.940039, -1.000090); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000020, 0.003007, 0.505215, -0.500642); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000038, 0.005145, 0.871006, -0.832029); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000023, 0.003345, 0.560050, -0.562058); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000032, 0.003025, 0.615890, -0.640307); }break;
            }
        }break;

    case HUNTER:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000022, 0.001800, 0.407867, -0.550889); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000040, 0.007416, 1.125108, -1.003045); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000031, 0.004480, 0.780040, -0.800471); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000020, 0.003007, 0.505215, -0.500642); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000017, 0.003803, 0.536846, -0.490026); }break;
            }
        }break;

    case ROGUE:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000025, 0.004170, 0.654096, -0.601491); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000038, 0.007834, 1.191028, -1.203940); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000032, 0.003025, 0.615890, -0.640307); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000008, 0.001001, 0.163190, -0.064280); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000024, 0.000981, 0.364935, -0.570900); }break;
            }
        }break;

    case PRIEST:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000008, 0.001001, 0.163190, -0.064280); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000022, 0.000022, 0.260756, -0.494000); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000024, 0.000981, 0.364935, -0.570900); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000039, 0.006981, 1.090090, -1.006070); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000040, 0.007416, 1.125108, -1.003045); }break;
            }
        }break;

    case SHAMAN:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000035, 0.003641, 0.734310, -0.800626); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000022, 0.001800, 0.407867, -0.550889); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000020, 0.006030, 0.809570, -0.809220); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000031, 0.004480, 0.780040, -0.800471); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000038, 0.005145, 0.871006, -0.832029); }break;
            }
        }break;

    case MAGE:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000002, 0.001003, 0.100890, -0.076055); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000008, 0.001001, 0.163190, -0.064280); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000006, 0.002031, 0.278360, -0.340077); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000040, 0.007416, 1.125108, -1.003045); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000039, 0.006981, 1.090090, -1.006070); }break;
            }
        }break;

    case WARLOCK:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000006, 0.002031, 0.278360, -0.340077); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000024, 0.000981, 0.364935, -0.570900); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000021, 0.003009, 0.486493, -0.400003); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000059, 0.004044, 1.040000, -1.488504); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000040, 0.006404, 1.038791, -1.039076); }break;
            }
        }break;

    case DRUID:
        {
            switch(Stat)
            {
            case STAT_STRENGTH:  { gain = CalculateStat(level, 0.000021, 0.003009, 0.486493, -0.400003); }break;
            case STAT_AGILITY:   { gain = CalculateStat(level, 0.000041, 0.000440, 0.512076, -1.000317); }break;
            case STAT_STAMINA:   { gain = CalculateStat(level, 0.000023, 0.003345, 0.560050, -0.562058); }break;
            case STAT_INTELLECT: { gain = CalculateStat(level, 0.000038, 0.005145, 0.871006, -0.832029); }break;
            case STAT_SPIRIT:    { gain = CalculateStat(level, 0.000059, 0.004044, 1.040000, -1.488504); }break;
            }
        }break;
    }
    return gain;
}

//TODO: Some awesome formula to determine how much damage to deal
//consider this is melee damage
//damage type =0 --melee, 1--dual wield, 2 - ranged
inline uint32 CalculateDamage(Unit *pAttacker, Unit *pVictim, uint32 damage_type, uint32 spellgroup)//spellid is used only for 2-3 spells, that have AP bonus
{
    //	Attack Power increases your base damage-per-second (DPS) by 1 for every 14 attack power. 
    //(c) wowwiki

    //type of this UNIT_FIELD_ATTACK_POWER_MODS is unknown, not even uint32 disabled for now.

    uint32 offset;
    Item *it = NULL;
    if(pAttacker->disarmed && pAttacker->IsPlayer())
    {
        offset=UNIT_FIELD_MINDAMAGE;
        it = static_cast<Player*>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    }
    else if(damage_type == MELEE)
        offset=UNIT_FIELD_MINDAMAGE;
    else if(damage_type == DUALWIELD)
        offset=UNIT_FIELD_MINOFFHANDDAMAGE;
    else
        offset=UNIT_FIELD_MINRANGEDDAMAGE;


    float min_damage = pAttacker->GetFloatValue(offset);
    float max_damage = pAttacker->GetFloatValue(offset+1);

    if(it)
    {
        min_damage -= it->GetProto()->DamageMin[0];
        max_damage -= it->GetProto()->DamageMax[0];
    }

    if(pVictim->GetCreatureName())
    {
        float bonus;
        int32 ap_bonus;
        if(offset == UNIT_FIELD_MINRANGEDDAMAGE)
        {
            ap_bonus = pAttacker->CreatureRangedAttackPowerMod[pVictim->GetCreatureName()->Type];
            if(ap_bonus != 0)
            {
                if(pAttacker->IsPlayer())
                {
                    uint32 speed;
                    if(!pAttacker->disarmed)
                    {
                        Item *it = static_cast<Player*>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                        if(it)
                            speed = it->GetProto()->Delay;
                        else 
                            speed = 2000;
                    }
                    else
                        speed = pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME);

                    bonus = (ap_bonus * (int32)speed) / 14000.0f;               
                }
                else
                {
                    bonus = (ap_bonus * (int32)pAttacker->GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME)) / 14000.0f;
                }

                min_damage += bonus;
                max_damage += bonus;
            }
        }
        else
        {
            ap_bonus = pAttacker->CreatureAttackPowerMod[pVictim->GetCreatureName()->Type];
            if(ap_bonus != 0)
            {
                if(pAttacker->IsPlayer())
                {
                    uint32 speed;
                    if(!pAttacker->disarmed)
                    {
                        Item *it = static_cast<Player*>(pAttacker)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                        if(it)
                            speed = it->GetProto()->Delay;
                        else 
                            speed = 2000;
                    }
                    else
                        speed = pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME);

                    bonus = (ap_bonus * (int32)speed) / 14000.0f;               
                }
                else
                {
                    bonus = (ap_bonus * (int32)pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME)) / 14000.0f;
                }

                min_damage += bonus;
                max_damage += bonus;
            }
        }
    }

    // Ehh, sometimes min is bigger than max!?!?
    /*if (min_damage > max_damage)
    {
    float temp = max_damage;
    max_damage = min_damage;
    min_damage = temp;
    }*/
    if(spellgroup)
    {
        int32 apb=0;
        SM_FIValue(pAttacker->SM_PAPBonus,&apb,spellgroup);
        if(apb)
        {		
            int32 ap = pAttacker->GetUInt32Value(UNIT_FIELD_ATTACK_POWER)+
                (int32)pAttacker->GetUInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
            float b = (ap*apb*pAttacker->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME))/1400000.0;

            min_damage += b;
            max_damage += b;
        }
    }
    // Fix creatures that have no base attack damage.
    //this is shit, critter should not have attack damage!
    //if db is wrong we should fix db
    //if(max_damage==0)
    //  max_damage=5;
    float diff = fabs(max_damage - min_damage);

    float result = min_damage;
    if(diff >= 1)
        result += sRand.rand(diff);;

    if(result >= 0)
        return FL2UINT(result);
    return 0;
}

inline bool isEven (int num)
{
    if ((num%2)==0) 
    {
        return true;
    }
    return false;
}

#endif



