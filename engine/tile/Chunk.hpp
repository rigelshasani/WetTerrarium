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

    TileID get(unsigned x, unsigned y) const { 
        if (x >= w_ || y >= h_) return Tile::Air;
        return data_[y*w_ + x]; 
    }
    void   set(unsigned x, unsigned y, TileID id) { 
        if (x >= w_ || y >= h_) return;
        data_[y*w_ + x] = id; 
    }

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

        // Tree placement pass — place different tree types after basic terrain
        for (unsigned lx = 0; lx < w_; ++lx) {
            const int worldX = org.x + static_cast<int>(lx);
            const int surf = surfaceCol[lx];
            
            // Only place trees on grass surface, with some spacing
            if (lx < w_ - 1) { // need space for tree
                const unsigned ly = static_cast<unsigned>(std::max(0, surf - org.y));
                if (ly < h_ && get(lx, ly) == Tile::Grass) {
                    // Use world coordinates for consistent tree placement
                    const std::uint64_t treeHash = hash2to1(static_cast<std::uint64_t>(worldX), static_cast<std::uint64_t>(seed));
                    
                    // Tree probability ~15% with spacing check
                    if ((treeHash & 0xFF) < 38 && lx % 6 < 3) { // more frequent trees
                        const unsigned treeSeed = static_cast<unsigned>(treeHash >> 8);
                        
                        // Choose tree type based on hash
                        TreeType treeType = static_cast<TreeType>((treeHash >> 16) % 3);
                        
                        switch (treeType) {
                            case TreeType::Classic:
                                placeClassicTree(lx, ly, treeSeed);
                                break;
                            case TreeType::Pine:
                                placePineTree(lx, ly, treeSeed);
                                break;
                            case TreeType::WeepingWillow:
                                placeWeepingWillowTree(lx, ly, treeSeed);
                                break;
                        }
                    }
                }
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
    enum class TreeType { Classic, Pine, WeepingWillow };
    
    void placeClassicTree(unsigned baseX, unsigned baseY, unsigned treeSeed) {
        const unsigned trunkHeight = 5 + (treeSeed % 4); // 5-8 tiles high
        
        // Place trunk
        for (unsigned i = 0; i < trunkHeight; ++i) {
            if (baseY > i) {
                set(baseX, baseY - i - 1, Tile::Wood);
            }
        }
        
        // Large bushy crown (5x5 with gaps for natural look)
        const unsigned crownCenterY = (baseY >= trunkHeight) ? baseY - trunkHeight : 0;
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                const int leafX = static_cast<int>(baseX) + dx;
                const int leafY = static_cast<int>(crownCenterY) + dy;
                
                if (leafX >= 0 && leafX < static_cast<int>(w_) && leafY >= 0 && leafY < static_cast<int>(h_)) {
                    // Create natural gaps - skip corners and some random positions
                    if ((std::abs(dx) == 2 && std::abs(dy) == 2)) continue; // corners
                    if (((treeSeed + dx + dy) & 0x7) == 0) continue; // random gaps
                    
                    set(static_cast<unsigned>(leafX), static_cast<unsigned>(leafY), Tile::Leaves);
                }
            }
        }
    }
    
    void placePineTree(unsigned baseX, unsigned baseY, unsigned treeSeed) {
        const unsigned trunkHeight = 8 + (treeSeed % 5); // 8-12 tiles high
        
        // Place trunk
        for (unsigned i = 0; i < trunkHeight; ++i) {
            if (baseY > i) {
                set(baseX, baseY - i - 1, Tile::Wood);
            }
        }
        
        // Triangular crown - wider at bottom, narrower at top
        const unsigned crownBase = (baseY >= trunkHeight) ? baseY - trunkHeight : 0;
        for (unsigned layer = 0; layer < trunkHeight / 2; ++layer) {
            const unsigned layerY = crownBase + layer;
            const unsigned width = 1 + (trunkHeight / 2 - layer) / 2; // gets narrower going up
            
            for (unsigned dx = 0; dx < width; ++dx) {
                // Place leaves on both sides
                if (baseX >= dx && layerY < h_) {
                    if (dx > 0) set(baseX - dx, layerY, Tile::Leaves);
                    set(baseX, layerY, Tile::Leaves);
                    if (baseX + dx < w_) set(baseX + dx, layerY, Tile::Leaves);
                }
            }
        }
    }
    
    void placeWeepingWillowTree(unsigned baseX, unsigned baseY, unsigned treeSeed) {
        const unsigned trunkHeight = 8 + (treeSeed % 6); // 8-13 tiles high (taller)
        
        // Place trunk
        for (unsigned i = 0; i < trunkHeight; ++i) {
            if (baseY > i) {
                set(baseX, baseY - i - 1, Tile::Wood);
            }
        }
        
        // Drooping branches - main crown at top, then hanging branches
        const unsigned crownTop = (baseY >= trunkHeight) ? baseY - trunkHeight : 0;
        
        // Larger top crown (5x3)
        for (int dy = 0; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                const int leafX = static_cast<int>(baseX) + dx;
                const int leafY = static_cast<int>(crownTop) + dy;
                
                if (leafX >= 0 && leafX < static_cast<int>(w_) && leafY >= 0 && leafY < static_cast<int>(h_)) {
                    // Skip some corners for natural look
                    if (dy == 0 && std::abs(dx) == 2) continue;
                    set(static_cast<unsigned>(leafX), static_cast<unsigned>(leafY), Tile::Leaves);
                }
            }
        }
        
        // More hanging droopy branches with varying lengths
        for (int dx = -3; dx <= 3; ++dx) { // wider spread
            const int branchX = static_cast<int>(baseX) + dx;
            if (branchX >= 0 && branchX < static_cast<int>(w_)) {
                // Longer drooping branches, especially in center
                const unsigned branchLength = 4 + (treeSeed % 4) + (std::abs(dx) == 0 ? 2 : 0); // center branches longer
                for (unsigned i = 0; i < branchLength && crownTop + 3 + i < h_; ++i) {
                    // Skip some positions for natural gaps
                    if (((treeSeed + dx + i) & 0x3) == 0) continue;
                    set(static_cast<unsigned>(branchX), crownTop + 3 + i, Tile::Leaves);
                }
            }
        }
    }
    ChunkCoord coord_;
    unsigned w_, h_;
    std::vector<TileID> data_;
};
