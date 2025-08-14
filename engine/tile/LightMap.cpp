#include "engine/tile/LightMap.hpp"
#include "engine/tile/Chunk.hpp"
#include <algorithm>
#include <cmath>

void LightMap::calculateLighting(const Chunk& chunk, unsigned ambientLight) {
    // First pass: Set base lighting levels for all tiles
    for (unsigned y = 0; y < h_; ++y) {
        for (unsigned x = 0; x < w_; ++x) {
            TileID tile = chunk.get(x, y);
            
            // Underground gets reasonable ambient lighting regardless of surface
            // Surface gets full ambient, underground gets reduced but decent lighting
            if (y < 8) {
                // Surface and near-surface - can be affected by shadows
                setLight(x, y, std::max(2u, ambientLight / 4));
            } else {
                // Underground - gets its own ambient lighting independent of surface
                setLight(x, y, std::max(4u, ambientLight / 2));
            }
        }
    }
    
    // Second pass: Add sunlight from top (only affects surface layers)
    if (ambientLight > 0) {
        for (unsigned x = 0; x < w_; ++x) {
            unsigned currentLight = ambientLight;
            for (unsigned y = 0; y < h_ && y < 12 && currentLight > 0; ++y) { // Limit sunlight depth
                TileID tile = chunk.get(x, y);
                
                if (tile == Tile::Air) {
                    setLight(x, y, std::max(getLight(x, y), currentLight));
                } else if (tile == Tile::Leaves) {
                    // Leaves get good lighting and only slightly reduce passing light
                    setLight(x, y, std::max(getLight(x, y), currentLight));
                    currentLight = std::max(currentLight / 2, currentLight - 2);
                } else if (blocksLight(tile)) {
                    // Surface blocks get good lighting
                    setLight(x, y, std::max(getLight(x, y), currentLight));
                    currentLight = std::max(0u, currentLight - 4); // Solid blocks reduce light more
                } else {
                    setLight(x, y, std::max(getLight(x, y), currentLight));
                }
            }
        }
    }
    
    // Third pass: Add light sources and propagate
    for (unsigned y = 0; y < h_; ++y) {
        for (unsigned x = 0; x < w_; ++x) {
            TileID tile = chunk.get(x, y);
            if (isLightSource(tile)) {
                unsigned emission = getLightEmission(tile);
                propagateLight(chunk, x, y, emission);
            }
        }
    }
}

void LightMap::propagateLight(const Chunk& chunk, unsigned startX, unsigned startY, unsigned lightLevel) {
    if (lightLevel <= 1) return; // No more light to propagate
    
    // Use flood-fill to propagate light
    std::queue<std::tuple<unsigned, unsigned, unsigned>> queue;
    queue.push({startX, startY, lightLevel});
    
    // Track visited positions to avoid infinite loops
    std::vector<bool> visited(w_ * h_, false);
    
    while (!queue.empty()) {
        auto [x, y, level] = queue.front();
        queue.pop();
        
        if (x >= w_ || y >= h_) continue;
        
        size_t idx = y * w_ + x;
        if (visited[idx]) continue;
        visited[idx] = true;
        
        // Set light level (take maximum of current and new)
        unsigned currentLight = getLight(x, y);
        unsigned newLight = std::max(currentLight, level);
        setLight(x, y, newLight);
        
        // Propagate to neighbors if we have light left
        if (level > 1) {
            TileID currentTile = chunk.get(x, y);
            unsigned nextLevel = level - 1;
            
            // Reduce light more if passing through blocking materials
            if (currentTile == Tile::Leaves) {
                nextLevel = std::max(0u, nextLevel - 1); // leaves reduce light more
            } else if (blocksLight(currentTile)) {
                nextLevel = 0; // solid blocks stop light completely
            }
            
            if (nextLevel > 0) {
                // Add neighbors to queue
                if (x > 0) queue.push({x-1, y, nextLevel});
                if (x < w_-1) queue.push({x+1, y, nextLevel});
                if (y > 0) queue.push({x, y-1, nextLevel});
                if (y < h_-1) queue.push({x, y+1, nextLevel});
            }
        }
    }
}