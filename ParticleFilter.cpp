
#include "ParticleFilter.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>



/* The approach is to first find out the possible positions the robot could be in. Initially, the robot can be at any position where there is a '.', thus fillParticle method fills all those positions in a temporary list called particleListDash (P') which is later filtered.
 Every time a new observation is received, it is checked if the previous and new orientation are same. If they are same, it indicates that the robot has moved forward. In case of different orientation, we assume that robot has changed its direction but has not moved.
 If the orientation is unknown, we check all four possibilities in the maze for the particles
 
 The issue that I encountered was allocating memory for the particleList and particleListDash such that there are accessible in all the functions without wasting any memory.
 This was resolved by taking dynamic list for the same as data member of the class.
 
 I chose to divide whole task into smaller functions or modules and kept them in private section of the class so that they can be called only through member function of the class. I have tried to use dynamic memory allocation wherever possible and deallocated the same when no longer required.
 
 The code seems to be efficient since I have followed the bottom-up approach and efficient memory management. I have checked all the unit tests and my code gives the expected results. However, there are some indirectly lost blocks in the memory leak summary which can be looked into.
 */

// Initialise a new particle filter with a given maze of size (x,y)
ParticleFilter::ParticleFilter(char** maze, int rows, int cols) {
    this->rows = rows;
    this->cols = cols;
    this->maze = maze;
    previousOrientation = ORIEN_UP;
    observationNumber = 0;
    particleList = new ParticleList();
    particleListDash = new ParticleList();
    
}

Grid ParticleFilter::getMaze()
{
    return maze;
}

// Clean-up the Particle Filter
ParticleFilter::~ParticleFilter() {
    particleList->clear();
    particleListDash->clear();
    delete particleList;
    delete particleListDash;
}

// Fetch the value of previous orientation
Orientation ParticleFilter::getPreviousOrientation()
{
    return previousOrientation;
}

// Change the value of previous orientation
void ParticleFilter::setPreviousOrientation(Orientation orientation)
{
    previousOrientation = orientation;
}

// Check the orientation of the current observation
Orientation  getOrientation(Grid observation)
{
    Orientation currentOrientation=0;
    if(observation[1][1] == '^')
        currentOrientation = ORIEN_UP;
    else if(observation[1][1] == '>')
        currentOrientation = ORIEN_RIGHT;
    else if(observation[1][1] == '<')
        currentOrientation = ORIEN_LEFT;
    else if(observation[1][1] == 'v')
        currentOrientation = ORIEN_DOWN;

    else {
        currentOrientation = UNKNOWN;
    }
    
    return currentOrientation;
}

// Check if the observation matches with a given position in maze
bool checkPosition(Grid observation, int diffx, int diffy,Grid maze)
{
    
    bool result = true;
    for(int i = 0; i< 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            // to avoid comparison of the orientation in the maze
            if(!(i == 1 && j == 1 && i == j))
            {
                // checks corresponding positions
                if(observation[i][j] != maze[i + diffy - 1][j + diffx -   1 ])
                    result = false;
            }
        }
    }
    return result;
}

// Rotate an observation by 90 degrees
void rotate90(Grid observation)
{
    char temp[3][3];
    int i, j;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            temp[i][j] = observation[2 - j][i];
        }
    }
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            observation[i][j] = temp[i][j];
        }
    }
}

// A new observation of the robot, of size 3x3
void ParticleFilter::newObservation(Grid observation) {
    
    observationNumber++;
  
    
    Orientation particleOrientation = getOrientation(observation);
    if(observationNumber == 1)
    {
        // call to fill initial list of particles
        fillParticles();
    }
    // Concludes robot has moved
    else if(previousOrientation == particleOrientation)
    {
        moveRobot();
        
    }
    // only the orientation is changed for the robot
    else {
       
        changeOrientation(particleOrientation);
        // change previousOrientation to be used in next iteration
        previousOrientation = particleOrientation;
    }
    

    // clear P before adding new set of values
    if(particleList->getNumberParticles()!= 0)
        particleList->clear();
  
    filterParticles(particleOrientation,observation);
   
    // free the memory for P' since that is a temporary list
    if(particleListDash->getNumberParticles()!= 0)
        particleListDash->clear();

    previousOrientation = particleOrientation;
}

