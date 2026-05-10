This assignment involves using OpenGL to create a simple Sun-Earth-Moon systemm
whereby they revovle around each other at different speed and agnles

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

## Representations

Sun - Red
Earth - Green
Moon - Blue

## Controls

Space - To pause and unpause the animation
Up - To increase the angular velocity of the Earth
Down - To decrease the angular velocity of the Earth
Left - To increase the angular velocity of the Moon
Right - To decrease the angular velocity of the Moon
