#include "Box.h"
#include "glm\glm.hpp"

using namespace glm;

Box::Box(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront){
    this->topLeftBack = topLeftBack;
    this->bottomLeftFront = topLeftFront;
    this->bottomLeftBack = bottomLeftBack;
    this->bottomLeftFront = bottomLeftFront;
}

void Box::updateLeft(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront) {
    this->topLeftBack = topLeftBack;
    this->bottomLeftFront = topLeftFront;
    this->bottomLeftBack = bottomLeftBack;
    this->bottomLeftFront = bottomLeftFront;
}