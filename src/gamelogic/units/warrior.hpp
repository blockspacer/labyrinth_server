//
//  warrior.hpp
//  labyrinth_serv_xcode
//
//  Created by Aleksandr Borzikh on 15.04.17.
//  Copyright © 2017 hate-red. All rights reserved.
//

#ifndef warrior_hpp
#define warrior_hpp

#include "hero.hpp"

#include <string>
#include <vector>

class Warrior : public Hero
{
public:
    Warrior();
    
    virtual void    SpellCast(const GameEvent::CLActionSpell*) override;
};

#endif /* warrior_hpp */
