# qutoss
Source code for quantum physics sandbox. Parsing done with ae, user-interface done with Qt, visualized in OpenGL. 

This is a work in progress, but I'm putting a temporary hiatus on it right now. 
Downloadable .app (MacOS) or .exe (Windows) zipped files can be found in the "dist" folder. Admittedly, the packaging for Windows is not well done.

The bulk of the numerical computation is implemented in "src/helpers.cpp" by the "nextStep" function, and is used in a velocity-verlet manner. 
The original [paper](http://www.hunter.cuny.edu/physics/courses/physics485/repository/files/reference-papers/Visscher%20NumSol%20Schrodinger%20Eqt.pdf) with which I based my algorithm off of used a leap-frog implementation, which I found to be less accurate and led to more noise.

TO DO:
- improve user-interface
- try eigenfunction approach to solving time-dependent equation
- improve graphics to look more engaging
- add support for graphics-accelerated computation

<p align="left">
  <img src="https://github.com/boxofpasta/qutoss/blob/master/assets/samples/5.png" width="500">
</p>

<p align="left">
  <img src="https://github.com/boxofpasta/qutoss/blob/master/assets/samples/3.png" width="500">
</p>
