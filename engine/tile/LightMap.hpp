#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include "engine/tile/TileTypes.hpp"
#include "engine/tile/Coords.hpp"

class Chunk; // forward declaration

class LightMap {
public:
    LightMap(unsigned w = CHUNK_W, unsigned h = CHUNK_H) 
        : w_(w), h_(h), lightLevels_(w*h, 0) {}

    unsigned width() const { return w_; }
    unsigned height() const { return h_; }

    unsigned getLight(unsigned x, unsigned y) const {
        if (x >= w_ || y >= h_) return 0;
        return lightLevels_[y*w_ + x];
    }

    void setLight(unsigned x, unsigned y, unsigned level) {
        if (x >= w_ || y >= h_) return;
        lightLevels_[y*w_ + x] = std::min(level, MAX_LIGHT_LEVEL);
    }

    // Calculate lighting for entire chunk based on tile data
    void calculateLighting(const Chunk& chunk, unsigned ambientLight = 0);

private:
    unsigned w_, h_;
    std::vector<unsigned> lightLevels_;

    // Light propagation using flood-fill algorithm
    void propagateLight(const Chunk& chunk, unsigned startX, unsigned startY, unsigned lightLevel);
};