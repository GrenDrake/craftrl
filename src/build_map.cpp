#include <iostream>
#include <cmath>
#include "world.h"

Point findOpenTile(World &w, Random &rng, bool allowActor, bool allowItem) {
    Point p;
    do {
        p.x = rng.next32() % w.width();
        p.y = rng.next32() % w.height();
        bool valid = true;
        const auto &t = w.at(p);
        if (w.getTileDef(t.terrain).solid) valid = false;
        if (!allowActor && t.actor) continue;
        if (valid) break;
    } while (1);
    return p;
}


bool buildmap(World &w, unsigned long seed) {
    Random rng;
    rng.seed(seed);

    // ensure all ground is grass
    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            w.at(Point(x, y)).terrain = 2;
        }
    }

    // add lakes
    for (int i = 0; i < 20; ++i) {
        int radius = rng.between(4, 20);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                if (dist <= radius) {
                    w.at(Point(x, y)).terrain = 3;
                }
            }
        }
    }

    // add mountains
    for (int i = 0; i < 40; ++i) {
        int radius = rng.between(2, 12);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                if (dist <= radius) {
                    w.at(Point(x, y)).terrain = 1;
                }
            }
        }
    }


    // add dirt patches
    for (int i = 0; i < 40; ++i) {
        int radius = rng.between(6, 12);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                Tile &tile = w.at(Point(x, y));
                if (dist <= radius && tile.terrain == 2) {
                    tile.terrain = 0;
                }
            }
        }
    }

    // build map borders
    for (int x = 0; x < w.width(); ++x) {
        int size = rng.between(3, 5);
        for (int y = 0; y < size; ++y) {
            Point top(x, y);
            w.at(top).terrain = 3;
            Point bottom(x, w.height() - 1 - y);
            w.at(bottom).terrain = 3;
        }
    }
    for (int y = 0; y < w.height(); ++y) {
        int size = rng.between(3, 5);
        for (int x = 0; x < size; ++x) {
            Point left(x, y);
            w.at(left).terrain = 3;
            Point right(w.width() - 1 - x, y);
            w.at(right).terrain = 3;
        }
    }


    // add plants
    int plantList[] = { 1000, 1000, 1001, 1005, 1007, 1009 };
    for (int i = 0; i < 1600; ++i) {
        Point p = findOpenTile(w, rng, false, false);
        int plantNum = rng.next32() % 6;
        Actor *actor = new Actor(w.getActorDef(plantList[plantNum]));
        actor->reset();
        if (!w.moveActor(actor, p)) {
            std::cerr << "Failed to place plant at " << p << ".\n";
            delete actor;
        }
    }

    // add NPCs
    int npcList[] = { 2, 3, 3, 4, 4, 4, 5, 5, 6, 3, 3, 4, 4, 4, 5, 5, 6};
    for (int i = 0; i < 75; ++i) {
        Point p = findOpenTile(w, rng, false, true);
        int npcNum = rng.next32() % 9;
        int type = npcList[npcNum];
        Actor *actor = new Actor(w.getActorDef(type));
        actor->reset();
        if (!w.moveActor(actor, p)) {
            std::cerr << "Failed to place actor at " << p << ".\n";
            delete actor;
        }
    }

    // add player
    Actor *player = new Actor(w.getActorDef(1));
    Point starting = findOpenTile(w, rng, false, true);
    player->reset();
    w.moveActor(player, starting);

    return true;
}