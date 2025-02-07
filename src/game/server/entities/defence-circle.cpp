/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include <base/math.h>
#include <base/vmath.h>

#include "defence-circle.h"

#include "growingexplosion.h"

CDefenceCircle::CDefenceCircle(CGameWorld *pGameWorld, vec2 Pos, int Owner)
	: CEntity(pGameWorld, CGameWorld::ENTTYPE_DEFENCE_CIRCLE)
{
	m_Pos = Pos;
	GameWorld()->InsertEntity(this);
	m_StartTick = Server()->Tick();
	m_Owner = Owner;

	for(int i = 0; i < NUM_PIECES; i++)
		m_PiecesIDs[i] = Server()->SnapNewID();
}

CDefenceCircle::~CDefenceCircle()
{
	for(int i = 0; i < NUM_PIECES; i++)
		Server()->SnapFreeID(m_PiecesIDs[i]);
}

void CDefenceCircle::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

int CDefenceCircle::GetOwner() const
{
	return m_Owner;
}

void CDefenceCircle::Explode(vec2 Target)
{
	float Radius = g_Config.m_InfDefenceCircleRadius;
	GameServer()->CreateExplosionDisk(Target, Radius/4.f, Radius, g_Config.m_InfDefenceCircleDamage,
									  32.0f, m_Owner, WEAPON_HAMMER, TAKEDAMAGEMODE_SELFHARM);

	if (GameServer()->GetPlayerChar(m_Owner))
		GameServer()->GetPlayerChar(m_Owner)->m_HasDefenceCircle = false;

	GameServer()->m_World.DestroyEntity(this);
}

void CDefenceCircle::Snap(int SnappingClient)
{
	if (IsDontSnapEntity(SnappingClient))
		return;

	float Radius = g_Config.m_InfDefenceCircleRadius;
	float time = (Server()->Tick()-m_StartTick)/(float)Server()->TickSpeed();
	float angle = fmodf(time*pi/2, 2.0f*pi);

	for(int i = 0; i < NUM_PIECES; i++)
	{	
		float shiftedAngle = angle + 2.0*pi*static_cast<float>(i)/static_cast<float>(NUM_PIECES);
		
		CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_PiecesIDs[i], sizeof(CNetObj_Projectile)));
		
		if(!pObj)
			continue;
		
		pObj->m_X = (int)(m_Pos.x + Radius * cos(shiftedAngle));
		pObj->m_Y = (int)(m_Pos.y + Radius * sin(shiftedAngle));
		pObj->m_VelX = 0;
		pObj->m_VelY = 0;
		pObj->m_StartTick = Server()->Tick();
	}
}

void CDefenceCircle::Tick()
{
	for (CCharacter *p = (CCharacter *)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
	{
		if(p->IsHuman())
			continue;
		if(p->GetClass() == PLAYERCLASS_UNDEAD && p->IsFrozen())
			continue;
		if(p->GetClass() == PLAYERCLASS_VOODOO && p->m_VoodooAboutToDie)
			continue;

		float Len = distance(p->m_Pos, m_Pos);
		if(Len < p->m_ProximityRadius + g_Config.m_InfDefenceCircleRadius)
		{
			Explode();
			return;
		}
	}
}

void CDefenceCircle::TickPaused()
{
	++m_StartTick;
}
