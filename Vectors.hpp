#pragma once
#include <cmath>

struct Vector2D {
    float x, y;

    // Konstanten für Vektoren
    static const Vector2D ZERO;
    static const Vector2D ONE;
    static const Vector2D UNIT_X;
    static const Vector2D UNIT_Y;

    // Konstruktor
    Vector2D(float x_val = 0, float y_val = 0) : x(x_val), y(y_val) {}

    // Addition
    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    // Subtraktion
    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    // Multiplikation mit Skalar
    Vector2D operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    // Division durch Skalar
    Vector2D operator/(float scalar) const {
        if (scalar != 0) {
            return Vector2D(x / scalar, y / scalar);
        } else {
            // Behandlung von Division durch Null
            return *this;
        }
    }

    // Normalisierungsfunktion
    Vector2D Normalized() const {
        float length = Magnitude();
        if (length != 0) {
            return *this / length;
        } else {
            return *this;
        }
    }

    // Dot-Produkt
    float DotProduct(const Vector2D& other) const {
        return x * other.x + y * other.y;
    }

    // Magnitude (Länge)
    float Magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    // Distance (Abstand zu einem anderen Vektor)
    float Distance(const Vector2D& other) const {
        return (*this - other).Magnitude();
    }

    // Überprüfen, ob der Vektor Null ist
    bool IsZeroVector() const {
        return x == 0 && y == 0;
    }
};

// Konstanteninitialisierung
const Vector2D Vector2D::ZERO = Vector2D(0, 0);
const Vector2D Vector2D::ONE = Vector2D(1, 1);
const Vector2D Vector2D::UNIT_X = Vector2D(1, 0);
const Vector2D Vector2D::UNIT_Y = Vector2D(0, 1);


struct Vector3D {
    float x, y, z;

    // Konstruktor
    Vector3D(float x_val = 0, float y_val = 0, float z_val = 0) : x(x_val), y(y_val), z(z_val) {}

    // Addition
    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    // Subtraktion
    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    // Multiplikation mit Skalar
    Vector3D operator*(float scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    // Division durch Skalar
    Vector3D operator/(float scalar) const {
        if (scalar != 0) {
            return Vector3D(x / scalar, y / scalar, z / scalar);
        } else {
            // Behandlung von Division durch Null
            return *this;
        }
    }

    // Normalisierungsfunktion
    Vector3D Normalized() const {
        float length = Magnitude();
        if (length != 0) {
            return *this / length;
        } else {
            return *this;
        }
    }

    // Magnitude (Länge)
    float Magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Distance (Abstand zu einem anderen Vektor)
    float Distance(const Vector3D& other) const {
        return (*this - other).Magnitude();
    }

    // Überprüfen, ob der Vektor Null ist
    bool IsZeroVector() const {
        return x == 0 && y == 0 && z == 0;
    }
};
