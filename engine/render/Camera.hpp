#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Camera {
public:
    Camera() = default;

    void init(sf::Vector2u size);
    void onResize(sf::Vector2u size);

    // Feed events so the camera tracks pressed/released scancodes (SFML 3)
    void handleEvent(const sf::Event& ev);

    // Move/zoom based on held keys; call once per frame with dt (seconds)
    void update(float dt);

    // Apply this camera's view to a window before drawing world-space items
    void applyTo(sf::RenderWindow& window) const { window.setView(view_); }

    // Optional tuning
    void setPanSpeed(float pxPerSec) { panSpeed_ = pxPerSec; }
    void setZoomStep(float step)     { zoomStep_ = step; } // e.g. 0.01f

    const sf::View& view() const { return view_; }

private:
    sf::View view_;
    float panSpeed_ = 300.f;  // pixels/second
    float zoomStep_ = 0.01f;  // multiplicative step per update when held

    // held-state (layout-agnostic scancodes)
    bool heldW_ = false, heldA_ = false, heldS_ = false, heldD_ = false;
    bool heldQ_ = false, heldE_ = false;
};
