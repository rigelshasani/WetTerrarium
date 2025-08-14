#pragma once
#include <SFML/Graphics.hpp>
#include "engine/tile/Chunk.hpp"
#include "engine/tile/TileAtlas.hpp"

class TileBatch : public sf::Drawable {
public:
    void build(const Chunk& chunk, const TileAtlas& atlas);
    void updateRegion(const Chunk& chunk, const TileAtlas& atlas, 
                     unsigned minX, unsigned minY, unsigned maxX, unsigned maxY);
    
    bool isDirty() const { return isDirty_; }
    void markDirty() { isDirty_ = true; }
    void markClean() { isDirty_ = false; }

private:
    sf::VertexArray     va_{sf::PrimitiveType::Triangles};
    sf::Vector2f        pixelOffset_{0.f, 0.f};   // <- REQUIRED
    const sf::Texture*  tex_ = nullptr;           // <- REQUIRED
    bool                isDirty_ = false;

    static void addQuad(sf::VertexArray& va,
                        float x, float y, float s,
                        const sf::IntRect& uv, sf::Color color = sf::Color::White);

    void draw(sf::RenderTarget& t, sf::RenderStates s) const override {
        s.transform.translate(pixelOffset_);
        s.texture = tex_;
        t.draw(va_, s);
    }
};
