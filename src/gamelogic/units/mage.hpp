//
//  mage.hpp
//  labyrinth_serv_xcode
//
//  Created by Aleksandr Borzikh on 15.04.17.
//  Copyright © 2017 hate-red. All rights reserved.
//

#ifndef mage_hpp
#define mage_hpp

#include "hero.hpp"

#include <string>
#include <vector>
#include <chrono>

class Mage : public Hero
{
public:
    Mage();
    
    virtual void    update(std::chrono::milliseconds) override;
    
    virtual void    SpellCast(const GameEvent::CLActionSpell*) override;
};

#endif /* mage_hpp */
