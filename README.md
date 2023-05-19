# Gravity Evolution

## Dependencies
### GLFW
### GLAD
### GLM

## Desicription
This program simulates a set of bodies under newtonian gravity. The bodies are all of unit mass and the gravitational constant is expressed in the simulation space distance unit per second squared. It uses threads to efficiently calculate the accelerations of the bodies. This is necessarily an operation of quadratic time complexity. A considerable approach is to speed up the process is to halve the number of acceleration term calculations by applying it to both bodies. It is unclear how this would be parallelized without purpose-defeating level of synchronization, so threading is the way to go. The only synchronization is between simulation steps. The vertex buffer is reused for storing the state of the system as well, and simply swapped with the next state.

## How to Use
Use WASD for camera yaw and pitch, and QE for roll. Arrow keys along with right control and shift translates the system.
Vary the independent values in constants.hpp as you please!
