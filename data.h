#ifndef DATA_H
#define DATA_H

#define MAX_NUM_PARTICLES 30
#define TIME_LIMIT 100.0    // allotted per simulation

#include <QVector>
#include <QVector3D>
#include <QString>
#include <qopengl.h>
#include <QOpenGLBuffer>
#include <QObject>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>
#include <QMutex>
#include <QWaitCondition>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <chrono>
#include <thread>
#include <random>
#include <list>
#include <iomanip>
#include "helpers.h"
#include "equationparser.h"

/* This file:
 * - contains necessary data and methods for simulations
 * - contains and manipulates vertex/element data (but does not write to buffer)
 *      - QGL uses this buffer data to draw
 * - uses methods to compute time-evolution of system
 * - is the end handler for setting position/velocity/find closest points
 * - defines methods used for computing time-evolution used for classical particles
*/


class Data : public QObject
{
    Q_OBJECT
public:
    Data(QObject* parent) {;}
    Data();
    ~Data() {;}

    // used only for 2 stages of the tutorial:
    // sets a potential field, along with velocity
    void useFixedSettings();

    // to keep arrow representation correct, must be updated when packet center moves
    void updateArrow();
    void updateParticle(const QVector3D& cameraPos, const Color& color);
    void updateGrid(char drawMode, bool flat, char probsCmap = 'H');
    void updateBrush();  // the brush indicator is a 2D circle approximating the area-of-effect

    unsigned getGridVerticesLength() { return (unsigned)gridVertices.length();}
    unsigned getGridElementsLength() { return (unsigned)gridElements.length();}
    unsigned getTileVerticesLength() { return (unsigned)tileVertices.length();}
    unsigned getTileElementsLength() { return (unsigned)tileElements.length();}
    unsigned getFuncMeshVerticesLength() { return (unsigned)funcMeshVertices.length();}
    unsigned getFuncMeshElementsLength() { return (unsigned)funcMeshElements.length();}
    unsigned getIndicatorVerticesLength() { return (unsigned)indicatorVertices.length();}
    unsigned getBrushElementsLength() { return (unsigned)brushElements.length();}
    unsigned getParticleVerticesLength() { return (unsigned)particleVertices.length();}
    unsigned getParticleElementsLength() { return (unsigned)particleElements.length();}
    unsigned getNetVerticesLength() { return (unsigned)netVertices.length();}
    unsigned getNetElementsLength() { return (unsigned)netElements.length();}
    unsigned getNumExistingTiles() { return tileNum;}
    unsigned getSamplesPerSide() { return samplesPerSide;}
    unsigned getNumParticles() { return numParticles;}
    double getSimulationTime() { return elapsedTime;}
    double getRemainingTime() { return TIME_LIMIT - elapsedTime;}
    double getElapsedTime() { return elapsedTime;}
    double getTileWidth() { return dR;}
    double getProbability(int x1, int x2, int y1, int y2);
    double getNettedProbability() { return getProbability(netAnchor.x(), netStretch.x(), netAnchor.y(), netStretch.y());}
    double getPacketSpeed() { return initialPacket.speed;}
    double getPacketAngle() { return initialPacket.angle;}
    double getPacketPrecision() { return initialPacket.precision;}
    QVector3D getPacketCenter() { return {float(initialPacket.xCen), float(initialPacket.yCen), 0.0};}   // in world coordinates

    // the calls exist for use with OpenGL buffers
    // gridVertices is altered by either 'initGridVertices' or 'updateGrid'
    const GLfloat* getGridVertices() { return gridVertices.constData();}
    const GLuint* getGridElements() { return gridElements.constData();}
    const GLfloat* getTileVertices() { return tileVertices.constData();}
    const GLuint* getTileElements() { return tileElements.constData();}
    const GLfloat* getFuncMeshVertices() { return funcMeshVertices.constData();}
    const GLuint* getFuncMeshElements() { return funcMeshElements.constData();}
    const GLfloat* getIndicatorVertices() { return indicatorVertices.constData();}
    const GLuint* getBrushElements() { return brushElements.constData();}
    const GLfloat* getParticleVertices() { return particleVertices.constData();}
    const GLuint* getParticleElements() { return particleElements.constData();}
    const GLfloat* getNetVertices() { return netVertices.constData();}
    const GLuint* getNetElements() { return netElements.constData();}

