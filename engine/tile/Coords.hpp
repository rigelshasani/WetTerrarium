#pragma once
#include <cstdint>
#include <functional>
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

// Convert world pixel -> tile -> chunk coords
inline ChunkCoord worldPixelsToChunk(float wx, float wy) {
    const int tx = static_cast<int>(std::floor(wx / static_cast<float>(TILE_SIZE)));
    const int ty = static_cast<int>(std::floor(wy / static_cast<float>(TILE_SIZE)));
    const int cx = (tx >= 0) ? (tx / static_cast<int>(CHUNK_W)) : ((tx - static_cast<int>(CHUNK_W) + 1) / static_cast<int>(CHUNK_W));
    const int cy = (ty >= 0) ? (ty / static_cast<int>(CHUNK_H)) : ((ty - static_cast<int>(CHUNK_H) + 1) / static_cast<int>(CHUNK_H));
    return {cx, cy};
}

// chunk origin in world tiles
inline sf::Vector2i chunkOriginTiles(ChunkCoord c) {
    return {c.x * static_cast<int>(CHUNK_W), c.y * static_cast<int>(CHUNK_H)};
}
