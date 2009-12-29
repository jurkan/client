/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef NET_MESSAGEOUT_H
#define NET_MESSAGEOUT_H

#include <iosfwd>
#include <SDL_types.h>

namespace Net {

/**
 * Used for building an outgoing message.
 *
 * \ingroup Network
 */
class MessageOut
{
    public:
        virtual void writeInt8(Sint8 value);          /**< Writes a byte. */
        virtual void writeInt16(Sint16 value) = 0;    /**< Writes a short. */
        virtual void writeInt32(Sint32 value) = 0;    /**< Writes a long. */

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        virtual void writeString(const std::string &string, int length = -1);

        /**
         * Returns the content of the message.
         */
        virtual char *getData() const;

        /**
         * Returns the length of the data.
         */
        virtual unsigned int getDataSize() const;

    protected:
        /**
         * Constructor.
         */
        MessageOut(short id);

        /**
         * Expand the packet data to be able to hold more data.
         *
         * NOTE: For performance enhancements this method could allocate extra
         * memory in advance instead of expanding size every time more data is
         * added.
         */
        virtual void expand(size_t size) = 0;

        char *mData;                         /**< Data building up. */
        unsigned int mDataSize;              /**< Size of data. */
        unsigned int mPos;                   /**< Position in the data. */
};

}

#endif // NET_MESSAGEOUT_H