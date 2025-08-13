#pragma once
#include <vector>
#include <cmath>
#include "engine/tile/TileTypes.hpp"
#include "engine/tile/Coords.hpp"

class Chunk {
public:
    Chunk(ChunkCoord cc, unsigned w = CHUNK_W, unsigned h = CHUNK_H)
        : coord_(cc), w_(w), h_(h), data_(w_*h_, Tile::Air) {}

    unsigned width()  const { return w_; }
    unsigned height() const { return h_; }
    ChunkCoord coord() const { return coord_; }

    TileID get(unsigned x, unsigned y) const { return data_[y*w_ + x]; }
    void   set(unsigned x, unsigned y, TileID id) { data_[y*w_ + x] = id; }

    // Naive seamless terrain: use world-x to compute surface so neighboring chunks match
    void generate(unsigned seed = 0) {
        (void)seed;
        const float mid  = h_ * 0.55f;
        const float amp  = h_ * 0.18f;
        const float freq = 2.0f * 3.1415926f / 180.0f; // wavelength ~180 tiles

        const auto org = chunkOriginTiles(coord_); // in tiles
        for (unsigned lx = 0; lx < w_; ++lx) {
            const int worldX = org.x + static_cast<int>(lx);
            const float f = mid + amp * std::sin(freq * static_cast<float>(worldX));
            const int surface = static_cast<int>(f);
            for (unsigned y = 0; y < h_; ++y) {
                const int worldY = org.y + static_cast<int>(y);
                if (worldY < surface) {
                    set(lx, y, Tile::Air);
                } else if (worldY == surface) {
                    set(lx, y, Tile::Grass);
                } else if (worldY <= surface + 4) {
                    set(lx, y, Tile::Dirt);
                } else {
                    set(lx, y, Tile::Stone);
                }
            }
        }
    }

private:
    ChunkCoord coord_;
    unsigned w_, h_;
    std::vector<TileID> data_;
};
