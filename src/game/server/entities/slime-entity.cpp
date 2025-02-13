/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "slime-entity.h"
#include "slug-slime.h"
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include <base/vmath.h>

#include "police-shield.h"
#include "growingexplosion.h"

CSlimeEntity::CSlimeEntity(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_SLIME_ENTITY)
{
	m_Pos = Pos;
	m_ActualPos = Pos;
	m_ActualDir = Dir;
	m_Direction = Dir;
	m_Owner = Owner;
	m_StartTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
}

void CSlimeEntity::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

vec2 CSlimeEntity::GetPos(float Time)
{
	float Curvature = GameServer()->Tuning()->m_GrenadeCurvature;
	float Speed = 3000.0f;

	return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}

void CSlimeEntity::TickPaused()
{
	m_StartTick++;
}

void CSlimeEntity::Tick()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);
	
	m_ActualPos = CurPos;
	m_ActualDir = normalize(CurPos - PrevPos);

	
	if(GameLayerClipped(CurPos))
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}
	
	if(random_prob(0.2f))
	{
		GameServer()->CreateDeath(m_ActualPos, m_Owner);
	}

    for(CPoliceShield *pShield = (CPoliceShield*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_POLICE_SHIELD); pShield; pShield = (CPoliceShield *)pShield->TypeNext())
	{
        for(int i = 1; i < CPoliceShield::NUM_IDS;i++)
	    {
		    float Len = distance(m_ActualPos, pShield->m_SnapIDsPos[i]);

		    if(Len < m_ProximityRadius + 16)
		    {
			    GameServer()->m_World.DestroyEntity(this);
				GameServer()->CreateExplosion(m_ActualPos, pShield->m_Owner, WEAPON_HAMMER, false, TAKEDAMAGEMODE_NOINFECTION);
                GameServer()->CreateSound(m_ActualPos, SOUND_GRENADE_EXPLODE);
			}
        }
	}
	
	int Collide = GameServer()->Collision()->IntersectLine(PrevPos, CurPos, NULL, &m_LastPos);
	if(Collide)
	{
		Explode();
	}
	
}

void CSlimeEntity::Collision()
{
	float Pt = (Server()->Tick()-m_StartTick-1)/(float)Server()->TickSpeed();
	float Ct = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	vec2 PrevPos = GetPos(Pt);
	vec2 CurPos = GetPos(Ct);

	//Thanks to TeeBall 0.6
	vec2 CollisionPos;
	CollisionPos.x = m_LastPos.x;
	CollisionPos.y = CurPos.y;
	int CollideY = GameServer()->Collision()->IntersectLine(PrevPos, CollisionPos, NULL, NULL);
	CollisionPos.x = CurPos.x;
	CollisionPos.y = m_LastPos.y;
	int CollideX = GameServer()->Collision()->IntersectLine(PrevPos, CollisionPos, NULL, NULL);
	
	m_Pos = m_LastPos;
	m_ActualPos = m_Pos;
	vec2 vel;
	vel.x = m_Direction.x;
	vel.y = m_Direction.y + 2*3.25f/10000*Ct*1500.0f;
	
	if (CollideX && !CollideY)
	{
		m_Direction.x = -vel.x;
		m_Direction.y = vel.y;
	}
	else if (!CollideX && CollideY)
	{
		m_Direction.x = vel.x;
		m_Direction.y = -vel.y;
	}
	else
	{
		m_Direction.x = -vel.x;
		m_Direction.y = -vel.y;
	}
	
	m_Direction.x *= (100 - 50) / 100.0;
	m_Direction.y *= (100 - 50) / 100.0;
	m_StartTick = Server()->Tick();
	
	m_ActualDir = normalize(m_Direction);
}
	
void CSlimeEntity::Explode()
{
	float t = (Server()->Tick()-m_StartTick-1.5)/(float)Server()->TickSpeed();
	new CSlugSlime(GameWorld(), m_LastPos, m_Owner);

	GameServer()->SendHitSound(m_Owner);
	
	GameServer()->m_World.DestroyEntity(this);
	
}
