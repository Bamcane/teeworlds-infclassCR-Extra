/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_DEFENCE_CIRCLE_H
#define GAME_SERVER_ENTITIES_DEFENCE_CIRCLE_H

#include <game/server/entity.h>

class CDefenceCircle : public CEntity
{

public:
    enum
    {
        NUM_PIECES = 4,
    };

public:

	CDefenceCircle(CGameWorld *pGameWorld, vec2 Pos, int Owner);
	virtual ~CDefenceCircle();

	virtual void Snap(int SnappingClient);
	virtual void Reset();
	virtual void TickPaused();
	virtual void Tick();

	int GetOwner() const;
	void Explode();

private:
    int m_StartTick;
    int m_Owner;
    int m_PiecesIDs[NUM_PIECES];

};

#endif