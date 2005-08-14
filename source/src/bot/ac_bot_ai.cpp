//
// C++ Implementation: ac_bot_ai
//
// Description: The AI part of the bot for ac is here(navigation, shooting etc)
//
//
// Author:  <rickhelmus@gmail.com>
//



#include "../cube.h"

#ifdef AC_CUBE

extern vector<server_entity> sents;
extern vector<dynent*> monsters;

// UNDONE
weaponinfo_s WeaponInfoTable[MAX_WEAPONS] =
{
     // KNIFE
     { TYPE_MELEE, 0.0f, 3.5f, 0.0f, 5.0f, 1 },
     // SEMI PISTOL
     { TYPE_NORMAL, 5.0f, 12.0f, 0.0f, 50.0f, 4 },
     // AUTO PISTOL
     { TYPE_AUTO, 10.0f, 20.0f, 0.0f, 100.0f, 10 },
     // SHOTGUN
     { TYPE_SHOTGUN, 15.0f, 25.0f, 10.0f, 70.0f, 3 },
     // SNIPER
     { TYPE_SNIPER, 12.0f, 25.0f, 0.0f, 100.0f, 4 },
     // SUBGUN
     { TYPE_AUTO, 12.0f, 25.0f, 0.0f, 100.0f, 4 },
     
     /* EDIT: Adrian Henke - those are not used yet
     // CARBINE
     { TYPE_AUTO, 12.0f, 25.0f, 0.0f, 100.0f, 4 },
     // SEMI RIFLE
     { TYPE_SNIPER, 12.0f, 25.0f, 0.0f, 100.0f, 4 },
     // AUTO RIFLE
     { TYPE_AUTO, 12.0f, 25.0f, 0.0f, 100.0f, 4 },
     */
     
     // GRENADE
     { TYPE_GRENADE, 12.0f, 25.0f, 0.0f, 100.0f, 4 }
};

// Code of CACBot - Start   

bool CACBot::ChoosePreferredWeapon()
{
     if (CBot::ChoosePreferredWeapon())
          return true;
     else
          return SelectGun(GUN_KNIFE);
}
     
