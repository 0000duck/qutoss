# qutoss
Source code for quantum physics sandbox. Parsing done with ae, user-interface done with Qt, visualized in OpenGL. 

This is a work in progress, but I'm putting a temporary hiatus on it right now. 
Downloadable .app (MacOS) or .exe (Windows) files can be found in the "packaged" folder. 

The bulk of the numerical computation is implemented in "helpers.cpp" by the "nextStep, and is used in a velocity-verlet manner. 
The original paper with which I based my algorithm off of used a leap-frog implementation, which I found to be less accurate and led to more noise.

TO DO:
- improve user-interface
- try eigenfunction approach to solving time-dependent equation
- improve graphics to look more engaging
- add support for graphics-accelerated computation