    // the setters here are divided into two types:
    // 1) set by mouse coordinates
    // 2) set by values from other widgets
    // lastly, note that for changes to appear on screen, signal must be emitted to glwidget to write to buffers
    void setSimSpeed(unsigned speed) { simSpeed = speed; }
    void setSensitivity(int s) { jetMax = 1.0 - double(s)/100.0; } // s in range [0.0 , 0.9]
    string setEquation(const QString& s);

    // uses current settings of brushHeight and brushSize
    void setBrushParams(double brushHeight, double brushPrecision);
    void paintPotential(const QVector3D& start, const QVector3D& direction);
    void paintPotentialHelper(const QVector3D& worldPoint, double precision, double height, double radius = 1.0);  // add a smooth potential centered around GLPoint
    void resetAnchorAndStretch();
    void setBrush(const QVector3D& start, const QVector3D& direction);
    void setAnchor(const QVector3D& start, const QVector3D& direction);  // GL coordinates (e.g on-screen plane ranges from x = -1 to 1)
    void setStretch(const QVector3D& start, const QVector3D& direction);  // for overview of coordinate systems refer to computations.h
    void setNetAnchor(const QVector3D& start, const QVector3D& direction);
    void setNetStretch(const QVector3D& start, const QVector3D& direction);

    // naming scheme :  'set' prefix is from external sources, to set to new value
    //                  'update' prefix is to update vertices/data to be uploaded to glbuffers based on current info
    // bad name : "Stretch" refers to the GL stretch value that the cursor is currently exerting,
    // not the stretch from the anchor that defines the rectangle
    void setPotentialFromStretch(double vertStretch);
    void setCenter(const QVector3D& GLPoint, bool preview = true);
    void setVelocity(const QVector3D& GLPoint);  // distance from center to p = speed, direction to p from center = direction
    void setCenter(double xCen, double yCen, bool preview = true);
    void setXCen(double xCen);   // these functions are used by other widgets to set values, redirects to use 'setCenter'
    void setYCen(double yCen);
    void setSpeed(double speed);
    void setAngle(double angleDeg);
    void setPrecision(double precision);
    void setArrow(const QVector3D& endPoint); // visualization for velocity
    void saveCurrentState() { initialPacketSaved = initialPacket;} // save snapshot of Packet that we can return to later
    void restoreSaved();
    void updateBrushRadius();   // using brushHeight and brushPrecision
    bool withinPacketVicinity(const QVector3D& GLPoint); // checks if the click was close enough to packet center
    bool withinArrowVicinity(const QVector3D& GLPoint);

    // setAnchor and setStretch call previewField();
    // if setTiles is called with previewVersion, what is currently seen on screen will be used (the preview)
    // otherwise the unfinished settings are cleared, and the original field is used
    void setTiles(bool previewVersion, bool setAll = false);
    void setMesh(bool previewVersion);     // updates mesh vertices based on current potential, either preview or actual
    void previewField();
    void confirmTile();  // adds the preview potential within block defined by anchor and stretch
    void confirmPacket();
    void observe(double precision, double timeLimit = TIME_LIMIT);  // sets the elapsed time to be high: don't want sim to run for much longer

    // clears and reorders everything in the element buffer
    void updateTileOrder(bool preview = true);

    // will lock up gridData for a brief moment (to copy data to backend) if mutex != NULL
    void advanceSimulation(QMutex* chicken = NULL, bool onlyClassical = false);
    void verletQuantum(double timeStep);     // not really anything special, as its name might imply
    void pefrlQuantum(double timeStep);
    void resetSimulation();

