#pragma once
#include <cstdint>

using TileID = std::uint16_t;

namespace Tile {
    enum : TileID { Air = 0, Grass = 1, Dirt = 2, Stone = 3 };
}

inline constexpr unsigned TILE_SIZE = 16; // px
inline constexpr unsigned CHUNK_W   = 128;
inline constexpr unsigned CHUNK_H   = 64;
