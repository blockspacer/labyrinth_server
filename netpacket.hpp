//
//  netpacket.hpp
//  labyrinth_server
//
//  Created by Aleksandr Borzikh on 01.02.17.
//  Copyright © 2017 sandyre. All rights reserved.
//

#ifndef netpacket_hpp
#define netpacket_hpp

#include "construction.hpp"
#include "item.hpp"

    //typedefs
using PlayerUID = int32_t;
using ItemUID   = int16_t;
using MonsterUID = int16_t;

struct MSPacket
{
    enum class Type : unsigned char
    {
        CL_PING,
        MS_PING,
        
        CL_FIND_GAME,
        MS_GAME_FOUND
    };
    
    MSPacket::Type  eType;
    PlayerUID       nUID;
    char            aData[50];
};

namespace MSPackets
{
    
struct CLFindGame
{
        // params
};

struct MSGameFound
{
    uint32_t    nGSPort;
};
    
}

struct GamePacket
{
    enum class Type : unsigned char
    {
        CL_MOVEMENT,
        SRV_MOVEMENT,
        
        CL_TAKE_EQUIP,
        SRV_TOOK_EQUIP,
        
        CL_ATTACK_PLAYER,
        SRV_ATTACK_PLAYER,
        
        CL_ATTACK_MONSTER,
        SRV_ATTACK_MONSTER,
        
        CL_CONNECT,
        SRV_PL_SET_UID,
        SRV_GEN_MAP,
        SRV_SPAWN_PLAYER,
        SRV_SPAWN_MONSTER,
        SRV_SPAWN_ITEM,
        SRV_SPAWN_CONSTRUCTION
    };
    
    GamePacket::Type    eType;
    PlayerUID           nUID;
    char                aData[50];
};

namespace GamePackets
{

struct CLMovement
{
    uint16_t    nXCoord;
    uint16_t    nYCoord;
};

struct SRVMovement
{
    PlayerUID   nPlayerUID;
    uint16_t    nXCoord;
    uint16_t    nYCoord;
};
    
struct CLTakeItem
{
    ItemUID     nItemUID;
};
    
struct SRVTakeItem
{
    PlayerUID   nPlayerUID;
    ItemUID     nItemUID;
};

struct CLAttackMonster
{
    MonsterUID   nMonsterUID;
};
    
struct SRVMonsterAttack
{
    PlayerUID    nPlayerUID;
    MonsterUID   nMonsterUID;
};
    
struct CLPlayerAttack
{
    PlayerUID   nPlayerUID;
};

struct SRVPlayerAttack
{
    PlayerUID    nAttackerID;
    PlayerUID    nAttackedID;
};
    
struct SRVSpawnItem
{
    Item::Type  eType;
    ItemUID     nItemUID;
    uint16_t    nXCoord;
    uint16_t    nYCoord;
};
    
struct SRVSpawnConstruction
{
    Construction::Type  eType;
    uint16_t            nXCoord;
    uint16_t            nYCoord;
};
    
struct SRVSpawnPlayer
{
    PlayerUID nPlayerUID;
    int16_t  nXCoord;
    int16_t  nYCoord;
};
    
struct SRVSpawnMonster
{
    MonsterUID nMonsterUID;
    int16_t  nXCoord;
    int16_t  nYCoord;
};
    
struct SRVGenMap
{
    uint16_t    nChunkN;
    uint16_t    nChunkW;
    uint16_t    nChunkH;
    uint32_t    nSeed;
};
    
struct CLPlayerConnect
{
    uint32_t    nPlayerUID;
        // to be implemented
};
    
}

#endif /* netpacket_hpp */
