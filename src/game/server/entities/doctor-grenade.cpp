#include <base/math.h>
#include <base/vmath.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include "growingexplosion.h"

#include "doctor-grenade.h"

CDoctorGrenade::CDoctorGrenade(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir) : CEntity(pGameWorld, CGameWorld::ENTTYPE_DOCTOR_GRENADE)
{
    m_Pos = Pos;
    m_ActualPos = Pos;
    m_ActualDir = Dir;
    m_Direction = Dir;
    m_Owner = Owner;
    m_LifeSpan = g_Config.m_InfDoctorGrenadeLifeSpan * Server()->TickSpeed();
    m_StartTick = Server()->Tick();
    m_ExplosionTick = 0;

    GameWorld()->InsertEntity(this);
}

void CDoctorGrenade::Reset()
{
    GameServer()->m_World.DestroyEntity(this);
}

vec2 CDoctorGrenade::GetPos(float Time)
{
    float Curvature = 0;
    float Speed = GameServer()->Tuning()->m_GrenadeSpeed;

    return CalcPos(m_Pos, m_Direction, Curvature, Speed, Time);
}

void CDoctorGrenade::TickPaused()
{
    m_StartTick++;
}

void CDoctorGrenade::Tick()
{
    if (!GameServer()->GetPlayerChar(m_Owner) || GameServer()->GetPlayerChar(m_Owner)->IsZombie())
    {
        GameServer()->m_World.DestroyEntity(this);
    }

    float Pt = (Server()->Tick() - m_StartTick - 1) / (float)Server()->TickSpeed();
    float Ct = (Server()->Tick() - m_StartTick) / (float)Server()->TickSpeed();
    vec2 PrevPos = GetPos(Pt);
    vec2 CurPos = GetPos(Ct);

    if (m_ExplosionTick)
    {
        m_ExplosionTick--;
        if (m_ExplosionTick % 2 == 0)
            GameServer()->CreateSound(m_ActualPos, SOUND_HOOK_NOATTACH);
        if (m_ExplosionTick <= 0)
        {
            new CGrowingExplosion(GameWorld(), m_ActualPos, vec2(0, 0), m_Owner, 7.f, GROWINGEXPLOSIONEFFECT_BOOM_INFECTED, true);
            Reset();
        }
        return;
    }

    m_ActualPos = CurPos;
    m_ActualDir = normalize(CurPos - PrevPos);
    m_Direction = m_ActualDir;

    m_LifeSpan--;

    for (CCharacter *pChr = (CCharacter *)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChr; pChr = (CCharacter *)pChr->TypeNext())
    {
        if (pChr->IsHuman())
            continue;
        float Len = distance(pChr->m_Pos, m_ActualPos);
        if (Len < pChr->m_ProximityRadius)
            Explode();
    }

    if (GameLayerClipped(CurPos))
    {
        GameServer()->m_World.DestroyEntity(this);
        return;
    }

    vec2 LastPos;
    bool Collide = GameServer()->Collision()->CheckPoint(CurPos);
    if (Collide || m_LifeSpan <= 0)
        Explode();
}

void CDoctorGrenade::Snap(int SnappingClient)
{
    float Ct = (Server()->Tick() - m_StartTick) / (float)Server()->TickSpeed();

    if (IsDontSnapEntity(SnappingClient, GetPos(Ct)))
        return;

    CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
    pProj->m_X = (int)m_ActualPos.x;
    pProj->m_Y = (int)m_ActualPos.y;
    pProj->m_VelX = 0;
    pProj->m_VelY = 0;
    pProj->m_StartTick = Server()->Tick();
    pProj->m_Type = WEAPON_GRENADE;
}

void CDoctorGrenade::Explode()
{
    m_ExplosionTick = 25;
    GameServer()->CreateExplosion(m_ActualPos, m_Owner, WEAPON_GRENADE, false);
}