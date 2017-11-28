#ifndef HELPERS_H
#define HELPERS_H

// the number of samples vs. time step must carefully be defined for stability of the algorithm
// to maintain stability, an increase in samples of factor k requires a decrease in time step of factor k
#define NUM_SAMPLES 39601 // ~199^2  //30625  // ~175^2
#define TIME_STEP 0.001
#define DIRAC_DELTA_RADIUS 0.05
#define EPSILON  1E-8       // used when comparing doubles, and for adding to division by zero issues
#define VERTEX_SIZE 6
#define ROUND_TO_ZERO 0.001   // NOT ok to actually round to zero, but anything below this constant is qualitatively negligible
#define PI 3.1415926535897
#define DEG_TO_RAD 0.0174533
#define RAD_TO_DEG 57.2958
#define CIRCLE_SIDES  50      // for graphics, the number of sides that we use to approximate a circle
#define CIRCLE_SIDES_HD  500    // just cause the brush tends to be a lot larger as a circle
#define SIDE_LENGTH 15.0
#define MAX_POTENTIAL 30  // the algorithm is stable for about 50 or 60, but lower limits help with accuracy
#define MAX_SPEED 6.5  // initial packet speed
#define MS_PER_FRAME 16.6667

#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <cmath>
#include <QFile>
#include <QPoint>
#include <QElapsedTimer>
#include <string>
#include <QString>
#include <QTextStream>
#include <QVector3D>
#include <QVector3D>
#include <QVector2D>
#include <QWidget>
#include <QVector>
#include <limits>
#include <unordered_map>
#include <complex>
#include <cmath>
#include "math.h"
#include "ae/ae.h"

using namespace std;

/* This file (lowest in the #include hierarchy) contains:
 * - helper functions, not specific to any class in particular (although mostly used by Data.cpp)
 *      - implementation of an explicit method for time-evolution according to Schrodinger's equation:
 *          - functions for calculating probability density of quantum Packet(s) given
 *          - the real and imaginary parts of the wave function are calculated as well
 *      - conversions between different coordinates used throughout the program
 * - definitions for structs used here and in other files
 * - ColorMapper class, useful for converting colors
 * */


typedef complex<double> cdouble;
typedef complex<double> cdouble;

// data to gl_vertex_buffer is transferred from an array of these structs
struct Point
{
    double x, y;
    double reBefore, reCur;
    double imBefore, imCur;   // saved data, might be helpful
    double V = 0, VPreview = 0;
    bool covered = false;     // deprecated: was for optimizing the number of drawn potential tiles

    // be warned: this value is not guaranteed to be up to date (to save us time)
    // this is a convenience member, use as storage space if you need to save this value
    // the intention is to store probability density here
    double refVal = 0;
};


// deprecated:
// the 3-D potential blocks that we draw to visualize barriers
// values can be converted to world coordinates using conversion functions defined in this file
struct Block
{
    int xLeft, xRight;
    int yLeft, yRight;
    double z;
};


// user-adjustable values (only used during initialization of wave-packet)
struct Packet
{
    double angle = 0.0, speed = 0;
    double xCen = 0.0, yCen = 0.0; //in world coordinates
    double precision = 1.0; //precision in position
};


// classical particle that obeys Newtonian physics
struct Particle
{
    double xVel = 0.0, yVel = 0.0;
    double xCen = 0.0, yCen = 0.0;
    double xCenBefore = 0.0, yCenBefore = 0.0;  // for external reading
};


struct Color
{
    double r, g, b;
    Color() {;}
    Color(double r, double g, double b) : r(r), g(g), b(b) {;}
    Color highlight() { return Color(r, g*0.5, b*0.5); }
    Color& operator*=(double factor) { r *= factor; g *= factor; b*= factor;
                                     return *this; }
};


inline Color operator*(Color c, const double factor)
{
    c.r *= factor;
    c.g *= factor;
    c.b *= factor;
    return c;
}

// a lot of these are currently unused, and ColorMapper only comes into play
// when flat drawing mode is enabled (otherwise handled by fragment shader code)
class ColorMapper {
public:
    ColorMapper();
    void getColorJet(double v,double vmin,double vmax, Color& out);
    void getColorHot(double v,double vmin,double vmax, Color& out);
    void getColorWinter(double v, double vmin, double vmax, Color& out);
    void getColorCool(double v, double vmin, double vmax, Color& out);
    void getColorSpring(double v, double vmin, double vmax, Color& out);
    void getColorAutumn(double v, double vmin, double vmax, Color& out);
    void getColorGray(double v, double vmin, double vmax, Color& out);
    void getColorInferno(double v, double vmin, double vmax, Color& out);

