/*
 * Copyright (C) 2018 Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 * Heinrich-Heine University
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __Missile_include__
#define __Missile_include__

#include <lib/file/bmp/Bmp.h>
#include "apps/game/GameObject.h"

class Missile : public GameObject {

  private:

    Missile(const Missile &copy);

    const static int width = 4;
    const static int height = 17;

    constexpr static float speed = 200.0f;

    int direction;

  public:

    Missile(Vector2 position, int direction, char* tag);

    static Image *enemySprite;

    static Image *shipSprite;

    ~Missile(){}

    void update(float delta);

    void draw(LinearFrameBuffer* g2d);

    void onCollisionEnter(GameObject &other) override;

    void onCollisionExit();

};

#endif
