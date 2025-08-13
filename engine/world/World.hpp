#pragma once
#include <unordered_map>
#include <vector>
#include <SFML/Graphics.hpp>
#include "engine/tile/Coords.hpp"
#include "engine/tile/Chunk.hpp"
#include "engine/tile/TileBatch.hpp"
#include "engine/tile/TileTypes.hpp"

class World : public sf::Drawable {
public:
    explicit World(unsigned seed = 0) : seed_(seed) {}

    // Ensure chunks exist around the given view in world pixels
    void ensureVisible(const sf::View& view, float inflatePixels = 256.f);

private:
    struct Entry {
        Chunk      chunk;
        TileBatch  batch;
    };

    std::unordered_map<ChunkCoord, Entry, ChunkCoordHash> chunks_;
    unsigned seed_{0};

    // drawing just iterates over visible ones (we’ll rely on caller to call ensureVisible first)
    void draw(sf::RenderTarget& t, sf::RenderStates s) const override;
};