    // for linear interpolant argument must be between 0 and 1
    void getLinearInterpolated(double normalizedArg, const Color& colorFrom, const Color& colorTo, Color& out);
    void computeColor(double arg, Color& color, double min, double max, char cmap);

    // for potential blocks, a gray color scheme
    double potentialToColor(double height);

private:
    // stored color maps that were too difficult to hand-code
    string infernoPath, magmaPath;
    unordered_map<int, Color> inferno, magma;  // indexed from 0 - 255
};


enum Mode { R, I, R_RK, I_RK};

// for advancing the simulation (using explicit pseudo-verlet method)
cdouble computeInitial(double x, double y, const Packet& packet);
double computeNext(Mode mode, int x, int y, double dR, double dT, const QVector<QVector<Point>>& dataGrid);

// this would have been (slightly) faster had it been a method of Data,
// but this makes more programming sense as a more general computations function :
// takes the fast-fourier transform of a 2-D array (or image), stores by reference
template<typename T> void fftImage(const QVector<QVector<T>>& in, QVector<QVector<T>>& out);

// important: as of now (and possibly forever), only works with powers of 2
template<typename T> QVector<T> fftHelper(const QVector<T>& in, int start, int stepSize, char mode);


// CONVERSIONS:

// - world coordinates are what the math framework/ equation calculations are using
// - index coordinates are vertex indices
// - GL coordinates are openGL coordinates
// - screen coordinates are viewport coordinates in pixels, depends on widget size
double GLToWorld(double screenCoord);

QVector3D GLToWorld(const QVector3D& glCoord);

double worldToGL(double worldCoord);
QVector3D worldToGL(const QVector3D& worldCoord);

double indexToGL(int index, int samplesPerSide);
double indexToWorld(int index, double dR);
double indexToPotential(int z);
double potentialToGL(double potential);
double potentialToWorld(double potential);
double worldToPotential(double world);
double GLToPotential(double gl);
QVector2D scrnToGL(int xScrn, int yScrn, int w, int h);

// helper to Data::setVelocity
double stretchToSpeed(double stretch);


// MISC:

// to is the topleft corner of the widget destination
void slideWidget(QWidget* target, const QPoint& to, float speedup = 1.0);

// saves you from having to cout << line << endl ...
void print(const char* line);
void print(const string& line);
void print(const QString& line);

// 1D bicubic interpolator:
// to approximate a point that is distFroml far from point l, we need the values of the function at 4 points surrounding it
// the point in question must be right of l, and left of r
double interpolateBicubic(double ll, double l, double r, double rr, double distFroml);

// xDis and yDist are both measured from point 1,1 (if indexing starts from 0,0)
// they must be contained within the 4x4 grid defined by (1,1), (1,2), (2,1), (2,2)
double interpolateBicubic2D(const QVector<QVector<double>>& values, double xDist, double yDist);
double roundToPrecision(double arg, int precision);

// finds the first element that is greater than target within info, not used anymore
template<typename T> int binarySearch(const vector<T>& info, T target);

// define a pixel as the square bounded by 4 vertices :
// then scaling = 2 means splitting each pixel into 4, scaling = 3 is splitting into 16, etc...
// right now this function only deals with squares, so numOriginal needs to be a perfect square!
int numVerticesAfterScaling(int numOriginal, int scaling);
double clipPotential(double potential);

// both of the below functions return 3D points (as opengl is 3D) but z is always set to 0
// for drawing arrows... returns 3 points
// forward and latStretch can be any values, determine the shape and size of the triangle
QVector<QVector3D> getTriangle(const QVector3D& center, double rotationDeg, double forwardStretch, double latStretch);

// for drawing circles... returns a set of points, in counterclockwise order, that define a circle
// to be precise, this is an n-gon, n specified in argument
// axis = 0 is x, 1 is y, 2 is z :
// these are the axes on which the circle is skewered through (the vector normal to its surface)
QVector<QVector3D> getCircle(const QVector3D &center, double radius, int n, const QVector3D& axis);

// out is assumed to be large enough to hold all the vertices
// axis = 0 is x, 1 is y, 2 is z :
// these are the axes on which the circle is skewered through
void getCircle(const QVector3D& center, double radius, int n, QVector<QVector3D>& out, const QVector3D& axis);


#endif // HELPERS_H