    // for the dual-simulation with classical particles
    // verlet can be used by itself, but we will call it (3x) as part of forest-ruth algorithm
    void verletClassical(double timeStep);
    void advanceSimulationClassical();

// for demanding opengl buffer updates,
// the functions responsible for emitting signals:
// updateGrid, setArrow, setTiles, setPrecision, (setCenter calls setArrow)
signals:
    void updateGridBuffers();   // only ever need to update vertex buffer
    void updateTileVbo();
    void updateMeshBuffers();
    void updateIndicatorBuffers();
    void updateParticleBuffers();
    void updateNetBuffers();
    void updateTileEbo();

private:
    // constants for Forest-Ruth (stolen from https://blog.frogslayer.com/symplectic-algorithms-what-you-need-to-know/)
    // these turned out to not be so useful ... velocity-verlet gives a high enough accuracy, the rest is in the grid-spacing
    const double frCoefficient = 1.0f / (2.0f - (double)pow(2.0, 1.0 / 3.0));
    const double frComplement = 1.0f - 2.0f / (2.0f - (double)pow(2.0, 1.0 / 3.0));
    const double squiggle = 0.1786178958448091;
    const double lambda = -0.2123418310626054;
    const double zeta = -0.06626458266981849;

    int placementSpacing = 3;
    int resolution = 2;  // the number of squares we use on each side is times 2, any value > 2 is not supported atm.
    unsigned simSpeed = 1;
    unsigned samplesPerSide;
    unsigned tileNum = 0;    // helps us keep track of where we are in tileVertices
    unsigned numParticles = 1;  // no longer makes any sense to have more than 1 particle (we only need one for classical analogy)
    double elapsedTime = 0;   // simulation world time (not done with QTime or std)
    double dR;
    double factor = 0.15;  // additional variable to control brushPrecision
    const double minPrecision = 0.5, maxPrecision = 25;    // if you change these values, it's important that you alter the appropriate QSlider as well
    double brushPrecision = 0.5, brushHeight = 1.0/24.0;   // precision from [0.5, 25] ; height from [2.04, -2.04]
    double brushRadius = 0.1;    // will help save time : anything outside of calculated radius is dropped to 0
    double jetMax = 0.02;   // the value at which the jet color scheme hits dark red (an endpoint on the color spectrum)
    int potential = 2;
    ColorMapper cmap;
    QVector<GLfloat> gridVertices;
    QVector<GLfloat> tileVertices;
    QVector<GLfloat> funcMeshVertices;
    QVector<GLfloat> indicatorVertices;  // contains vertices for both the arrow and brush circle
    QVector<GLfloat> particleVertices;   // particleVertices[0] denotes the center of the circle
    QVector<GLfloat> netVertices;  // probability net
    QVector<GLuint> gridElements;
    QVector<GLuint> tileElements;
    QVector<GLuint> netElements;
    QVector<GLuint> funcMeshElements;
    QVector<GLuint> particleElements;   // needed for drawing a circle
    QVector<GLuint> brushElements;      // uses indicator vertices
    QVector<QVector<Point>> gridData;
    QVector<QVector<int>> simpsonCoeffs;  // for 2D simpsons rule
    Particle particle;
    QPoint anchorBlockID, stretchBlockID;  // raw outputs from find closest indices
                                           // they are exactly equal to anchor and stretch if spacing = 1
                                           // the block ids are the topleft index of the block which they represent
    QPoint anchor, prevAnchor;   // together, anchor and stretch determine the tile to-be-placed
    QPoint stretch, prevStretch;  // note that QPoint holds the indices within gridData (int, int)
    QPoint netAnchor;  // same ideas as above, but used for probability net
    QPoint netStretch;
    QPoint center;

    // for multithreading advanceSimulation
    QMutex linker;
    QWaitCondition manager;
    int activeSimThreads = 1;   // by default, there has to be at least 1

    // both in world coordinates, must be converted if working in normalized GL coordinates
    QVector3D arrowCenter;
    QVector3D brushCenter;

