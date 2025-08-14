#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "engine/tile/TileTypes.hpp"
#include "engine/tile/Coords.hpp"
#include "engine/noise/ValueNoise.hpp"

class Chunk {
public:
    Chunk(ChunkCoord cc, unsigned w = CHUNK_W, unsigned h = CHUNK_H)
        : coord_(cc), w_(w), h_(h), data_(w_*h_, Tile::Air) {}

    unsigned width()  const { return w_; }
    unsigned height() const { return h_; }
    ChunkCoord coord() const { return coord_; }

    TileID get(unsigned x, unsigned y) const { return data_[y*w_ + x]; }
    void   set(unsigned x, unsigned y, TileID id) { data_[y*w_ + x] = id; }

    void generate(unsigned seed = 0) {
        const float mid  = h_ * 0.55f;
        const float amp  = h_ * 0.18f;
        const float wavL = 180.f; // tiles
        const float twoPi = 6.28318530718f;
        const float freq = twoPi / wavL;

        const auto org = chunkOriginTiles(coord_); // in tiles

        // Surface-first pass
        std::vector<int> surfaceCol(w_, 0);
        for (unsigned lx = 0; lx < w_; ++lx) {
            const int worldX = org.x + static_cast<int>(lx);
            const float f = mid + amp * std::sin(freq * static_cast<float>(worldX));
            const int surface = static_cast<int>(std::floor(f));
            surfaceCol[lx] = surface;

            for (unsigned ly = 0; ly < h_; ++ly) {
                const int worldY = org.y + static_cast<int>(ly);
                TileID t = Tile::Air;
                if (worldY == surface)              t = Tile::Grass;
                else if (worldY > surface && worldY <= surface + 4) t = Tile::Dirt;
                else if (worldY > surface + 4)      t = Tile::Stone;
                set(lx, ly, t);
            }
        }

        // Cave carving pass — deterministic FBM per world tile
        // Only carve in deep dirt/stone; spare the top band for stability
        const float baseFreq = 1.0f / 22.0f; // larger -> more caves
        for (unsigned lx = 0; lx < w_; ++lx) {
            const int worldX = org.x + static_cast<int>(lx);
            const int surf   = surfaceCol[lx];

            for (unsigned ly = 0; ly < h_; ++ly) {
                const int worldY = org.y + static_cast<int>(ly);
                TileID t = get(lx, ly);
                if (t == Tile::Air) continue;

                const int depthBelowSurface = worldY - surf;
                if (depthBelowSurface < 6) continue; // keep top layers solid

                // FBM noise; deeper -> more caverns (lower threshold)
                const float nx = static_cast<float>(worldX) * baseFreq;
                const float ny = static_cast<float>(worldY) * baseFreq * 0.8f;
                constexpr std::uint64_t CAVE_SALT = 0xC0FFEE5EEDULL; // valid hex, 64-bit
                float n = fbm2(nx, ny, static_cast<std::uint64_t>(seed) ^ CAVE_SALT,
                /*octaves=*/4, 
                /*lacunarity=*/2.0f, 
                /*gain=*/0.5f);                
                // depth factor 0..1 over ~80 tiles
                const float d = std::clamp(depthBelowSurface / 80.f, 0.f, 1.f);
                const float threshold = 0.62f - 0.25f * d; // deeper => more carve
                if (n > threshold) continue; // keep solid
                set(lx, ly, Tile::Air);
            }
        }
    }

private:
    ChunkCoord coord_;
    unsigned w_, h_;
    std::vector<TileID> data_;
};
