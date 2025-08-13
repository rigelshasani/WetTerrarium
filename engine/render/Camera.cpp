#include "engine/render/Camera.hpp"

void Camera::init(sf::Vector2u size) {
    view_.setSize(sf::Vector2f(size));
    view_.setCenter(sf::Vector2f(size) * 0.5f);
}

void Camera::onResize(sf::Vector2u size) {
    auto c = view_.getCenter();
    view_.setSize(sf::Vector2f(size));
    view_.setCenter(c);
}