    QVector<QVector3D> particlePerimeter, brushPerimeter;
    Packet initialPacket, initialPacketSaved;
    string equation;
    EquationParser parser;

    // keeps track of which tiles are at which potential levels
    // use a BST for this, the keys will be the potential value, up to 2 sig figs
    multimap<int, int> tileOrganizer;

    // each element corresponds with a point in gridData
    // each element denotes the end of current bucket (previous element denotes start of it)
    // to sample, use a value from 0 - 1.0 and do a binary search for the bucket containing that
    vector<double> buckets;

    // if anchor and stretch are solely from findClosestIndices and unrefined,
    // block placement overlap issues will arise : i.e we refine the bounds s.t there is never
    // any unavoidable overlaps
    pair<QPoint, QPoint> getAnchorAndStretch(const QPoint& curAnchor, const QPoint& curStretch, int spacing);

    // given mouse ray (start + direction), finds the closest allowable indices to its intersection with the surface
    // arguments are in glCoords, caller should only be from glWidget or camera classes
    QPoint findClosestIndices(const QVector3D &start, const QVector3D &direction, int spacing);

    // helper function for findClosestIndices:
    // finds the closest point with the specified potential, with search centered around start
    // radius is specified in index coordinates
    QPoint searchProximity(const QPoint& start, int target, int maxRadius = 10);

    // flat version: height doesn't matter
    // finds closest indices on z = 0 plane
    // argument may be normalized (gl) or world coordinates, default is gl coordinates
    QPoint findClosestIndicesFlat(const QVector3D& coords, int spacing, bool worldCoords = false);

    // argument is default glCoords
    QPoint findClosestIndicesFlat(const QVector3D& start, const QVector3D &direction, int spacing);

    // call with preview = true in any other case (won't be simulation-ready but changes are reflected)
    void initGridData();
    void initGridVertices();
    void initTileVertexData();
    void initMeshVertices();

    // element data does not change over program lifetime
    void initGridElementData();
    void initTileElementData();
    void initNetElementData();
    void initFuncMeshElementData();
    void initParticleElementData();
    void initBrushElementData();
    void initSimpsonCoeffs();

    // works with tileVertices only: sets specified region to a certain height
    // if outline flag is true, height argument does not matter
    void setTile(int tileNum, int x1, int x2, int y1, int y2, double height, bool outline);
    void setNet(int x1, int x2, int y1, int y2, double height);

    // deprecated:
    // helper function to find a rectangle at the given point: if no rectangle was found, return false.
    // the information for the rectangle is stored via reference
    bool findRectangle(Block& tile, int x, int y, bool preview);

    // finds 4 points (on dataGrid) that contain the specified point, if impossible returns false
    // points are specified as indices within gridData
    bool findBoundingGrid(QVector<QPoint>& bounds, double xWorld, double yWorld);

    // to get correct results, ensure that xWorld and yWorld are within bounds (NOT on the edge or past it)
    void computeGradient(double xWorld, double yWorld, QVector2D& out);
    void addPotential(int x1, int x2, int y1, int y2, double potential, bool preview);    // adds the specified potential to the area (changes only gridData)
    double getPotential(double xWorld, double yWorld, bool discretized = true);  // must be in bounds

    void setupBucketsPosition();
    void setupBucketsMomentum();

    // make an observation as to where the particle is at
    QVector2D observePosition();

    // helpful function, needed in updateGrid
    // index denotes element in gridData, i.e xIndex*samplesPerSide + yIndex
    double getProbFromIndex(int index);
    double computeNext(Mode mode, int x, int y, double dT);

    void interpolateBilinear(char drawMode, char probsCmap, bool preview);
    void interpolateBicubic(char drawMode, char probsCmap, bool preview);

    // not used: multithreading verletQuantum, not too useful
    // id indicates which block (out of 'total' number of blocks)
    void verletQuantumParcel(double timeStep, int id, int numParcels);
    void updateGridParcel(Mode mode, double timeStep, int id, int numParcels);
};

#endif // DATA_H
