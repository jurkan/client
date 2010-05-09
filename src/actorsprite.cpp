/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "actorsprite.h"

#include "client.h"
#include "effectmanager.h"
#include "imagesprite.h"
#include "localplayer.h"
#include "log.h"
#include "simpleanimation.h"
#include "sound.h"
#include "statuseffect.h"

#include "gui/theme.h"

#include "net/net.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#define EFFECTS_FILE "effects.xml"

ActorSprite::ActorSprite(int id):
    mId(id),
    mStunMode(0),
    mStatusParticleEffects(&mStunParticleEffects, false),
    mChildParticleEffects(&mStatusParticleEffects, false),
    mMustResetParticles(false),
    mUsedTargetCursor(NULL)
{
    //
}

ActorSprite::~ActorSprite()
{
    setMap(NULL);

    mUsedTargetCursor = NULL;

    if (player_node && player_node->getTarget() == this)
        player_node->setTarget(NULL);
}

bool ActorSprite::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    // TODO: Eventually, we probably should fix all sprite offsets so that
    //       these translations aren't necessary anymore. The sprites know
    //       best where their base point should be.
    const int px = getPixelX() + offsetX - 16;
    // Temporary fix to the Y offset.
    const int py = getPixelY() + offsetY -
        ((Net::getNetworkType() == ServerInfo::MANASERV) ? 15 : 32);

    if (mUsedTargetCursor)
        mUsedTargetCursor->draw(graphics, px, py);

    return drawSpriteAt(graphics, px, py);
}

bool ActorSprite::drawSpriteAt(Graphics *graphics, int x, int y) const
{
    return CompoundSprite::draw(graphics, x, y);
}

void ActorSprite::logic()
{
    // Update sprite animations
    if (mUsedTargetCursor)
        mUsedTargetCursor->update(tick_time * MILLISECONDS_IN_A_TICK);

    update(tick_time * MILLISECONDS_IN_A_TICK);

    // Restart status/particle effects, if needed
    if (mMustResetParticles)
    {
        mMustResetParticles = false;
        for (std::set<int>::iterator it = mStatusEffects.begin();
             it != mStatusEffects.end(); it++)
        {
            const StatusEffect *effect = StatusEffect::getStatusEffect(*it, true);
            if (effect && effect->particleEffectIsPersistent())
                updateStatusEffect(*it, true);
        }
    }

    // Update particle effects
    mChildParticleEffects.moveTo(mPos.x, mPos.y);
}

void ActorSprite::setMap(Map* map)
{
    Actor::setMap(map);

    // Clear particle effect list because child particles became invalid
    mChildParticleEffects.clear();
    mMustResetParticles = true; // Reset status particles on next redraw
}

void ActorSprite::controlParticle(Particle *particle)
{
    mChildParticleEffects.addLocally(particle);
}

void ActorSprite::setTargetAnimation(SimpleAnimation *animation)
{
    mUsedTargetCursor = animation;
    mUsedTargetCursor->reset();
}

struct EffectDescription {
    std::string mGFXEffect;
    std::string mSFXEffect;
};

static EffectDescription *default_effect = NULL;
static std::map<int, EffectDescription *> effects;
static bool effects_initialized = false;

static EffectDescription *getEffectDescription(xmlNodePtr node, int *id)
{
    EffectDescription *ed = new EffectDescription;

    *id = atoi(XML::getProperty(node, "id", "-1").c_str());
    ed->mSFXEffect = XML::getProperty(node, "audio", "");
    ed->mGFXEffect = XML::getProperty(node, "particle", "");

    return ed;
}

static EffectDescription *getEffectDescription(int effectId)
{
    if (!effects_initialized)
    {
        XML::Document doc(EFFECTS_FILE);
        xmlNodePtr root = doc.rootNode();

        if (!root || !xmlStrEqual(root->name, BAD_CAST "being-effects"))
        {
            logger->log("Error loading being effects file: "
                    EFFECTS_FILE);
            return NULL;
        }

        for_each_xml_child_node(node, root)
        {
            int id;

            if (xmlStrEqual(node->name, BAD_CAST "effect"))
            {
                EffectDescription *EffectDescription =
                    getEffectDescription(node, &id);
                effects[id] = EffectDescription;
            }
            else if (xmlStrEqual(node->name, BAD_CAST "default"))
            {
                EffectDescription *effectDescription =
                    getEffectDescription(node, &id);

                if (default_effect)
                    delete default_effect;

                default_effect = effectDescription;
            }
        }

        effects_initialized = true;
    } // done initializing

    EffectDescription *ed = effects[effectId];

    return ed ? ed : default_effect;
}

