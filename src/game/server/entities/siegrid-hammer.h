#ifndef GAME_SERVER_ENTITIES_SIEGRID_HAMMER_H
#define GAME_SERVER_ENTITIES_SIEGRID_HAMMER_H

#include <game/server/entity.h>

const int NumIDs = 6;

// THANKS FOR FlowerFell-Sans (ST-Chara)
class CSiegridHammer : public CEntity
{
public:
    CSiegridHammer(CGameWorld *pGameWorld, int Owner, vec2 Pos);
    ~CSiegridHammer();

    virtual void Tick();
    virtual void Snap(int SnappingClient);

    void GiveForce(vec2 Force);
    void SetAnchor(vec2 Pos);

    vec2 CalculateRopeForce(const vec2 Pos, const vec2 AnchorPos, float K, float Length);

    int GetOwner() const { return m_Owner; }
    vec2 GetPos() const { return m_Pos; }
private:
	struct CHitEntity
	{
		int m_LastHitTick;
		CEntity *m_pEntity;
	}m_aHammerHitObjects[16];

    int m_Owner;
    int m_aIDs[NumIDs];
    float m_Mass;
    float m_K;
    float m_Length;
    vec2 m_Anchor;
    vec2 m_Vel;
};

#endif // GAME_SERVER_ENTITIES_SIEGRID_HAMMER_H
