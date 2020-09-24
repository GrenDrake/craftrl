#include <iostream>
#include "../src/world.h"


bool buildmap(World &w, unsigned long seed);
bool loadGameData(World &w);


int main() {
    World w;
    if (!loadGameData(w)) {
        std::cerr << "ERROR: Failed to load game data.\n";
        return 1;
    }

    int seed = 9743;
    for (int i = 0; i < 100; ++i) {
        w.allocMap(160, 160);
        bool result = buildmap(w, seed + i);
        if (!result) {
            std::cerr << "ERROR: Buildmap seed " << seed + i << " error returned false.\n";
            return 1;
        }

        Actor *player = w.getPlayer();
        if (!player) {
            std::cerr << "ERROR: player is null. (SEED " << seed + i << ")\n";
            return 1;
        }
        if (player->def.ident != 1) {
            std::cerr << "ERROR: player ActorDef has ident of " << player->def.ident << ", but should have ident of 1. (SEED " << seed + i << ")\n";
            return 1;
        }
    }

}
