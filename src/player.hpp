//
//  player.hpp
//  labyrinth_server
//
//  Created by Aleksandr Borzikh on 28.01.17.
//  Copyright © 2017 sandyre. All rights reserved.
//

#ifndef player_hpp
#define player_hpp

#include <Poco/Net/SocketAddress.h>
#include <vector>
#include "item.hpp"
#include "globals.h"

struct Player
{
    enum State : unsigned char
    {
        WALKING,
        DUEL,
        DROWNING,
        INVULNERABLE
    };
    
    State       eState;
    Point2      stPosition;
    uint32_t    nUID;
    char        sNickname[16];
    
    Poco::Net::SocketAddress sock_addr;
};

#endif /* player_hpp */
