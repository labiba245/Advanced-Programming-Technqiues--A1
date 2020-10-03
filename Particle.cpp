
#include "Particle.h"

// x-co-ordinate of the particle
int Particle::getX() {
   return x;
}

// y-co-ordinate of the particle
int Particle::getY() {
   return y;
}

// Orientation of the particle
Orientation Particle::getOrientation() {
   return orientation;
}

Particle::Particle() {
    x = y= 0;
    orientation = ORIEN_UP;
}
//Parameterised constructor
Particle::Particle(int x,int y,Orientation orientation) {
    this->x = x;
    this->y = y;
    this->orientation = orientation;
}
//void Particle::setX(int x)  {
//    this->x = x;
//}
//void Particle::setY(int y) {
//    this->y = y;
//}
//void Particle::setOrientation(Orientation orientation) {
//    this->orientation = orientation;
//}
