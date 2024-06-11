#ifndef VEC_H
#define VEC_H

#include <string>

namespace vec {
    struct vec2 {
        float x, y;
    };

    struct vec3 {
        float x, y, z;
    };

    struct vec4 {
        float x, y, z, w;
    };

    inline vec2 operator+(const vec2& lhs, const vec2& rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    inline vec2 operator-(const vec2& lhs, const vec2& rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    inline vec2 operator*(const vec2& lhs, const vec2& rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y};
    }

    inline vec2 operator/(const vec2& lhs, const vec2& rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y};
    }
    inline bool operator>(const vec2& lhs, const vec2& rhs) {
        return lhs.x > rhs.x && lhs.y > rhs.y;
    }

    inline bool operator==(const vec2& lhs, const vec2& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    inline bool operator!=(const vec2& lhs, const vec2& rhs) {
        return lhs.x != rhs.x && lhs.y != rhs.y;
    }

    inline vec3 operator+(const vec3& lhs, const vec3& rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
    }

    inline vec3 operator-(const vec3& lhs, const vec3& rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    }

    inline vec3 operator*(const vec3& lhs, const vec3& rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
    }

    inline vec3 operator/(const vec3& lhs, const vec3& rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
    }

    inline vec4 operator+(const vec4& lhs, const vec4& rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
    }

    inline vec4 operator-(const vec4& lhs, const vec4& rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
    }

    inline vec4 operator*(const vec4& lhs, const vec4& rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w};
    }

    inline vec4 operator/(const vec4& lhs, const vec4& rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w};
    }

    struct vec2Hash {
        std::size_t operator()(const vec::vec2& k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            return ((hash<float>()(k.x)
                     ^ (hash<float>()(k.y) << 1)) >> 1);
        }
    };
}

#endif // VEC_H