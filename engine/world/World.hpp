#pragma once
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <SFML/Graphics.hpp>
#include "engine/tile/Coords.hpp"
#include "engine/tile/Chunk.hpp"
#include "engine/tile/TileBatch.hpp"
#include "engine/tile/TileAtlas.hpp"
#include "engine/tile/TileTypes.hpp"

class World : public sf::Drawable {
public:
    explicit World(const TileAtlas* atlas, unsigned seed = 0, size_t maxChunks = 1000)
        : atlas_(atlas), seed_(seed), maxChunks_(maxChunks) {
        if (!atlas_) {
            throw std::invalid_argument("World requires a valid TileAtlas pointer");
        }
    }

    // keepMarginChunks: extra chunk margin to keep around visible area
    void ensureVisible(const sf::View& view,
                       float inflatePixels = 256.f,
                       int keepMarginChunks = 2);

    // NEW: edit helpers
    bool setTileAtTile(int tx, int ty, TileID id);
    bool setTileAtPixel(const sf::Vector2f& worldPx, TileID id);

private:
    struct Entry { Chunk chunk; TileBatch batch; };

    std::unordered_map<ChunkCoord, Entry, ChunkCoordHash> chunks_;
    const TileAtlas* atlas_{nullptr};
    unsigned seed_{0};
    size_t maxChunks_{1000};

    void draw(sf::RenderTarget& t, sf::RenderStates s) const override;
};
