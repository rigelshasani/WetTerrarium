#pragma once
#include <cstdint>
#include <functional>
#include <cmath>
#include <limits>
#include <SFML/System/Vector2.hpp>
#include "engine/tile/TileTypes.hpp"

using ChunkX = int32_t;
using ChunkY = int32_t;
using TileX  = int32_t;
using TileY  = int32_t;

// Chunk coordinate (in chunk units)
struct ChunkCoord {
    ChunkX x{};
    ChunkY y{};
    bool operator==(const ChunkCoord& o) const noexcept { return x == o.x && y == o.y; }
};

// Hash for unordered_map
struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& c) const noexcept {
        // 64-bit mix
        uint64_t ux = static_cast<uint64_t>(static_cast<int64_t>(c.x));
        uint64_t uy = static_cast<uint64_t>(static_cast<int64_t>(c.y));
        uint64_t h = ux * 0x9E3779B185EBCA87ull ^ (uy + 0x9E3779B185EBCA87ull + (ux << 6) + (ux >> 2));
        return static_cast<size_t>(h);
    }
};

// Convert world pixel -> tile -> chunk coords with overflow protection
inline ChunkCoord worldPixelsToChunk(float wx, float wy) {
    // Clamp input to safe ranges to prevent overflow
    constexpr float max_coord = static_cast<float>(std::numeric_limits<int32_t>::max() / 2);
    wx = std::clamp(wx, -max_coord, max_coord);
    wy = std::clamp(wy, -max_coord, max_coord);
    
    const int tx = static_cast<int>(std::floor(wx / static_cast<float>(TILE_SIZE)));
    const int ty = static_cast<int>(std::floor(wy / static_cast<float>(TILE_SIZE)));
    
    // Safe division with proper negative handling
    auto safe_div = [](int numerator, int denominator) -> int {
        if (denominator == 0) return 0;
        return (numerator >= 0) ? (numerator / denominator) 
                                : ((numerator - denominator + 1) / denominator);
    };
    
    const int cx = safe_div(tx, static_cast<int>(CHUNK_W));
    const int cy = safe_div(ty, static_cast<int>(CHUNK_H));
    return {static_cast<ChunkX>(cx), static_cast<ChunkY>(cy)};
}

// chunk origin in world tiles with overflow protection
inline sf::Vector2i chunkOriginTiles(ChunkCoord c) {
    // Check for potential overflow before multiplication
    constexpr int32_t max_chunk = std::numeric_limits<int32_t>::max() / static_cast<int32_t>(std::max(CHUNK_W, CHUNK_H));
    
    const int32_t safe_cx = std::clamp(c.x, -max_chunk, max_chunk);
    const int32_t safe_cy = std::clamp(c.y, -max_chunk, max_chunk);
    
    return {safe_cx * static_cast<int>(CHUNK_W), safe_cy * static_cast<int>(CHUNK_H)};
}
