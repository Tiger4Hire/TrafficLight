#pragma once
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdexcept>

class GluSphere {
    GLUquadric* light_sphere;

public:
    GluSphere() noexcept : light_sphere(gluNewQuadric()) {}
    ~GluSphere()
    {
        if (light_sphere)
            gluDeleteQuadric(light_sphere);
    }
    void Render(GLdouble size, int slices) const
    {
        if (!light_sphere)
            throw std::runtime_error("gluDeleteQuadric failed");
        gluSphere(light_sphere, size, slices, slices);
    }
};
