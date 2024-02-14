/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "police-shield.h"
#include <engine/server/roundstatistics.h>
#include <engine/shared/config.h>

CPoliceShield::CPoliceShield(CGameWorld *pGameWorld, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_POLICE_SHIELD)
{
	m_Owner = Owner;
	m_ExplodeTick = 0;
    m_Radius = g_Config.m_InfPoliceShieldRadius;
	GameWorld()->InsertEntity(this);

    m_IDs.set_size(NUM_IDS);

    for(int i=0; i<NUM_IDS; i++)
	{
		m_IDs[i] = Server()->SnapNewID();
        m_SnapIDsPos[i] = vec2(0,0);
	}
	m_SnapIDsPos[NUM_IDS] = vec2(0,0);
}

CPoliceShield::~CPoliceShield()
{
	for(int i = 0; i < NUM_IDS; i++)
	{
		Server()->SnapFreeID(m_IDs[i]);
	}
}

void CPoliceShield::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CPoliceShield::Tick()
{
	if(!GameServer()->GetPlayerChar(m_Owner) || GameServer()->GetPlayerChar(m_Owner)->IsZombie())
	{
		GameServer()->m_World.DestroyEntity(this);
		return;
	}else
	{
		m_Direction = normalize(vec2(GameServer()->GetPlayerChar(m_Owner)->GetCore().m_Input.m_TargetX, GameServer()->GetPlayerChar(m_Owner)->GetCore().m_Input.m_TargetY));
		m_OwnerVel = GameServer()->GetPlayerChar(m_Owner)->GetCore().m_Vel;
	}

	int Degres = (int)(atan2f(m_Direction.y, m_Direction.x) * 180.0f / pi + 360) % 360 + 45;

	for(int i = 0; i < CPoliceShield::NUM_IDS;i++)
	{
		vec2 StartPos = m_Pos + (GetDir(Degres*pi/180) * m_Radius);
		Degres -= 90 / NUM_IDS;
		vec2 EndPos = m_Pos + (GetDir(Degres*pi/180) * m_Radius);

        m_SnapIDsPos[i] = vec2( (int) StartPos.x, (int) StartPos.y);
        m_SnapIDsPos[i + 1] = vec2( (int) EndPos.x, (int) EndPos.y);
	}

	if(m_ExplodeTick)
		m_ExplodeTick--;

    m_Pos = GameServer()->GetPlayerChar(m_Owner)->m_Pos;

    for(CCharacter *pChr = (CCharacter*) GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChr; pChr = (CCharacter *)pChr->TypeNext())
	{
		if(pChr->IsHuman()) 
			continue;

        for(int i=0;i < CPoliceShield::NUM_IDS + 1;i++)
	    {
		    float Len = distance(pChr->m_Pos, m_SnapIDsPos[i]);

		    if(Len < pChr->m_ProximityRadius + 2)
		    {
				if(GameServer()->GetPlayerChar(m_Owner))
				{
					switch (GameServer()->GetPlayerChar(m_Owner)->m_ShieldExplode)
					{
						case 1:
							if(!m_ExplodeTick)
							{
								GameServer()->CreateExplosionDisk(m_SnapIDsPos[i], 32.0f, 48.0f, g_Config.m_InfPoliceShieldDamage, 32.0f, m_Owner, WEAPON_HAMMER, TAKEDAMAGEMODE_NOINFECTION);
								m_ExplodeTick = g_Config.m_InfPoliceShieldExplodeTime;
							}
						case 0:
							if(abs(m_OwnerVel.x) > 4)
							{
								pChr->SetVel(vec2(m_OwnerVel.x - pChr->GetCore().m_Vel.x, pChr->GetCore().m_Vel.y));
							}

							if(abs(m_OwnerVel.y) > 4)
							{
								pChr->SetVel(vec2(pChr->GetCore().m_Vel.x, m_OwnerVel.y - pChr->GetCore().m_Vel.y));
							}
							break;
					}
					if(m_ExplodeTick)
						break;
				}
			}
        }
	}
}

void CPoliceShield::Snap(int SnappingClient)
{
	if(IsDontSnapEntity(SnappingClient))
		return;

	for(int i=0;i < CPoliceShield::NUM_IDS;i++)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDs[i], sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		pObj->m_FromX = (int)m_SnapIDsPos[i].x;
		pObj->m_FromY = (int)m_SnapIDsPos[i].y;
		pObj->m_X = (int)m_SnapIDsPos[i + 1].x;
		pObj->m_Y = (int)m_SnapIDsPos[i + 1].y;
		pObj->m_StartTick = Server()->Tick();
	}
}
