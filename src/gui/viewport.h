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

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "beingmanager.h"
#include "configlistener.h"
#include "position.h"

#include "gui/widgets/windowcontainer.h"

#include <guichan/mouselistener.hpp>

class Being;
class BeingPopup;
class FloorItem;
class Graphics;
class ImageSet;
class Item;
class Map;
class PopupMenu;
class Window;

/** Delay between two mouse calls when dragging mouse and move the player */
const int walkingMouseDelay = 500;

/**
 * The viewport on the map. Displays the current map and handles mouse input
 * and the popup menu.
 *
 * TODO: This class is planned to be extended to allow floating widgets on top
 * of it such as NPC messages, which are positioned using map pixel
 * coordinates.
 */
class Viewport : public WindowContainer, public gcn::MouseListener,
    public ConfigListener
{
    public:
        /**
         * Constructor.
         */
        Viewport();

        /**
         * Destructor.
         */
        ~Viewport();

        /**
         * Sets the map displayed by the viewport.
         */
        void setMap(Map *map);

        /**
         * Draws the viewport.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Implements player to keep following mouse.
         */
        void logic();

        /**
         * Toggles whether the path debug graphics are shown
         */
        void toggleDebugPath();

        /**
         * Handles mouse press on map.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Handles mouse move on map
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Handles mouse button release on map.
         */
        void mouseReleased(gcn::MouseEvent &event);

        /**
         * Handles mouse move on map.
         */
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * Shows a popup for an item.
         * TODO Find some way to get rid of Item here
         */
        void showPopup(Window *parent, int x, int y, Item *item,
                       bool isInventory = true);

        /**
         * Closes the popup menu. Needed for when the player dies or switching
         * maps.
         */
        void closePopupMenu();

        /**
         * A relevant config option changed.
         */
        void optionChanged(const std::string &name);

        /**
         * Returns camera x offset in pixels.
         */
        int getCameraX() const { return (int) mPixelViewX; }

        /**
         * Returns camera y offset in pixels.
         */
        int getCameraY() const { return (int) mPixelViewY; }

        /**
         * Returns mouse x in pixels.
         */
        int getMouseX() const { return mMouseX; }

        /**
         * Returns mouse y in pixels.
         */
        int getMouseY() const { return mMouseY; }

        /**
         * Changes viewpoint by relative pixel coordinates.
         */
        void scrollBy(float x, float y) { mPixelViewX += x; mPixelViewY += y; }

        /**
         * Returns the current map object.
         */
        Map *getCurrentMap() const { return mMap; }

        /**
         * Hides the BeingPopup.
         */
        void hideBeingPopup();

    protected:
        friend class BeingManager;

        /// Clears the hovered being if it matches
        void clearHoverBeing(Being *being);

    private:
        /**
         * Finds a path from the player to the mouse, and draws it. This is for
         * debug purposes.
         */
        void _drawDebugPath(Graphics *graphics);

        /**
         * Draws the given path.
         */
        void _drawPath(Graphics *graphics, const Path &path);

        /**
         * Make the player go to the mouse position.
         */
        void _followMouse();

        Map *mMap;                   /**< The current map. */

        int mScrollRadius;
        int mScrollLaziness;
        int mScrollCenterOffsetX;
        int mScrollCenterOffsetY;
        int mMouseX;                 /**< Current mouse position in pixels. */
        int mMouseY;                 /**< Current mouse position in pixels. */
        float mPixelViewX;           /**< Current viewpoint in pixels. */
        float mPixelViewY;           /**< Current viewpoint in pixels. */
        int mShowDebugPath;         /**< Show a path from player to pointer. */

        bool mPlayerFollowMouse;

        int mLocalWalkTime; /**< Timestamp before the next walk can be sent. */

        PopupMenu *mPopupMenu;       /**< Popup menu. */
        Being *mHoverBeing;          /**< Being mouse is currently over. */
        FloorItem *mHoverItem;       /**< FloorItem mouse is currently over. */
        BeingPopup *mBeingPopup;     /**< Being information popup. */
};

extern Viewport *viewport;           /**< The viewport. */

#endif
