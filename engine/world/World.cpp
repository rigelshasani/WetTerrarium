#include "engine/world/World.hpp"
#include <cassert>

void World::ensureVisible(const sf::View& view, float inflatePixels) {
    assert(atlas_ && "World requires a valid TileAtlas*");

    const sf::Vector2f c  = view.getCenter();
    const sf::Vector2f sz = view.getSize();
    const float left   = c.x - sz.x * 0.5f - inflatePixels;
    const float right  = c.x + sz.x * 0.5f + inflatePixels;
    const float top    = c.y - sz.y * 0.5f - inflatePixels;
    const float bottom = c.y + sz.y * 0.5f + inflatePixels;

    const ChunkCoord cmin = worldPixelsToChunk(left,  top);
    const ChunkCoord cmax = worldPixelsToChunk(right, bottom);

    for (int cy = cmin.y; cy <= cmax.y; ++cy) {
        for (int cx = cmin.x; cx <= cmax.x; ++cx) {
            ChunkCoord key{cx, cy};
            if (chunks_.find(key) != chunks_.end()) continue;

            Entry e{Chunk(key)};
            e.chunk.generate(seed_);
            e.batch.build(e.chunk, *atlas_);
            chunks_.emplace(key, std::move(e));
        }
    }
}

void World::draw(sf::RenderTarget& t, sf::RenderStates s) const {
    // Draw all loaded chunks (caller should have ensured visibility)
    for (const auto& kv : chunks_) {
        t.draw(kv.second.batch, s);
    }
}
