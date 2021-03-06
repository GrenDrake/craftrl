#include <cmath>
#include "world.h"

Point findOpenTile(World &w, Random &rng, bool allowActor, bool allowItem) {
    Point p;
    do {
        p.x = rng.next32() % w.width();
        p.y = rng.next32() % w.height();
        bool valid = true;
        const auto &t = w.at(p);
        if (t.building > 0) valid = false;
        if (w.getTileDef(t.terrain).solid) valid = false;
        if (!allowActor && t.actor) continue;
        if (valid) break;
    } while (1);
    return p;
}


bool buildmap(World &w, unsigned long seed) {
    Random rng;
    rng.seed(seed);

    const int mapArea = w.width() * w.height();
    const int lakeCount = mapArea / 1280;
    const int mountainCount = mapArea / 640;
    const int dirtCount = mapArea / 640;
    const int sandCount = mapArea / 640;
    const int oreCount = mapArea / 32;
    const int plantCount = mapArea / 16;
    const int actorCount = mapArea / 340;

    // ensure all ground is grass
    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            w.setTerrain(Point(x, y), TILE_GRASS);
        }
    }

    // add lakes
    for (int i = 0; i < lakeCount; ++i) {
        int radius = rng.between(4, 20);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                if (dist <= radius) {
                    w.setTerrain(Point(x, y), TILE_WATER);
                }
            }
        }
    }

    // add mountains
    for (int i = 0; i < mountainCount; ++i) {
        int radius = rng.between(2, 12);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                if (dist <= radius) {
                    w.setTerrain(Point(x, y), TILE_DIRT);
                    w.setBuilding(Point(x, y), TILE_STONE);
                }
            }
        }
    }


    // add dirt patches
    for (int i = 0; i < dirtCount; ++i) {
        int radius = rng.between(6, 12);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                const Tile &tile = w.at(Point(x, y));
                if (dist <= radius && tile.terrain == 2) {
                    w.setTerrain(Point(x, y), TILE_DIRT);
                }
            }
        }
    }


    // add sand patches
    for (int i = 0; i < sandCount; ++i) {
        int radius = rng.between(6, 12);
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        for (int y = cy - radius; y <= cy + radius; ++y) {
            for (int x = cx - radius; x <= cx + radius; ++x) {
                int dist = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                const Tile &tile = w.at(Point(x, y));
                if (dist <= radius && tile.terrain == 2) {
                    w.setTerrain(Point(x, y), TILE_SAND);
                }
            }
        }
    }


    // add ore veins
    std::vector<int> oreList{ 30, 31, 32, 32, 33, 34 };
    for (int i = 0; i < oreCount; ++i) {
        int cx = rng.next32() % w.width();
        int cy = rng.next32() % w.height();
        Point c(cx, cy);
        if (w.at(c).building == TILE_STONE) {
            int oreNum = rng.next32() % oreList.size();
            w.setBuilding(c, oreList[oreNum]);
        }
    }

    // build map borders
    for (int x = 0; x < w.width(); ++x) {
        int size = rng.between(3, 5);
        for (int y = 0; y < size; ++y) {
            Point top(x, y);
            w.setTerrain(top, TILE_OCEAN);
            w.setBuilding(top, 0);
            Point bottom(x, w.height() - 1 - y);
            w.setTerrain(bottom, TILE_OCEAN);
            w.setBuilding(bottom, 0);
        }
    }
    for (int y = 0; y < w.height(); ++y) {
        int size = rng.between(3, 5);
        for (int x = 0; x < size; ++x) {
            Point left(x, y);
            w.setTerrain(left, TILE_OCEAN);
            w.setBuilding(left, 0);
            Point right(w.width() - 1 - x, y);
            w.setTerrain(right, TILE_OCEAN);
            w.setBuilding(right, 0);
        }
    }


    // add plants
    int plantList[] = { 1000, 1000, 1001, 1005, 1007, 1009 };
    for (int i = 0; i < plantCount; ++i) {
        Point p = findOpenTile(w, rng, false, false);
        int plantNum = rng.next32() % 6;
        Actor *actor = new Actor(w.getActorDef(plantList[plantNum]));
        actor->reset();
        if (!w.moveActor(actor, p)) {
            logger_log("Failed to place plant at " + p.toString() + ".");
            delete actor;
        }
    }

    // add NPCs
    int npcList[] = { 2, 3, 3, 4, 4, 4, 5, 5, 6, 3, 3, 4, 4, 4, 5, 5, 6, 2000};
    for (int i = 0; i < actorCount; ++i) {
        Point p = findOpenTile(w, rng, false, true);
        int npcNum = rng.next32() % 18;
        int type = npcList[npcNum];
        Actor *actor = new Actor(w.getActorDef(type));
        actor->reset();
        if (!w.moveActor(actor, p)) {
            logger_log("Failed to place actor at " + p.toString() + ".");
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