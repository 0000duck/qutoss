#include "camera.h"


Camera::Camera()
{
    rightAbove = { 0.0, 0.0, 2.414};
    position = {0.6, 0.0, 1.5};
    proj.perspective(45.0, 1.5, 0.1, 100.0);
    view.lookAt({position[0], position[1], position[2]}, {0,0,0}, {0,0,2.5});
}


void Camera::moveUp()
{
    move(Up);
}


void Camera::moveDown()
{
    move(Down);
}


void Camera::moveRight()
{
    move(Right);
}


void Camera::moveLeft()
{
    move(Left);
}


void Camera::zoomIn()
{
    move(In);
}


void Camera::zoomOut()
{
    move(Out);
}


// helper function:
// moveIDs 0 - 5 correspond to different camera movements
void Camera::move(unsigned moveID)
{
    if (fabs(position[0] - 0.0) < 0.01*ROUND_TO_ZERO && (position[1] - 0.0) < 0.01*ROUND_TO_ZERO)
    {
        position += QVector3D(minNormTheta, minNormTheta, 0.0);
        return;
    }

    float normPhi = sqrt(position[0]*position[0] + position[1]*position[1] + position[2]*position[2]);
    float normTheta = sqrt(position[0]*position[0] + position[1]*position[1]);
    normTheta = max(normTheta, 1E-8f);
    QVector3D theta(-position[1]/normTheta, position[0]/normTheta, 0.0);
    QVector3D r(position[0]/normPhi,position[1]/normPhi,position[2]/normPhi);
    QVector3D phi = QVector3D::crossProduct(r, theta);

    if (moveID == Up && normTheta > minNormTheta)
    {
        QVector3D oldPos(position);
        position += phi * 2*PI*normPhi*0.1 * speed * deltaTime;

        // if we flipped while crossing z axis, prevent it
        if (position.x()*oldPos.x() < 0 || position.y()*oldPos.y() < 0)
        {
             float ratio = fabs(oldPos.y()/oldPos.x());
             float newX = minNormTheta/sqrt(ratio*ratio + 1);
             float newY = ratio*newX;
             float newZ = sqrt(normPhi*normPhi - newX*newX - newY*newY);

             if (oldPos.x() < 0)
                 newX *= -1;
             if (oldPos.y() < 0)
                 newY *= -1;
             position = {newX, newY, newZ};
        }
    }
    else if (moveID == Down && position[2] >= 0)
        position -= phi * 2*PI*normPhi*0.1 * speed * deltaTime;
    else if (moveID == Right)
        position += theta * 2*PI*normTheta*0.3 * speed * deltaTime;
    else if (moveID == Left)
        position -= theta * 2*PI*normTheta*0.3 * speed * deltaTime;
    else if (moveID == In && normPhi >= 0.1)
        position -= r * exp(normPhi-2.5) * speed * deltaTime;
    else if (moveID == Out && normPhi <= 4)
        position += r * exp(normPhi-2.5) * speed * deltaTime;

    if (position[2] < 0)
        position[2] = 0;

    // must normalize (to prevent spiraling outwards)
    if (moveID == Left || moveID == Right)
    {
        float newNormTheta = sqrt(position[0]*position[0] + position[1]*position[1]);
        float diff = newNormTheta - normTheta;
        QVector3D tozAxis(-position[0]/newNormTheta, -position[1]/newNormTheta, 0.0);
        position += diff*tozAxis;
    }
    else if (moveID == Up || moveID == Down)
    {
        float newNorm = position.distanceToPoint({0,0,0});
        float diff = newNorm - normPhi;
        position += -diff*r;
    }

    view.setToIdentity();
    view.lookAt({position[0], position[1], position[2]}, {0,0,0}, {0,0,1});
}


QVector3D Camera::getIntersection(float normedX, float normedY)
{
    QVector3D ray = getMouseRay(normedX, normedY);

    // solving for intersection with z = 0:
    // a*ray[2] + position[2] = 0   =>  a = -position[2]/ray[2]
    float a = -position[2]/ray[2];
    intersection = { a*ray[0] + position[0], a*ray[1] + position[1], a*ray[2] + position[2]};
    return intersection;
}


// note that an alternative (to directly get the ray without subtraction) is to
// set w = 0 after applying the inverse of the projection matrix -- however, how
// this worked was not completely clear to me, so I stuck with a more familiar method
QVector3D Camera::getMouseRay(float normedX, float normedY)
{
    QVector4D rayView = proj.inverted() * QVector4D(normedX, normedY, 0.0, 1.0);
    QVector4D point = view.inverted() * QVector4D(rayView[0], rayView[1], rayView[2], 1.0);
    QVector3D ray = {point[0] - position[0], point[1] - position[1], point[2] - position[2]};
    ray.normalize();
    return ray;
}


QVector3D Camera::getMouseRay(const QVector2D &normedCoords)
{
    return getMouseRay(normedCoords.x(), normedCoords.y());
}


void Camera::adjustToResize(float w, float h)
{
    proj.setToIdentity();
    proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}


void Camera::moveToAbove(QMutex* chicken, bool* inProgress)
{
    // interpolate positionition, along with up vector
    orientation = {0, 0, 1};
    QVector3D finalOrientation(0,-1,0);
    QVector3D originalPosition = position;
    QVector3D originalOrientation(0,0,1);
    QVector3D dirPosition = (rightAbove - position).normalized();
    QVector3D dirOrientation = (finalOrientation - QVector3D(0,0,1)).normalized();
    float distPosition = rightAbove.distanceToPoint(position);
    float distOrientation = sqrt(2);

    // use a mapping function with range [0,1] to interpolate and animate
    // e.g sin(t) for t from [0, pi/2]
    float endTime = PI/2.0;
    float dt = endTime/60.0;
    QElapsedTimer timer;

    for (float t = 0; t <= endTime; t += dt)
    {
        timer.start();
        float funcVal = sin(t)*sin(t);   // didn't put much thought into this

        chicken->lock();
        position = originalPosition + dirPosition*distPosition*funcVal;
        orientation = originalOrientation + dirOrientation*distOrientation*funcVal;
        view.setToIdentity();
        view.lookAt(position, {0,0,0}, orientation);
        chicken->unlock();

        while(timer.elapsed() < MS_PER_FRAME);
    }

    chicken->lock();
    view.setToIdentity();
    position = rightAbove;
    view.lookAt(rightAbove, {0,0,0}, finalOrientation);
    *inProgress = false;
    chicken->unlock();
}
