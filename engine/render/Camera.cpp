#include "engine/render/Camera.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

void Camera::init(sf::Vector2u size) {
    // Validate input size
    if (size.x == 0 || size.y == 0) {
        size = {800u, 600u}; // Fallback to reasonable default
    }
    view_.setSize(sf::Vector2f(size));
    view_.setCenter(sf::Vector2f(size) * 0.5f);
}

void Camera::onResize(sf::Vector2u size) {
    // Validate input size
    if (size.x == 0 || size.y == 0) {
        return; // Ignore invalid resize events
    }
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
    // Validate delta time
    if (!std::isfinite(dt) || dt < 0.f || dt > 1.f) {
        return; // Ignore invalid time steps
    }
    
    const float pan = panSpeed_ * dt;
    
    // Current position for bounds checking
    const sf::Vector2f currentCenter = view_.getCenter();
    constexpr float maxWorldCoord = 1000000.f; // Reasonable world limit
    
    // Apply movement with bounds checking
    sf::Vector2f newCenter = currentCenter;
    if (heldW_) newCenter.y -= pan;
    if (heldS_) newCenter.y += pan;
    if (heldA_) newCenter.x -= pan;
    if (heldD_) newCenter.x += pan;
    
    // Clamp to world bounds
    newCenter.x = std::clamp(newCenter.x, -maxWorldCoord, maxWorldCoord);
    newCenter.y = std::clamp(newCenter.y, -maxWorldCoord, maxWorldCoord);
    view_.setCenter(newCenter);

    // multiplicative zoom each frame while held with bounds
    if (heldQ_) {
        const float zoomFactor = std::max(0.001f, 1.f - zoomStep_);
        view_.zoom(zoomFactor);
    }
    if (heldE_) {
        const float zoomFactor = std::min(1000.f, 1.f + zoomStep_);
        view_.zoom(zoomFactor);
    }
}
