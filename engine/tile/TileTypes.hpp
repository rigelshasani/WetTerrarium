#pragma once
#include <cstdint>

using TileID = std::uint16_t;

namespace Tile {
    enum : TileID { Air = 0, Grass = 1, Dirt = 2, Stone = 3, Wood = 4, Leaves = 5, Torch = 6, Lantern = 7 };
}

// Light properties
inline constexpr unsigned MAX_LIGHT_LEVEL = 15;
inline bool isLightSource(TileID id) {
    return id == Tile::Torch || id == Tile::Lantern;
}
inline unsigned getLightEmission(TileID id) {
    switch (id) {
        case Tile::Torch: return 12;   // bright
        case Tile::Lantern: return 14; // very bright
        default: return 0;
    }
}
inline bool blocksLight(TileID id) {
    return id != Tile::Air && id != Tile::Leaves; // leaves partially block light
}

inline constexpr unsigned TILE_SIZE = 16; // px
inline constexpr unsigned CHUNK_W   = 128;
inline constexpr unsigned CHUNK_H   = 64;
