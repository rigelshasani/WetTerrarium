#pragma once
#include <vector>
#include <cmath>
#include "engine/tile/TileTypes.hpp"

class Chunk {
public:
    Chunk(unsigned w = CHUNK_W, unsigned h = CHUNK_H)
        : w_(w), h_(h), data_(w_*h_, Tile::Air) {}

    unsigned width()  const { return w_; }
    unsigned height() const { return h_; }

    TileID get(unsigned x, unsigned y) const { return data_[y*w_ + x]; }
    void   set(unsigned x, unsigned y, TileID id) { data_[y*w_ + x] = id; }

    void generate(unsigned seed = 0) {
        (void)seed;
        const float mid  = h_ * 0.55f;
        const float amp  = h_ * 0.18f;
        const float freq = 2.0f * 3.1415926f / float(w_) * 2.0f;
        for (unsigned x = 0; x < w_; ++x) {
            int surface = int(mid + amp * std::sin(freq * float(x)));
            for (unsigned y = 0; y < h_; ++y) {
                if (y < unsigned(surface)) set(x,y,Tile::Air);
                else if (y == unsigned(surface)) set(x,y,Tile::Grass);
                else if (y <= unsigned(surface + 4)) set(x,y,Tile::Dirt);
                else set(x,y,Tile::Stone);
            }
        }
    }

private:
    unsigned w_, h_;
    std::vector<TileID> data_;
};
