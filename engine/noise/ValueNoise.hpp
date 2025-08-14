#pragma once
#include <cstdint>
#include <cmath>

// 64-bit mix (SplitMix64)
static inline std::uint64_t smix(std::uint64_t x) {
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

static inline float hash01(std::uint64_t k) {
    // take upper 24 bits -> [0,1)
    return (float)((smix(k) >> 40) & 0xFFFFFFull) / 16777216.0f;
}

static inline std::uint64_t pack2i(std::int64_t x, std::int64_t y, std::uint64_t seed) {
    return (std::uint64_t)(x) * 0x9E3779B185EBCA87ull
         ^ (std::uint64_t)(y) * 0xC2B2AE3D27D4EB4Full
         ^ (seed * 0x165667B19E3779F9ull);
}

static inline float rand2i(std::int64_t xi, std::int64_t yi, std::uint64_t seed) {
    return hash01(pack2i(xi, yi, seed));
}

static inline float smooth(float t) { // quintic
    return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}

// 2D value noise with smooth bilinear interpolation
static inline float valueNoise2(float x, float y, std::uint64_t seed) {
    const float xf = std::floor(x), yf = std::floor(y);
    const int   xi = (int)xf,        yi = (int)yf;
    const float tx = x - xf,         ty = y - yf;

    const float r00 = rand2i(xi+0, yi+0, seed);
    const float r10 = rand2i(xi+1, yi+0, seed);
    const float r01 = rand2i(xi+0, yi+1, seed);
    const float r11 = rand2i(xi+1, yi+1, seed);

    const float sx = smooth(tx), sy = smooth(ty);
    const float a  = r00 + (r10 - r00) * sx;
    const float b  = r01 + (r11 - r01) * sx;
    return a + (b - a) * sy;
}

// Simple FBM
static inline float fbm2(float x, float y, std::uint64_t seed,
                         int octaves = 4, float lacunarity = 2.f, float gain = 0.5f)
{
    float f = 0.0f, amp = 0.5f, fx = x, fy = y;
    for (int i = 0; i < octaves; ++i) {
        f  += valueNoise2(fx, fy, seed + (std::uint64_t)i * 1315423911u) * amp;
        fx *= lacunarity; fy *= lacunarity; amp *= gain;
    }
    return f; // ~[0,1)
}
