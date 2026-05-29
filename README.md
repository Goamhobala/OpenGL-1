This assignment involves using OpenGL to create a simple Sun-Earth-Moon systemm
whereby they revovle around each other at different speed and angles. Each planet will have its own texture. The lighting effects are implemented using the Phong Model.

## Moving Light source

Notably, for the second, moving light source, I added a [brown dwrarf](https://en.wikipedia.org/wiki/Brown_dwarf), which is a kind of self-luminous object that failed to become a star. Here I simply treated it as an additional planet in the solar system that rotates around the Sun. It is not physically correct, but I made it followed the same orbit as Jupyter, and placed it directly across it, acting as Jupyter's evil twin.

The brown dwraf emits a pink light. To make the effect more obvious, I have made the brown dwarf's and Jupiter's angular velocities 5 times greater than the rest of the planets.

## Setup

I used cmake for this. It is my first time managing dependencies with it so this is the only way I know to make it work.

```
mkdir build
mv ./src/simple.* ./build
cd build
cmake ..
make
./Assignment1
```

## Extension

For extension, I implemented the first category: `Additional planets with textures, and better scaling to separate out the planets. In this case, a fixed rotation speed for each planet could be used.`

Additionally, I added a brown dwarf (a failed star) to our solar system, acting as a second light source rotating around the Sun. In reality, the physical orbits should change, but here I just treated it as an additional planet.

I downloaded the [planet textures](https://www.solarsystemscope.com/textures/) from the link provided, and completed our solar systems with all 9 major planets, in the correct order. Note that the scalings are optimised for visual effects rather than pysical accuracy, which would make the smaller planets too small to be visible.


## Controls

### Camera Control

The camera orbits are implemented using Euler angles, the controls are listed below.

w - increase the pitch rotate the camera around the x-axis, counter-clockwise

s - decrease the pitch, rotate the camera around the x-axis, clockwise

a - increase the yaw, rotate the camera around the y-axis, clockwise

d - decrease the yaw, rotate the camera around the y-axis, counter-clockwise

q - increase the roll, rotate the camera around the z-axis, clockwise

e - decrease the roll, rotate the camera around the z-axis, counter-clockwise

### Planet Rotation

Space - To pause and unpause the animation
Up - To increase the angular velocity of the major planets
Down - To decrease the angular velocity of the major planets
Left - To increase the angular velocity of the Moon
Right - To decrease the angular velocity of the Moon
