#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QApplication>
#include <QCoreApplication>
#include <QWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QColor>
#include <QToolTip>
#include "data.h"
#include "camera.h"

/* This file:
 * - subclasses QWidget, is a widget for openGL drawing.
 * - contains initialization, vertex/fragment shader sources,
 * - paintGL definition, draw states/event-handling, etc.
 * - this file is included and integrated by mainwindow.h
 */

class GLWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    friend class Window;

public:
    GLWidget(QWidget* parent = NULL);
    ~GLWidget();
    void setEquation(const QString& s);

    double getSimulationTime() { return simData.getSimulationTime();}  // not world time
    double getRemainingTime() { return simData.getRemainingTime();}
    double getTileWidth() { return simData.getTileWidth();}  // aka smallest length we use : info needed for our QSpinBoxes
    Camera* getCamera() { return &camera; }
    double getNettedProbability() { return simData.getNettedProbability();}
    bool simulationStarted() { return hoverState == 3 || hoverState > 8;}  // if the simulation is in progress (true even if paused)

public slots:

    // from tutorial
    void readyTutStage(int tutState);

    // from buttons
    void observe();
    void resetSimulation();
    void toggleBrush();
    void toggleDrawClassical() { drawClassicalParticle = !drawClassicalParticle;
                                 update();}

    // size (argument) ranges from 1 to 10
    void setBrushSize(int size);

    // height (argument) ranges from -5 to 5, at step sizes = 2
    void setBrushHeight(int height);
    void setSimSpeed(unsigned speed) { if (!lockedSimSpeed) simData.setSimSpeed(speed);}
    void setSensitivity(unsigned sensitivity);
    void setXCen(double xCen, bool preview = true);
    void setYCen(double yCen, bool preview = true);
    void setPacketSpeed(double speed, bool preview = true);
    void setPacketAngle(double angle, bool preview = true);
    void setPacketPrecision(double precision, bool preview = true);

    void setHoverState(unsigned tabNum);  // note that hoverState != tabNum
    void togglePause();
    void setSimulationState(bool pause);  // either paused or not paused
    void refreshRequest();
    void brushRequest();    // see if potential brush should be applied (i.e if left mouse button is held down)

    // signals will come from simData
    void updateGridBuffers();
    void updateTileVbo();
    void updateIndicatorBuffers();
    void updateParticleBuffers();
    void updateNetBuffers();
    void updateMeshBuffers();
    void updateTileEbo();

    // everytime timeout occurs we check to see if tutorial stage (applies for 1 & 2) is completed
    void tutTimer1TO();
    void tutTimer2TO();
    void startSimulation();

signals:

    // startingSimulation and pausingSimulation need only be emitted if the event is internal, i.e
    // not caused by the UI (e.g tutorial auto-pause)
    // however, do note that the buttons will not, under any circumstances, react when curStage = 1 or 2
    void startingSimulation();
    void pausingSimulation();
    void resetOccurred();
    void error(string message);
    void updatedCenter(QVector3D center);
    void updatedSpeed(double speed);
    void updatedAngle(double angle);
    void updatedPrecision(double precision);
    void observingWavefunction();    // signal pause button to toggle
    void classicalSimComplete(bool fromSimulation = true);
    void quantumSimComplete(bool fromSimulation = true);

private:

    // member variables
    int tutStage = -1;   // refer to class tutorial for documentation of states
    bool lockedSimSpeed = false;
    bool noRestart = false;
    bool hideQuantum = false;
    bool glInitialized = false;
    bool drawClassicalParticle = false;  // toggled by button
    bool drawPotentials = true;
    bool justObserved = false; // to disallow immediate observations (uncertain behaviour, although it might not be incorrect)
    bool moving = false;  // animations flag
    bool paused = true;
    bool drawWithoutTiles = false;
    bool active = true;
    bool placementMode = true, smoothBrush = false;
    double brushPrecision;
    double brushHeight;
    int sensitivity;
    int key = 0;
    int hoverState = 0; // 0 is anchor follow, 1 is stretch follow,
                        // 2 is height follow, 3 is idle (simulation-in-progress)
                        // 4 is position follow, 5 is precision setting (5 not currently used)
                        // 6 is position idle (for tab 2 = position)
                        // 7 is velocity setting, 8 is velocity idle
                        // NOTE: 8 is deprecated, 5, 7, now both use 6 for idle state
                        // > 8 or = 3: after simulation has started
                        // 9 is anchor follow, 10 is stretch follow (probability net)
                        // to idle during simulation we go back to 3
                        // 11 is a deep idle that cannot be clicked out of (needs external reset of some sort)

    char drawMode = 'P';  // either 'P' or 'R' or 'I'
    char probsCmap = 'H';   // either H for afm_hot or J for jet : only applies to drawmode = 'P'
    unsigned curTabNum = 0;
    QPoint cursorPos;
    QPoint screenAnchor;  // for setting potential values, we compare current mouse pos with this one and scale it
    Camera camera;
    QTimer timer, fastTimer, tutTimer1, tutTimer2;
    QTimer startSimClock;  // once this times out the procedure to start the simulation begins

    QOpenGLBuffer gridVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);  // gridVbo = vertices for wavefunction
    QOpenGLBuffer tileVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);  // tileVbo = vertices for potential tiles
    QOpenGLBuffer funcMeshVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);  // not used: funcMeshVbo = vertices for plot of potential function (lines)
    QOpenGLBuffer indicatorVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    QOpenGLBuffer netVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    QOpenGLBuffer particleVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

    QOpenGLBuffer gridEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QOpenGLBuffer tileEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QOpenGLBuffer funcMeshEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QOpenGLBuffer indicatorEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QOpenGLBuffer particleEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QOpenGLBuffer netEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    // for readability in the source file these are defined near the bottom of the .cpp file,
    // so compiler needs to know about them in the header
    const char* fragmentShader, *fragmentShaderHot, *fragmentShaderCool, *fragmentShaderJet;
    const char* vertexShaderSource, *vertexShaderSource2, *vertexShaderNoColor;

    // an artificial "shadow" on the left side of the GLWidget
    QOpenGLBuffer shadeVbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    QOpenGLBuffer shadeEbo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QVector<GLfloat> shadeVertices;
    QVector<GLuint> shadeElements;

    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgram* shader, * shaderNoTransforms, *shaderHot, *shaderCool, *shaderJet;
    Data simData;
    QMutex chicken;  // inspired by the rubber chicken mutex analogy
    QWaitCondition manager;
    QFuture<void> supervisor1;  // simulation thread
    QFuture<void> supervisor2;  // animation thread

    // functions
    void initializeShaderCode();
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void initializeShadeData();
    void repeatKey();
    void setShadeVertexAttributes();
    void setIndicatorVertexAttributes();
    void setGridVertexAttributes(bool flatVersion = true);
    void setTileVertexAttributes();
    void setFuncMeshVertexAttributes();
    void setParticleVertexAttributes();
    void setNetVertexAttributes();
    void setVertexAttributes(bool hasColorInfo = true);
    void advanceSimulation();
    void cleanup();
};


#endif // GLWIDGET_H
