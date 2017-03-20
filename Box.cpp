#include "Box.h"
#include "glm\glm.hpp"

using namespace glm;

Box::Box() {
}

Box::Box(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront){
    this->topLeftBack = topLeftBack;
    this->topLeftFront = topLeftFront;
    this->bottomLeftBack = bottomLeftBack;
    this->bottomLeftFront = bottomLeftFront;
}

Box::Box(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront, unsigned int topRightBack, unsigned int topRightFront, unsigned int bottomRightBack, unsigned int bottomRightFront) {
    this->topLeftBack = topLeftBack;
    this->topLeftFront = topLeftFront;
    this->bottomLeftBack = bottomLeftBack;
    this->bottomLeftFront = bottomLeftFront;

    this->topRightBack = topRightBack;
    this->topRightFront = topRightFront;
    this->bottomRightBack = bottomRightBack;
    this->bottomRightFront = bottomRightFront;
}

Box::~Box(){}

void Box::updateLeft(unsigned int topLeftBack, unsigned int topLeftFront, unsigned int bottomLeftBack, unsigned int bottomLeftFront) {
    this->topLeftBack = topLeftBack;
    this->topLeftFront = topLeftFront;
    this->bottomLeftBack = bottomLeftBack;
    this->bottomLeftFront = bottomLeftFront;
}

void Box::updateRight(unsigned int topRightBack, unsigned int topRightFront, unsigned int bottomRightBack, unsigned int bottomRightFront) {
    this->topRightBack = topRightBack;
    this->topRightFront = topRightFront;
    this->bottomRightBack = bottomRightBack;
    this->bottomRightFront = bottomRightFront;
}


void Box::updateNose() {
    this->topRightNose = this->topRightFront;
    this->topLeftNose = this->topLeftFront;
    this->bottomRightNose = this->bottomRightFront;
    this->bottomLeftNose = this->bottomLeftFront;
}


void Box::updateNose(unsigned int topLeftNose, unsigned int topRightNose, unsigned int bottomLeftNose, unsigned int bottomRightNose) {
    this->topRightNose = topRightNose;
    this->topLeftNose = topLeftNose;
    this->bottomRightNose = bottomRightNose;
    this->bottomLeftNose = bottomLeftNose;
}