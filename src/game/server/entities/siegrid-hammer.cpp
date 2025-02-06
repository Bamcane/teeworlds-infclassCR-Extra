#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/gamecore.h>

#include "growingexplosion.h"
#include "siegrid-hammer.h"

const float dt = 0.01f;

CSiegridHammer::CSiegridHammer(CGameWorld *pGameWorld, int Owner, vec2 Pos)
    : CEntity(pGameWorld, CGameWorld::ENTTYPE_SIEGRID_HAMMER)
{
    m_Owner = Owner;
    m_Anchor = Pos;
    m_Pos = Pos;
    m_K = 60.f;
    m_Length = g_Config.m_InfSiegridHammerLength;
    m_Mass = 1.f;
    m_ProximityRadius = 28.f;
    mem_zero(m_aHammerHitObjects, sizeof(m_aHammerHitObjects));

    for(int i = 0; i < NumIDs; i++)
        m_aIDs[i] = Server()->SnapNewID();

    GameWorld()->InsertEntity(this);
}

CSiegridHammer::~CSiegridHammer()
{
    for (int i = 0; i < NumIDs; i++)
        Server()->SnapFreeID(m_aIDs[i]);
}

void CSiegridHammer::Tick()
{
    if(m_MarkedForDestroy)
        return;

    CCharacter *aEnts[MAX_CLIENTS];
    float Radius = 128.f;
    int Num = GameServer()->m_World.FindEntities(m_Pos, Radius, (CEntity **) aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
    static const int MaxDamage = 14;
    int Damage = clamp(round_to_int(length(m_Vel * dt) / 32.f * MaxDamage), 5, MaxDamage);

    for(int i = 0; i < Num; ++i)
    {
        if(aEnts[i]->IsHuman())
            continue;

        // make sure we haven't Hit this object before
        int Index = -1;
        bool bAlreadyHit = false;
        for(int j = 0; j < 10; j++)
        {
            if(Index == -1 && Server()->Tick() - m_aHammerHitObjects[j].m_LastHitTick > 10)
                Index = j;

            if(m_aHammerHitObjects[j].m_pEntity == aEnts[i] && Server()->Tick() - m_aHammerHitObjects[j].m_LastHitTick < 15)
                bAlreadyHit = true;
        }
        if(bAlreadyHit || Index == -1)
            continue;

        // check so we are sufficiently close
        if(distance(aEnts[i]->m_Pos, m_Pos) > 64.f)
            continue;

        // Hit a player, give him damage and stuffs...
        GameServer()->CreateSound(aEnts[i]->m_Pos, SOUND_NINJA_HIT);
        // set his velocity to fast upward (for now)
        m_aHammerHitObjects[Index].m_pEntity = aEnts[i];
        m_aHammerHitObjects[Index].m_LastHitTick = Server()->Tick();

        aEnts[i]->TakeDamage(normalize(aEnts[i]->m_Pos - m_Pos) * Damage, Damage, m_Owner, WEAPON_HAMMER, TAKEDAMAGEMODE_NOINFECTION);
    }

    SetAnchor(GameServer()->GetPlayerChar(m_Owner)->m_Pos);

    vec2 GravityForce = vec2(0.0f, m_Mass * 981.f);
    vec2 RopeForce = CalculateRopeForce(m_Pos, m_Anchor, m_K, m_Length);
    vec2 TotalForce = vec2(GravityForce.x + RopeForce.x, GravityForce.y + RopeForce.y);
    vec2 Accel = TotalForce / m_Mass;
    m_Vel += Accel * dt;

    m_Vel.y += GameWorld()->m_Core.m_Tuning.m_Gravity * 12.f;

    bool CollisionBefore = false;
	if(GameServer()->Collision()->TestBox(m_Pos + m_Vel * dt, vec2(m_ProximityRadius, m_ProximityRadius)))
		CollisionBefore = true;

    vec2 InoutVel = m_Vel * dt;
    GameServer()->Collision()->MoveBox(&m_Pos, &InoutVel, vec2(m_ProximityRadius, m_ProximityRadius), 0.1f);

    if(!CollisionBefore)
    {
        bool Collision = false;
        if(GameServer()->Collision()->TestBox(m_Pos + m_Vel * dt, vec2(m_ProximityRadius, m_ProximityRadius)))
            Collision = true;
        if(Collision && length(m_Vel * dt) > 24.f)
        {
            GameServer()->CreateHammerHit(m_Pos);
            new CGrowingExplosion(GameWorld(), m_Pos, -normalize(m_Vel * dt), m_Owner, 2, GROWINGEXPLOSIONEFFECT_FREEZE_INFECTED);
        }
    }

    m_Vel = InoutVel / dt;
}

vec2 CSiegridHammer::CalculateRopeForce(const vec2 Pos, const vec2 AnchorPos, float K, float RestLength)
{
    vec2 Displacement = vec2(Pos.x - AnchorPos.x, Pos.y - AnchorPos.y);
    float Dist = sqrt(Displacement.x * Displacement.x + Displacement.y * Displacement.y);

    if (Dist < m_Length - 48.f)
        return Pos - AnchorPos;

    vec2 MaxLength = vec2(RestLength - Dist, RestLength - Dist);
    vec2 ForceDirection = vec2(Displacement.x / Dist, Displacement.y / Dist);
    vec2 Force = MaxLength * ForceDirection;
    return Force * K;
}

void CSiegridHammer::SetAnchor(vec2 Pos)
{
    m_Anchor = Pos;
}

void CSiegridHammer::GiveForce(vec2 Force)
{
    m_Vel += Force * m_K;
}

void CSiegridHammer::Snap(int SnappingClient)
{
    if (NetworkClipped(SnappingClient, m_Anchor) && NetworkClipped(SnappingClient, m_Pos))
        return;

    for (int i = 0; i < NumIDs; i++)
    {
        float shiftedAngle = GetAngle(normalize(m_Anchor - m_Pos)) + 2.0 * pi * static_cast<float>(i) / static_cast<float>(NumIDs);
        float shiftedAngleNext = GetAngle(normalize(m_Anchor - m_Pos)) + 2.0 * pi * static_cast<float>(i + 1) / static_cast<float>(NumIDs);

        CNetObj_Laser *pProj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aIDs[i], sizeof(CNetObj_Laser)));
        pProj->m_X = (int) (m_Pos.x + 64.f * cos(shiftedAngle));
        pProj->m_Y = (int) (m_Pos.y + 64.f * sin(shiftedAngle));
        pProj->m_FromX = (int) (m_Pos.x + 64.f * cos(shiftedAngleNext));
        pProj->m_FromY = (int) (m_Pos.y + 64.f * sin(shiftedAngleNext));
        pProj->m_StartTick = Server()->Tick();
    }

    CNetObj_Laser *pLaser = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));
    if (!pLaser)
        return;

    pLaser->m_X = (int)m_Pos.x;
    pLaser->m_Y = (int)m_Pos.y;
    pLaser->m_FromX = (int)m_Anchor.x;
    pLaser->m_FromY = (int)m_Anchor.y;
    pLaser->m_StartTick = Server()->Tick();
}