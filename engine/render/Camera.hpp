#pragma once
#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera() = default;

    void init(sf::Vector2u size);
    void onResize(sf::Vector2u size);

    void move(sf::Vector2f delta) { view_.move(delta); }
    void zoom(float factor) { view_.zoom(factor); }

    const sf::View& view() const { return view_; }

private:
    sf::View view_;
};