entity *CACBot::SearchForEnts(bool bUseWPs, float flRange, float flMaxHeight)
{
     /* Entities are scored on the following things:
          - Visibility
          - For ammo: Need(ie has this bot much of this type or not)
          - distance
     */
     
     float flNearestDist = 9999, flDist;
     entity *pNewTargetEnt = NULL;
     waypoint_s *pWptNearBot = NULL, *pBestWpt = NULL;
     short sScore, sHighestScore = 0;
     
     if ((WaypointClass.m_iWaypointCount >= 1) && bUseWPs)
          pWptNearBot = GetNearestWaypoint(15.0f);

#ifdef WP_FLOOD                    
     if (!pWptNearBot && bUseWPs)
          pWptNearBot = GetNearestFloodWP(5.0f);
#endif
          
     loopv(ents)
     {
          sScore = 0;
          entity &e = ents[i];
          vec o = { e.x, e.y, S(e.x, e.y)->floor+player1->eyeheight };
     
          if (!ents[i].spawned) continue;
          if (OUTBORD(e.x, e.y)) continue;
          
          bool bInteresting = false;
          short sAmmo = 0, sMaxAmmo = itemstats[e.type-I_SEMIPISTOL].max;
          
          switch(e.type)
          {
          case I_SEMIPISTOL:
               bInteresting = (m_pMyEnt->ammo[1]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[1];
               break;
          case I_AUTOPISTOL:
               bInteresting = (m_pMyEnt->ammo[2]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[2];
               break;
          case I_SHOTGUN:
               bInteresting = (m_pMyEnt->ammo[3]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[3];
               break;
          case I_SNIPER:
               bInteresting = (m_pMyEnt->ammo[4]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[4];
               break;              
          case I_SUBGUN:
               bInteresting = (m_pMyEnt->ammo[5]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[4];
               break;              
          case I_CARBINE:
               bInteresting = (m_pMyEnt->ammo[6]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[4];
               break;              
          case I_SEMIRIFLE:
               bInteresting = (m_pMyEnt->ammo[7]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[4];
               break;              
          case I_AUTORIFLE:
               bInteresting = (m_pMyEnt->ammo[8]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[4];
               break;              
          case I_GRENADE:
               bInteresting = (m_pMyEnt->ammo[9]<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->ammo[4];
               break;                        
          case I_HELMET:
               bInteresting = ((m_pMyEnt->armour<itemstats[e.type-I_SEMIPISTOL].max) &&
                               (m_pMyEnt->armourtype!=A_YELLOW || m_pMyEnt->armour<=66));
               sAmmo = m_pMyEnt->armour;
               break;
          case I_ARMOUR:
               bInteresting = (m_pMyEnt->armour<itemstats[e.type-I_SEMIPISTOL].max);
               sAmmo = m_pMyEnt->armour;
               break;
          }
              
          if (!bInteresting)
              continue; // Not an interesting item, skip
          
          // Score on ammo and need
          if (sAmmo == -1)
          {
               // This entity doesn't have/need ammo
               // Score on type instead
               switch(e.type)
               { // UNDONE
               }
          }
          else
          {
               // Calculate current percentage of max ammo
               float percent = ((float)sAmmo / (float)sMaxAmmo) * 100.0f;
               if (percent > 100.0f) percent = 100.0f;
               sScore += ((100 - short(percent))/2);
          }
               
          flDist = GetDistance(o);
          
          if (flDist > flRange) continue;
          
          // Score on distance
          float f = flDist;
          if (f > 100.0f) f = 100.0f;
          sScore += ((100 - short(f)) / 2);
          
          waypoint_s *pWptNearEnt = NULL;
          // If this entity isn't visible check if there is a nearby waypoint
          if (!IsReachable(o, flMaxHeight))//(!IsVisible(o))
          {
               if (!pWptNearBot) continue;
               
#ifdef WP_FLOOD               
               if (pWptNearBot->pNode->iFlags & W_FL_FLOOD)
                    pWptNearEnt = GetNearestFloodWP(o, 8.0f);
               else
#endif               
                    pWptNearEnt = GetNearestWaypoint(o, 15.0f);
                    
               if (!pWptNearEnt) continue;                              
          }
                                    
          // Score on visibility
          if (pWptNearEnt == NULL) // Ent is visible
               sScore += 30;
          else
               sScore += 15;
               
          if (sScore > sHighestScore)
          {
               // Found a valid wp near the bot and the ent,so...lets store it :)
               if (pWptNearEnt)
                    pBestWpt = pWptNearEnt;
               else
                    pBestWpt = NULL; // Best ent so far doesn't need any waypoints
               
               sHighestScore = sScore;
               flNearestDist = flDist;
               pNewTargetEnt = &ents[i];
          }
     }
     
     if (pNewTargetEnt)
     {          
          // Need waypoints to reach it?
          if (pBestWpt)
          {
               ResetWaypointVars();
               SetCurrentWaypoint(pWptNearBot);
               SetCurrentGoalWaypoint(pBestWpt);
          }
               
          m_vGoal.x = pNewTargetEnt->x;
          m_vGoal.y = pNewTargetEnt->y;
          m_vGoal.z = S(pNewTargetEnt->x, pNewTargetEnt->y)->floor+player1->eyeheight;
     }
          
     return pNewTargetEnt;
}

bool CACBot::HeadToTargetEnt()
{     
     if (m_pTargetEnt)
     {
          vec o = { m_pTargetEnt->x, m_pTargetEnt->y,
                    S(m_pTargetEnt->x, m_pTargetEnt->y)->floor+m_pMyEnt->eyeheight };
          
          if (m_pTargetEnt->spawned && (!UnderWater(m_pMyEnt->o) ||
              !UnderWater(o)))
          {
               bool bIsVisible = false;
               if (m_pCurrentGoalWaypoint)
               {
                    if ((GetDistance(o) <= 20.0f) && IsReachable(o, 1.0f))
                         bIsVisible = true;
                    else if (HeadToGoal())
                    {
                         //debugbeam(m_pMyEnt->o, m_pCurrentWaypoint->pNode->v_origin);
                         //debugbeam(m_pMyEnt->o,
                         //            m_pCurrentGoalWaypoint->pNode->v_origin);
                         AddDebugText("Using WPs for ents");
                         return true;
                    }
               }
               else
                    bIsVisible = IsVisible(o);
                                   
               if (bIsVisible)
               {
                    if (m_pCurrentWaypoint || m_pCurrentGoalWaypoint)
                    {
                         condebug("ent is now visible");
                         ResetWaypointVars();
                    }
                    
                    float flHeightDiff = o.z - m_pMyEnt->o.z;
                    bool bToHigh = false;
                    if (Get2DDistance(o) <= 2.0f)
                    {
                         if (flHeightDiff >= 1.5f)
                         {
                              if (flHeightDiff <= JUMP_HEIGHT)
                              {
#ifndef RELEASE_BUILD                    
                                   char sz[64];
                                   sprintf(sz, "Ent z diff: %f", o.z-m_pMyEnt->o.z);
                                   condebug(sz);
#endif                         
                                   // Jump if close to ent and the ent is high
                                   m_pMyEnt->jumpnext = true;
                              }
                              else
                                   bToHigh = true;
                         }
                    }
                    
                    if (!bToHigh)
                    {
                         AimToVec(o);
                         return true;                              
                    }                                        
               }
          }
     }
     
     return false;
}
          
bool CACBot::DoSPStuff()
{
     return false;
}
    
// Code of CACBot - End

#endif
