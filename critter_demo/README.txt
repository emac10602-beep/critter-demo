Critter 3D Platformer Demo (DS/DSi)
===================================

How to build on macOS:
  1) Install devkitPro packages:
       brew install devkitpro-pacman
       sudo dkp-pacman -S nds-dev

  2) In Terminal, cd into this folder and run:
       make

  3) The build will produce: critter_demo.nds
     - Test in melonDS or DeSmuME
     - Or copy to your DSi SD card and run via TWiLight Menu++

Controls:
  D-Pad = move
  A = Jump / Glide
  B = Swat (attack)
  L/R = Orbit camera
  Y = Quick turn
  START = Reset to checkpoint
