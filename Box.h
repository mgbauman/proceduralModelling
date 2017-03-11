#ifndef BOX_H
#define BOX_H

#include "glm/glm.hpp"
#include <cstdio>

class Box {
public:
    Box(unsigned int, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront);

    //Add indices for the right side as well
    unsigned int topLeftBack;
    unsigned int topLeftFront;
    unsigned int bottomLeftBack;
    unsigned int bottomLeftFront;

    void updateLeft(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront);
};

#endif