// Return a DEEP COPY of the ParticleList of all particles representing
// the current possible locations of the robot
ParticleList* ParticleFilter::getParticles() {
    ParticleList *deepCopy;
    deepCopy = new ParticleList;
    for(int k = 0; k < particleList->getNumberParticles(); k++)
    {
        
        ParticlePtr ptr=particleList->get(k);
        
        // add particle with same position but changed orientation
        ParticlePtr newPtr = new Particle(ptr->getX(), ptr->getY(),ptr->getOrientation());
        deepCopy->add_back(newPtr);
    }
    return deepCopy;
}

//fill the temporary particle list with initial possible particle positions
void ParticleFilter::fillParticles()
{
    
    // Initialsing P
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            if(maze[i][j] == '.') // check for a possible position
            {
                ParticlePtr particle = new Particle(j,i,ORIEN_UP);
                // Add all the possible positions of the robot
                particleListDash->add_back(particle);
                
            }
        }
    }
    
}

//used to move the robot one step forward based on its orientation
void ParticleFilter::moveRobot()
{
    for(int k = 0; k < particleList->getNumberParticles(); k++)
    {
        
        ParticlePtr ptr=particleList->get(k);
        
        int presentX = ptr->getX();
        int presentY = ptr->getY();
        if(ptr->getOrientation() == ORIEN_UP)
        {
            presentY--; // moves up so value of y decreases
        }
        else if(ptr->getOrientation() == ORIEN_DOWN)
        {
            presentY++; //moves down so value of y increases
        }
        else if(ptr->getOrientation() == ORIEN_LEFT)
        {
            presentX--; //moves left so value of x decreases
        }
        else if(ptr->getOrientation() == ORIEN_RIGHT)
        {
            presentX++; //moves right so value of x increases
        }
        
        // Creates another particle and adds to the particleListDash,
        // this will find the new position
        ParticlePtr newPtr = new Particle(presentX, presentY, ptr->getOrientation());
        particleListDash->add_back(newPtr);
    }
}

//change the list of particles to contain their updated orientation
void ParticleFilter::changeOrientation(Orientation particleOrientation)
{
    for(int k = 0; k < particleList->getNumberParticles(); k++)
    {
        
        ParticlePtr ptr=particleList->get(k);
        
        int presentX = ptr->getX();
        int presentY = ptr->getY();
        
        // add particle with same position but changed orientation
        ParticlePtr newPtr = new Particle(presentX, presentY, particleOrientation);
        particleListDash->add_back(newPtr);
    }
}

// to create the final list for the current observation after eliminating the positions from
//particleListDash which does not match the orientation
//and copying only the matching ones to the ParticleList
void ParticleFilter::filterParticles(Orientation particleOrientation, Grid observation)
{
    for(int i = 0; i < particleListDash->getNumberParticles(); i++)
    {
        ParticlePtr ptr=particleListDash->get(i);
        
        int presentX = ptr->getX();
        int presentY = ptr->getY();
        
        if(particleOrientation != UNKNOWN)
        {
            if(checkPosition(observation, presentX, presentY,getMaze()))
            {
                ParticlePtr newPtr= new Particle(presentX,presentY,ptr->getOrientation());
                particleList->add_back(newPtr);
                
            }
        }
        else {
            for(int r = 1; r <= 4; r++)
            {
                if(checkPosition(observation, presentX, presentY,getMaze()))
                {
                    
                    ParticlePtr newPtr = new Particle(presentX,presentY,r % 4);
                    particleList->add_back(newPtr);
                    
                }
                // rotate the observation
                rotate90(observation);
            }
        }
    }
}
