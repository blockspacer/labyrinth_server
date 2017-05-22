//
//  monster.hpp
//  labyrinth_serv_xcode
//
//  Created by Aleksandr Borzikh on 15.04.17.
//  Copyright © 2017 hate-red. All rights reserved.
//

#ifndef monster_hpp
#define monster_hpp

#include "unit.hpp"

#include <queue>

class Monster : public Unit
{
public:
    Monster();
    
    virtual void    update(std::chrono::milliseconds) override;
    
    virtual void    Spawn(Point2) override;
protected:
    std::chrono::milliseconds m_msMoveCD;;
    std::chrono::milliseconds m_msMoveACD;
    
    std::chrono::milliseconds m_msAtkCD;;
    std::chrono::milliseconds m_msAtkACD;
    
    Unit * m_pChasingUnit;
    std::queue<Point2> m_pPathToUnit;
};

#endif /* monster_hpp */
