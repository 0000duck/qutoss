#ifndef CAMERA_H
#define CAMERA_H

#define PI 3.1415926535897
#define MS_PER_FRAME 16.6667
#define MIN_TIME_BETWEEN_UPDATES 10.0
#define ROUND_TO_ZERO 0.001
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <iostream>
#include <QElapsedTimer>
#include <QMutex>
#include <QOpenGLWidget>
#include "qopengl.h"
#include "math.h"

using namespace std;

/* This file:
 * - camera reactions to keyboard are to be implemented here (redirected from glwidget)
 *   - camera movements follow spherical coordinates
 * - contains necessary info (e.g camera position)
 * - contains appropriate mappings for keys -> changes in view/projection matrices
 * - information on most recent mouse ray and where it intersected our simulation plane (z = 0)
*/

class Camera : public QObject
{
    Q_OBJECT
    enum Moves { Up, Down, Right, Left, In, Out};

public:
    Camera();
    bool isRightAbove() { return position == rightAbove;}   // after an autozoom
    const QMatrix4x4& getView() { return view;}
    const QMatrix4x4& getProj() { return proj;}
    const QVector3D& getPosition() { return position;}
    void moveUp();     // how much movement depends on the defined deltaTime value (aka very little per call)
    void moveDown();    // magnitude of movement may be scaled to appropriate levels by adjusting private members
    void moveRight();
    void moveLeft();
    void zoomIn();
    void zoomOut();

    // gets the intersection to the plane z = 0 given these normalized coordinates
    QVector3D getIntersection(float normedX, float normedY);    // x and y vary between -1 and 1
    QVector3D getMouseRay(float normedX, float normedY);
    QVector3D getMouseRay(const QVector2D& normedCoords);
    void adjustToResize(float w, float h);
    void moveToAbove(QMutex* chicken, bool* inProgress);  // autozoom functionality for viewing
                                                          // bool inProgress is needed as this function is run in a separate loop

private:
    void move(unsigned moveID);

    float minNormTheta = 0.01; // the smallest that it can be, to prevent rotating onto the other side of the z-axis
    float speed = 3.0;  // speed will vary depending on the position (i.e we will multiply another factor)
    float deltaTime = 0.017;  // for 60fps
    QMatrix4x4 view;
    QMatrix4x4 proj;
    QVector3D orientation;
    QVector3D position;
    QVector3D intersection;
    QVector3D rightAbove;  // position to auto-move to
};

#endif // CAMERA_H
