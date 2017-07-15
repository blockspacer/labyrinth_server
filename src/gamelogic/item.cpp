//
//  item.cpp
//  labyrinth_serv_xcode
//
//  Created by Aleksandr Borzikh on 15.04.17.
//  Copyright © 2017 hate-red. All rights reserved.
//

#include "item.hpp"

Item::Item(GameWorld& world)
: GameObject(world),
  _carrierUID(0)
{
    _objType = GameObject::Type::ITEM;
    _objAttributes |= GameObject::Attributes::PASSABLE;
}

Item::Type
Item::GetType() const
{
    return _itemType;
}

Key::Key(GameWorld& world)
: Item(world)
{
    _itemType = Item::Type::KEY;
}

Sword::Sword(GameWorld& world)
: Item(world)
{
    _itemType = Item::Type::SWORD;
}
