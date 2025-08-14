#include "engine/world/World.hpp"
#include <cassert>
#include <algorithm>
#include <cmath>

void World::ensureVisible(const sf::View& view, float inflatePixels, int keepMarginChunks) {
    assert(atlas_ && "World requires a valid TileAtlas*");
    
    // Validate input parameters
    if (inflatePixels < 0.f || !std::isfinite(inflatePixels)) {
        inflatePixels = 0.f;
    }
    if (keepMarginChunks < 0) {
        keepMarginChunks = 0;
    }
    
    // Validate view parameters
    const sf::Vector2f center = view.getCenter();
    const sf::Vector2f size = view.getSize();
    if (!std::isfinite(center.x) || !std::isfinite(center.y) || 
        !std::isfinite(size.x) || !std::isfinite(size.y) ||
        size.x <= 0.f || size.y <= 0.f) {
        return; // Invalid view, skip processing
    }

    const float left   = center.x - size.x * 0.5f - inflatePixels;
    const float right  = center.x + size.x * 0.5f + inflatePixels;
    const float top    = center.y - size.y * 0.5f - inflatePixels;
    const float bottom = center.y + size.y * 0.5f + inflatePixels;

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

    // Prune far chunks and enforce memory limits
    const int xmin = cmin.x - keepMarginChunks;
    const int xmax = cmax.x + keepMarginChunks;
    const int ymin = cmin.y - keepMarginChunks;
    const int ymax = cmax.y + keepMarginChunks;

    std::vector<ChunkCoord> toErase;
    toErase.reserve(chunks_.size());
    
    // First pass: mark chunks outside visible area
    for (const auto& kv : chunks_) {
        const ChunkCoord cc = kv.first;
        if (cc.x < xmin || cc.x > xmax || cc.y < ymin || cc.y > ymax)
            toErase.push_back(cc);
    }
    
    // Second pass: enforce hard memory limit if needed
    if (chunks_.size() > maxChunks_) {
        // Calculate distances from camera center and sort by distance
        const ChunkCoord camChunk = worldPixelsToChunk(view.getCenter().x, view.getCenter().y);
        
        std::vector<std::pair<int, ChunkCoord>> distances;
        for (const auto& kv : chunks_) {
            const ChunkCoord cc = kv.first;
            // Skip chunks already marked for removal
            if (std::find(toErase.begin(), toErase.end(), cc) != toErase.end()) continue;
            
            const int dx = cc.x - camChunk.x;
            const int dy = cc.y - camChunk.y;
            const int distSq = dx * dx + dy * dy;
            distances.emplace_back(distSq, cc);
        }
        
        // Sort by distance (farthest first)
        std::sort(distances.begin(), distances.end(), [](const auto& a, const auto& b) {
            return a.first > b.first; // Compare only the distance (first element)
        });
        
        // Mark excess chunks for removal
        const size_t chunksToKeep = maxChunks_ - toErase.size();
        if (distances.size() > chunksToKeep) {
            for (size_t i = chunksToKeep; i < distances.size(); ++i) {
                toErase.push_back(distances[i].second);
            }
        }
    }
    
    for (const ChunkCoord& cc : toErase) chunks_.erase(cc);
}

void World::draw(sf::RenderTarget& t, sf::RenderStates s) const {
    // Get view bounds for frustum culling
    const sf::View view = t.getView();
    const sf::Vector2f center = view.getCenter();
    const sf::Vector2f size = view.getSize();
    const float left = center.x - size.x * 0.5f;
    const float right = center.x + size.x * 0.5f;
    const float top = center.y - size.y * 0.5f;
    const float bottom = center.y + size.y * 0.5f;
    
    // Convert to chunk bounds
    const ChunkCoord minChunk = worldPixelsToChunk(left, top);
    const ChunkCoord maxChunk = worldPixelsToChunk(right, bottom);
    
    // Draw only visible chunks, rebuilding dirty batches on-demand
    for (auto& kv : chunks_) {
        const ChunkCoord cc = kv.first;
        
        // Frustum culling: skip chunks outside view
        if (cc.x < minChunk.x - 1 || cc.x > maxChunk.x + 1 ||
            cc.y < minChunk.y - 1 || cc.y > maxChunk.y + 1) {
            continue;
        }
        
        Entry& entry = const_cast<Entry&>(kv.second); // Safe: only modifying batch state
        if (entry.batch.isDirty()) {
            entry.batch.build(entry.chunk, *atlas_);
        }
        t.draw(entry.batch, s);
    }
}

bool World::setTileAtTile(int tx, int ty, TileID id) {
    assert(atlas_ && "World requires a valid TileAtlas*");

    // Find chunk containing (tx, ty)
    // Use existing helpers: tile origin of chunk and CHUNK dims
    auto div_floor = [](int a, int b) {
        if (b == 0) return 0; // Handle division by zero
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
    ent.batch.markDirty(); // mark for rebuild instead of immediate rebuild
    return true;
}

bool World::setTileAtPixel(const sf::Vector2f& worldPx, TileID id) {
    // Validate input coordinates
    if (!std::isfinite(worldPx.x) || !std::isfinite(worldPx.y)) {
        return false; // Ignore NaN/infinity coordinates
    }
    
    // Validate tile ID
    if (id > Tile::Leaves) {
        return false; // Unknown tile type
    }
    
    const int tx = static_cast<int>(std::floor(worldPx.x / static_cast<float>(TILE_SIZE)));
    const int ty = static_cast<int>(std::floor(worldPx.y / static_cast<float>(TILE_SIZE)));
    return setTileAtTile(tx, ty, id);
}