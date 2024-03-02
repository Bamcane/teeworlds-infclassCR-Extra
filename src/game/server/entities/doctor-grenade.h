#ifndef GAME_SERVER_ENTITIES_DOCTOR_GRENADE_H
#define GAME_SERVER_ENTITIES_DOCTOR_GRENADE_H

#include <game/server/entity.h>

class CDoctorGrenade : public CEntity
{
public:
    CDoctorGrenade(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir);

    vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

    int m_Owner;

    void Explode();
private:
	vec2 m_ActualPos;
	vec2 m_ActualDir;
	vec2 m_Direction;
	int m_StartTick;
	int m_LifeSpan;
	int m_BounceLeft;
	float m_DistanceLeft;
    int m_ExplosionTick;
};

#endif