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

#include "resources/monsterdb.h"

#include "log.h"

#include "resources/monsterinfo.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/xml.h"

#include "net/net.h"

#define OLD_TMWATHENA_OFFSET 1002

namespace
{
    MonsterDB::MonsterInfos mMonsterInfos;
    MonsterInfo mUnknown;
    bool mLoaded = false;
}

void MonsterDB::load()
{
    if (mLoaded)
        return;

    {
        SpriteReference *unknownSprite = new SpriteReference;
        unknownSprite->sprite = "error.xml";
        unknownSprite->variant = 0;

        SpriteDisplay display;
        display.sprites.push_front(unknownSprite);

        mUnknown.setDisplay(display);
    }

    logger->log("Initializing monster database...");

    XML::Document doc("monsters.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "monsters"))
    {
        logger->error("Monster Database: Error while loading monster.xml!");
    }

    int offset = XML::getProperty(rootNode, "offset", Net::getNetworkType() ==
                                 ServerInfo::TMWATHENA ? OLD_TMWATHENA_OFFSET : 0);

    //iterate <monster>s
    for_each_xml_child_node(monsterNode, rootNode)
    {
        if (!xmlStrEqual(monsterNode->name, BAD_CAST "monster"))
        {
            continue;
        }

        MonsterInfo *currentInfo = new MonsterInfo;

        currentInfo->setName(XML::getProperty(monsterNode, "name", _("unnamed")));

        std::string targetCursor;
        targetCursor = XML::getProperty(monsterNode, "targetCursor", "medium");
        if (targetCursor == "small")
        {
            currentInfo->setTargetCursorSize(Being::TC_SMALL);
        }
        else if (targetCursor == "medium")
        {
            currentInfo->setTargetCursorSize(Being::TC_MEDIUM);
        }
        else if (targetCursor == "large")
        {
            currentInfo->setTargetCursorSize(Being::TC_LARGE);
        }
        else
        {
            logger->log("MonsterDB: Unknown target cursor type \"%s\" for %s -"
                        "using medium sized one",
                        targetCursor.c_str(), currentInfo->getName().c_str());
            currentInfo->setTargetCursorSize(Being::TC_MEDIUM);
        }

        SpriteDisplay display;

        //iterate <sprite>s and <sound>s
        for_each_xml_child_node(spriteNode, monsterNode)
        {
            if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
            {
                SpriteReference *currentSprite = new SpriteReference;
                currentSprite->sprite = (const char*)spriteNode->xmlChildrenNode->content;
                currentSprite->variant = XML::getProperty(spriteNode, "variant", 0);
                display.sprites.push_back(currentSprite);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "sound"))
            {
                std::string event = XML::getProperty(spriteNode, "event", "");
                const char *filename;
                filename = (const char*) spriteNode->xmlChildrenNode->content;

                if (event == "hit")
                {
                    currentInfo->addSound(MONSTER_EVENT_HIT, filename);
                }
                else if (event == "miss")
                {
                    currentInfo->addSound(MONSTER_EVENT_MISS, filename);
                }
                else if (event == "hurt")
                {
                    currentInfo->addSound(MONSTER_EVENT_HURT, filename);
                }
                else if (event == "die")
                {
                    currentInfo->addSound(MONSTER_EVENT_DIE, filename);
                }
                else
                {
                    logger->log("MonsterDB: Warning, sound effect %s for "
                                "unknown event %s of monster %s",
                                filename, event.c_str(),
                                currentInfo->getName().c_str());
                }
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "attack"))
            {
                const int id = XML::getProperty(spriteNode, "id", 0);
                const std::string particleEffect = XML::getProperty(
                        spriteNode, "particle-effect", "");
                SpriteAction spriteAction = SpriteDef::makeSpriteAction(
                        XML::getProperty(spriteNode, "action", "attack"));
                const std::string missileParticle = XML::getProperty(
                        spriteNode, "missile-particle", "");
                currentInfo->addMonsterAttack(id, particleEffect, spriteAction, missileParticle);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
            {
                display.particles.push_back(
                    (const char*) spriteNode->xmlChildrenNode->content);
            }
        }
        currentInfo->setDisplay(display);

        mMonsterInfos[XML::getProperty(monsterNode, "id", 0) + offset] = currentInfo;
    }

    mLoaded = true;
}

void MonsterDB::unload()
{
    delete_all(mMonsterInfos);
    mMonsterInfos.clear();

    mLoaded = false;
}


const MonsterInfo &MonsterDB::get(int id)
{
    MonsterInfoIterator i = mMonsterInfos.find(id);

    if (i == mMonsterInfos.end())
    {
        logger->log("MonsterDB: Warning, unknown monster ID %d requested", id);
        return mUnknown;
    }
    else
    {
        return *(i->second);
    }
}
