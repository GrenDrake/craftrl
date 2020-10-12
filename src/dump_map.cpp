#include <vector>
#include <physfs.h>
#include "lodepng.h"
#include "world.h"

void dumpActorMap(World &w) {
    std::vector<unsigned char> image;
    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            auto tile = w.at(Point(x, y));
            if (tile.actor && tile.actor->def.type != TYPE_PLANT) {
                image.push_back((tile.actor->def.colour & 0x00FF0000) >> 16);
                image.push_back((tile.actor->def.colour & 0x0000FF00) >> 8);
                image.push_back((tile.actor->def.colour & 0x000000FF));
                image.push_back(255);
            } else {
                image.push_back(0);
                image.push_back(0);
                image.push_back(0);
                image.push_back(0);
            }
        }
    }

    std::vector<unsigned char> pngData;
    unsigned error = lodepng::encode(pngData, image, w.width(), w.height());
    if (error) {
        w.addLogMsg("encoder error ");// << error << ": "<< lodepng_error_text(error) << std::endl;
    }

    PHYSFS_File *file = PHYSFS_openWrite("map_actor.png");
    for (unsigned char c : pngData) {
        PHYSFS_writeBytes(file, &c, 1);
    }
    PHYSFS_close(file);
}


void dumpPlantMap(World &w) {
    std::vector<unsigned char> image;
    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            auto tile = w.at(Point(x, y));
            if (tile.actor && tile.actor->def.type == TYPE_PLANT) {
                image.push_back((tile.actor->def.colour & 0x00FF0000) >> 16);
                image.push_back((tile.actor->def.colour & 0x0000FF00) >> 8);
                image.push_back((tile.actor->def.colour & 0x000000FF));
                image.push_back(255);
            } else {
                image.push_back(0);
                image.push_back(0);
                image.push_back(0);
                image.push_back(0);
            }
        }
    }

    std::vector<unsigned char> pngData;
    unsigned error = lodepng::encode(pngData, image, w.width(), w.height());
    if (error) {
        w.addLogMsg("encoder error ");// << error << ": "<< lodepng_error_text(error) << std::endl;
    }

    PHYSFS_File *file = PHYSFS_openWrite("map_plant.png");
    for (unsigned char c : pngData) {
        PHYSFS_writeBytes(file, &c, 1);
    }
    PHYSFS_close(file);
}
void dumpTerrainMap(World &w) {
    std::vector<unsigned char> image;
    for (int y = 0; y < w.height(); ++y) {
        for (int x = 0; x < w.width(); ++x) {
            auto tile = w.at(Point(x, y));
            auto td = w.getTileDef(tile.terrain);
            image.push_back((td.colour & 0x00FF0000) >> 16);
            image.push_back((td.colour & 0x0000FF00) >> 8);
            image.push_back((td.colour & 0x000000FF));
            image.push_back(255);
        }
    }

    std::vector<unsigned char> pngData;
    unsigned error = lodepng::encode(pngData, image, w.width(), w.height());
    if (error) {
        w.addLogMsg("encoder error ");// << error << ": "<< lodepng_error_text(error) << std::endl;
    }

    PHYSFS_File *file = PHYSFS_openWrite("map_terrain.png");
    for (unsigned char c : pngData) {
        PHYSFS_writeBytes(file, &c, 1);
    }
    PHYSFS_close(file);
}
