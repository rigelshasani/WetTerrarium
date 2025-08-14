#include "engine/world/World.hpp"
#include <cassert>

void World::ensureVisible(const sf::View& view, float inflatePixels, int keepMarginChunks) {
    assert(atlas_ && "World requires a valid TileAtlas*");

    const sf::Vector2f c  = view.getCenter();
    const sf::Vector2f sz = view.getSize();
    const float left   = c.x - sz.x * 0.5f - inflatePixels;
    const float right  = c.x + sz.x * 0.5f + inflatePixels;
    const float top    = c.y - sz.y * 0.5f - inflatePixels;
    const float bottom = c.y + sz.y * 0.5f + inflatePixels;

    const ChunkCoord cmin = worldPixelsToChunk(left,  top);
    const ChunkCoord cmax = worldPixelsToChunk(right, bottom);

    // Load visible chunks
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

    // Prune far chunks
    const int xmin = cmin.x - keepMarginChunks;
    const int xmax = cmax.x + keepMarginChunks;
    const int ymin = cmin.y - keepMarginChunks;
    const int ymax = cmax.y + keepMarginChunks;

    std::vector<ChunkCoord> toErase;
    toErase.reserve(chunks_.size());
    for (const auto& kv : chunks_) {
        const ChunkCoord cc = kv.first;
        if (cc.x < xmin || cc.x > xmax || cc.y < ymin || cc.y > ymax)
            toErase.push_back(cc);
    }
    for (const ChunkCoord& cc : toErase) chunks_.erase(cc);
}

void World::draw(sf::RenderTarget& t, sf::RenderStates s) const {
    // Draw all loaded chunks (caller should have ensured visibility)
    for (const auto& kv : chunks_) {
        t.draw(kv.second.batch, s);
    }
}

bool World::setTileAtTile(int tx, int ty, TileID id) {
    assert(atlas_ && "World requires a valid TileAtlas*");

    // Find chunk containing (tx, ty)
    // Use existing helpers: tile origin of chunk and CHUNK dims
    auto div_floor = [](int a, int b) {
        int q = a / b, r = a % b;
        if ((r != 0) && ((r > 0) != (b > 0))) --q;
        return q;
    };

    ChunkCoord cc{div_floor(tx, (int)CHUNK_W), div_floor(ty, (int)CHUNK_H)};
    const sf::Vector2i org = chunkOriginTiles(cc); // top-left tile of that chunk
    const int lx = tx - org.x;
    const int ly = ty - org.y;
    if (lx < 0 || ly < 0 || lx >= (int)CHUNK_W || ly >= (int)CHUNK_H) return false;

    // Get or create entry
    auto it = chunks_.find(cc);
    if (it == chunks_.end()) {
        Entry e{Chunk(cc)};
        e.chunk.generate(seed_);
        e.batch.build(e.chunk, *atlas_);
        it = chunks_.emplace(cc, std::move(e)).first;
    }

    // Apply edit if changed
    Entry& ent = it->second;
    if (ent.chunk.get((unsigned)lx, (unsigned)ly) == id) return false;
    ent.chunk.set((unsigned)lx, (unsigned)ly, id);
    ent.batch.build(ent.chunk, *atlas_); // rebuild this chunk's batch only
    return true;
}

bool World::setTileAtPixel(const sf::Vector2f& worldPx, TileID id) {
    const int tx = static_cast<int>(std::floor(worldPx.x / static_cast<float>(TILE_SIZE)));
    const int ty = static_cast<int>(std::floor(worldPx.y / static_cast<float>(TILE_SIZE)));
    return setTileAtTile(tx, ty, id);
}