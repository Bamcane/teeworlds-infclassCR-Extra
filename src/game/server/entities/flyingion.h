#ifndef GAME_SERVER_ENTITIES_FLYINGION_H
#define GAME_SERVER_ENTITIES_FLYINGION_H

#include <game/server/entity.h>

class CFlyingIon : public CEntity
{
private:
	int m_Owner;
	int m_Radius;
    int m_StartTick;
	vec2 m_Vel;
	
public:
	CFlyingIon(CGameWorld *pGameWorld, vec2 Pos, vec2 Vel, int Owner, int Radius);
	
    int GetOwner() { return m_Owner; }
	virtual void Tick();
};

#endif // GAME_SERVER_ENTITIES_FLYINGION_H
