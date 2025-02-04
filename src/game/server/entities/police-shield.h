#ifndef GAME_SERVER_ENTITIES_POLICE_WALL_H
#define GAME_SERVER_ENTITIES_POLICE_WALL_H

#include <game/server/entity.h>

class CPoliceShield : public CEntity
{
public:
	enum
	{
		NUM_IDS = 5,
	};
public:
	CPoliceShield(CGameWorld *pGameWorld, int Owner);
	virtual ~CPoliceShield();
	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	int m_Owner;
	int m_ExplodeTick;
	array<int> m_IDs;
	vec2 m_SnapIDsPos[NUM_IDS + 1];

	vec2 m_Direction;
	
private:
	float m_Radius;
	vec2 m_OwnerVel;
};

#endif
