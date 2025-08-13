#include "engine/world/World.hpp"
#include <cmath>

void World::ensureVisible(const sf::View& view, float inflatePixels) {
    const sf::Vector2f c = view.getCenter();
    const sf::Vector2f sz = view.getSize();
    const float left   = c.x - sz.x * 0.5f - inflatePixels;
    const float right  = c.x + sz.x * 0.5f + inflatePixels;
    const float top    = c.y - sz.y * 0.5f - inflatePixels;
    const float bottom = c.y + sz.y * 0.5f + inflatePixels;

    // Compute chunk rect overlapping the inflated view
    const ChunkCoord cmin = worldPixelsToChunk(left,  top);
    const ChunkCoord cmax = worldPixelsToChunk(right, bottom);

    for (int cy = cmin.y; cy <= cmax.y; ++cy) {
        for (int cx = cmin.x; cx <= cmax.x; ++cx) {
            ChunkCoord key{cx, cy};
            if (chunks_.find(key) != chunks_.end()) continue;

            // Create and generate
            Entry e{Chunk(key)};
            e.chunk.generate(seed_);
            e.batch.build(e.chunk);
            chunks_.emplace(key, std::move(e));
        }
    }
}

void World::draw(sf::RenderTarget& t, sf::RenderStates s) const {
    // In a real engine we’d cull here too; for now draw all loaded entries.
    for (const auto& kv : chunks_) {
        t.draw(kv.second.batch, s);
    }
}
