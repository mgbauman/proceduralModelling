#ifndef BOX_H
#define BOX_H

#include "glm/glm.hpp"
#include <cstdio>

class Box {
public:
    Box();
    Box(unsigned int, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront);
    Box(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront, 
             unsigned int topRightBack, unsigned int topRightFront, unsigned int bottomRightBack, unsigned int bottomRightFront);
    ~Box();

    float sideLength;
    
    unsigned int topLeftBack;
    unsigned int topLeftFront;
    unsigned int bottomLeftBack;
    unsigned int bottomLeftFront;

    unsigned int topRightBack;
    unsigned int topRightFront;
    unsigned int bottomRightBack;
    unsigned int bottomRightFront;

    unsigned int topRightNose;
    unsigned int topLeftNose;
    unsigned int bottomRightNose;
    unsigned int bottomLeftNose;



    void updateLeft(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront);
    void updateRight(unsigned int topRightBack, unsigned int topRightFront, unsigned int bottomRightBack, unsigned int bottomRightFront);
    void updateNose();
    void updateNose(unsigned int topLeftNose, unsigned int topRightNose, unsigned int bottomLeftNose, unsigned int bottomRightNose);

};

#endif