#include "world.h"


bool buildmap(World &w) {
    Random &rng = w.getRandom();

    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            int tile = rng.between(0, 8);
            switch (tile) {
                case 0: tile = 0; break;
                case 1: tile = 1; break;
                case 2: tile = 1; break;
                case 3: tile = 1; break;
                case 4: tile = 2; break;
                case 5: tile = 2; break;
                case 6: tile = 2; break;
                case 7: tile = 2; break;
                case 8: tile = 3; break;
            }
            w.at(Point(x, y)).terrain = tile;
        }
    }

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
    for (int i = 0; i < 100; ++i) {
        Point p = w.findOpenTile(true, false);
        Item *item = new Item(w.getItemDef(w.getRandom().between(0, 3)));
        w.moveItem(item, p);
    }


    // add plants
    for (int i = 0; i < 100; ++i) {
        Point p = w.findOpenTile(false, false);
        Actor *actor = new Actor(w.getActorDef(w.getRandom().between(1000, 1001)));
        actor->reset();
        w.moveActor(actor, p);
    }


    // add characters
    for (int i = 0; i < 25; ++i) {
        Point p = w.findOpenTile(false, true);
        int type = w.getRandom().between(2, 6);
        Actor *actor = new Actor(w.getActorDef(type));
        actor->reset();
        w.moveActor(actor, p);
    }





    return true;
}