void ActorSprite::setStatusEffect(int index, bool active)
{
    const bool wasActive = mStatusEffects.find(index) != mStatusEffects.end();

    if (active != wasActive)
    {
        updateStatusEffect(index, active);
        if (active)
            mStatusEffects.insert(index);
        else
            mStatusEffects.erase(index);
    }
}

void ActorSprite::setStatusEffectBlock(int offset, Uint16 newEffects)
{
    for (int i = 0; i < STATUS_EFFECTS; i++)
    {
        int index = StatusEffect::blockEffectIndexToEffectIndex(offset + i);

        if (index != -1)
            setStatusEffect(index, (newEffects & (1 << i)) > 0);
    }
}

void ActorSprite::internalTriggerEffect(int effectId, bool sfx, bool gfx)
{
    logger->log("Special effect #%d on %s", effectId,
                getId() == player_node->getId() ? "self" : "other");

    EffectDescription *ed = getEffectDescription(effectId);

    if (!ed)
    {
        logger->log("Unknown special effect and no default recorded");
        return;
    }

    if (gfx && !ed->mGFXEffect.empty())
    {
        Particle *selfFX;

        selfFX = particleEngine->addEffect(ed->mGFXEffect, 0, 0);
        controlParticle(selfFX);
    }

    if (sfx && !ed->mSFXEffect.empty())
        sound.playSfx(ed->mSFXEffect);
}

void ActorSprite::updateStunMode(int oldMode, int newMode)
{
    handleStatusEffect(StatusEffect::getStatusEffect(oldMode, false), -1);
    handleStatusEffect(StatusEffect::getStatusEffect(newMode, true), -1);
}

void ActorSprite::updateStatusEffect(int index, bool newStatus)
{
    handleStatusEffect(StatusEffect::getStatusEffect(index, newStatus), index);
}

void ActorSprite::handleStatusEffect(StatusEffect *effect, int effectId)
{
    if (!effect)
        return;

    // TODO: Find out how this is meant to be used
    // (SpriteAction != Being::Action)
    //SpriteAction action = effect->getAction();
    //if (action != ACTION_INVALID)
    //    setAction(action);

    Particle *particle = effect->getParticle();

    if (effectId >= 0)
    {
        mStatusParticleEffects.setLocally(effectId, particle);
    }
    else
    {
        mStunParticleEffects.clearLocally();
        if (particle)
            mStunParticleEffects.addLocally(particle);
    }
}

void ActorSprite::setupSpriteDisplay(const SpriteDisplay &display,
                                     bool forceDisplay)
{
    clear();

    SpriteRefs it, it_end;

    for (it = display.sprites.begin(), it_end = display.sprites.end();
         it != it_end; it++)
    {
        std::string file = "graphics/sprites/" + (*it)->sprite;
        int variant = (*it)->variant;
        push_back(AnimatedSprite::load(file, variant));
    }

    // Ensure that something is shown, if desired
    if (size() == 0 && forceDisplay)
    {
        if (display.image.empty())
            push_back(AnimatedSprite::load("graphics/sprites/error.xml"));
        else
        {
            ResourceManager *resman = ResourceManager::getInstance();
            std::string imagePath = "graphics/items/" + display.image;
            Image *img = resman->getImage(imagePath);

            if (!img)
                img = Theme::getImageFromTheme("unknown-item.png");

            push_back(new ImageSprite(img));
        }
    }

    mChildParticleEffects.clear();

    //setup particle effects
    if (Particle::enabled)
    {
        std::list<std::string>::const_iterator it, it_end;
        for (it = display.particles.begin(), it_end = display.particles.end();
             it != it_end; it++)
        {
            Particle *p = particleEngine->addEffect(*it, 0, 0);
            controlParticle(p);
        }
    }

    mMustResetParticles = true;
}
