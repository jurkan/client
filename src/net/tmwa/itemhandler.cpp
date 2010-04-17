/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/tmwa/itemhandler.h"

#include "flooritemmanager.h"

#include "net/messagein.h"

#include "net/tmwa/protocol.h"

namespace TmwAthena {

ItemHandler::ItemHandler()
{
    static const Uint16 _messages[] = {
        SMSG_ITEM_VISIBLE,
        SMSG_ITEM_DROPPED,
        SMSG_ITEM_REMOVE,
        0
    };
    handledMessages = _messages;
}

void ItemHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_ITEM_VISIBLE:
        case SMSG_ITEM_DROPPED:
            {
                int id = msg.readInt32();
                int itemId = msg.readInt16();
                msg.readInt8();  // identify flag
                int x = msg.readInt16();
                int y = msg.readInt16();
                msg.skip(4);     // amount,subX,subY / subX,subY,amount

                floorItemManager->create(id, itemId, x, y);
            }
            break;

        case SMSG_ITEM_REMOVE:
            if (FloorItem *item = floorItemManager->findById(msg.readInt32()))
                floorItemManager->destroy(item);
            break;
    }
}

} // namespace TmwAthena
