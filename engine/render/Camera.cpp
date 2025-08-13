#include "engine/render/Camera.hpp"
#include <algorithm>

void Camera::init(sf::Vector2u size) {
    view_.setSize(sf::Vector2f(size));
    view_.setCenter(sf::Vector2f(size) * 0.5f);
}

void Camera::onResize(sf::Vector2u size) {
    const auto c = view_.getCenter();
    view_.setSize(sf::Vector2f(size));
    view_.setCenter(c);
}

void Camera::handleEvent(const sf::Event& ev) {
    if (auto r = ev.getIf<sf::Event::Resized>()) {
        onResize({r->size.x, r->size.y});
        return;
    }
    if (auto kp = ev.getIf<sf::Event::KeyPressed>()) {
        switch (kp->scancode) {
            case sf::Keyboard::Scan::W: heldW_ = true; break;
            case sf::Keyboard::Scan::A: heldA_ = true; break;
            case sf::Keyboard::Scan::S: heldS_ = true; break;
            case sf::Keyboard::Scan::D: heldD_ = true; break;
            case sf::Keyboard::Scan::Q: heldQ_ = true; break;
            case sf::Keyboard::Scan::E: heldE_ = true; break;
            default: break;
        }
        return;
    }
    if (auto kr = ev.getIf<sf::Event::KeyReleased>()) {
        switch (kr->scancode) {
            case sf::Keyboard::Scan::W: heldW_ = false; break;
            case sf::Keyboard::Scan::A: heldA_ = false; break;
            case sf::Keyboard::Scan::S: heldS_ = false; break;
            case sf::Keyboard::Scan::D: heldD_ = false; break;
            case sf::Keyboard::Scan::Q: heldQ_ = false; break;
            case sf::Keyboard::Scan::E: heldE_ = false; break;
            default: break;
        }
        return;
    }
}

void Camera::update(float dt) {
    const float pan = panSpeed_ * dt;
    if (heldW_) view_.move({0.f, -pan});
    if (heldS_) view_.move({0.f,  pan});
    if (heldA_) view_.move({-pan, 0.f});
    if (heldD_) view_.move({ pan, 0.f});

    // multiplicative zoom each frame while held
    if (heldQ_) view_.zoom(std::max(0.01f, 1.f - zoomStep_));
    if (heldE_) view_.zoom(1.f + zoomStep_);
}
