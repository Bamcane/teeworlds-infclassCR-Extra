#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include "growingexplosion.h"
#include "flyingion.h"

CFlyingIon::CFlyingIon(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, int Owner, int Radius)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FLYINGION)
{
	m_Pos = Pos;
    m_Radius = Radius;
    m_Vel = Vel;
    m_StartTick = Server()->Tick();
    m_Owner = Owner;

	GameWorld()->InsertEntity(this);
}

void CFlyingIon::Tick()
{
    vec2 OldPos = m_Pos;
	m_Pos += m_Vel;

    if(GameServer()->Collision()->IntersectLine(OldPos, m_Pos, nullptr, nullptr) || m_StartTick + g_Config.m_InfFlyingIonLifespan < Server()->Tick())
        return GameWorld()->DestroyEntity(this);

    int Radius = g_Config.m_InfFlyingIonSlowlyGrow ? round_to_int(clamp(Server()->Tick() - m_StartTick, 0, 125) / ((float) 125) * m_Radius) : m_Radius;
    if(((Server()->Tick() - m_StartTick) % 5) == 0)
        new CGrowingExplosion(GameWorld(), m_Pos, normalize(m_Pos), m_Owner, Radius, GROWINGEXPLOSIONEFFECT_ELECTRIC_INFECTED);
}
