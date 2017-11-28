#include "glwidget.h"


GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent), timer(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);
    initializeShaderCode();

    connect(&simData, SIGNAL(updateTileVbo()), this, SLOT(updateTileVbo()));
    connect(&simData, SIGNAL(updateTileEbo()), this, SLOT(updateTileEbo()));
    connect(&simData, SIGNAL(updateGridBuffers()), this, SLOT(updateGridBuffers()));
    connect(&simData, SIGNAL(updateIndicatorBuffers()), this, SLOT(updateIndicatorBuffers()));
    connect(&simData, SIGNAL(updateParticleBuffers()), this, SLOT(updateParticleBuffers()));
    connect(&simData, SIGNAL(updateNetBuffers()), this, SLOT(updateNetBuffers()));
    connect(&simData, SIGNAL(updateMeshBuffers()), this, SLOT(updateMeshBuffers()));

    timer.start(16); // t.o every 16 milliseconds (to refresh screen, if needed)
    fastTimer.start(8);
    connect(&timer, SIGNAL(timeout()), this, SLOT(refreshRequest()));
    connect(&fastTimer, SIGNAL(timeout()), this, SLOT(brushRequest()));
    connect(&tutTimer1, SIGNAL(timeout()), this, SLOT(tutTimer1TO()));
    connect(&tutTimer2, SIGNAL(timeout()), this, SLOT(tutTimer2TO()));
    connect(&startSimClock, SIGNAL(timeout()), this, SLOT(startSimulation()));

    update();
}


GLWidget::~GLWidget()
{
    manager.wakeOne();
    active = false;

    // wait for completion
    while (!(supervisor1.isFinished() && supervisor2.isFinished()));

    cleanup();
}



//----------------------------------------------------------------------
// SLOTS
//----------------------------------------------------------------------


void GLWidget::tutTimer1TO()
{
    if (simData.getElapsedTime() >= 0.7)
    {
        tutTimer1.stop();
        paused = true;
        emit classicalSimComplete();
    }
}


void GLWidget::tutTimer2TO()
{
    if(simData.getElapsedTime() >= 0.85)
    {
        tutTimer2.stop();
        paused = true;
        emit quantumSimComplete();
    }
}


void GLWidget::startSimulation()
{
    paused = false;
    emit startingSimulation();
    hoverState = 9;
}


void GLWidget::readyTutStage(int tutState)
{
    tutStage = tutState;

    if (tutState == 0)
    {
        resetSimulation();
        drawClassicalParticle = true;
        hideQuantum = true;
        setSensitivity(0);   // what is in the argument actually doesnt matter if tutState = 0
    }
    else if (tutState == 1)
    {
        // play a simulation with only the classical particle running towards a wall
        resetSimulation();
        simData.useFixedSettings();
        emit updatedCenter(simData.getPacketCenter());
        emit updatedSpeed(simData.getPacketSpeed());
        emit updatedAngle(simData.getPacketAngle());

        setSimSpeed(2);
        lockedSimSpeed = true;
        drawClassicalParticle = true;
        hideQuantum = true;
        noRestart = true;
        setSensitivity(0);

        // once this stage is over, we send a signal to queue next dialogue
        startSimClock.setSingleShot(true);
        startSimClock.start(1000);
        tutTimer1.start(100);
    }
    else if (tutState == 2)
    {
        // play a simulation with only the quantum particle heading towards the same wall
        simData.useFixedSettings();
        emit updatedCenter(simData.getPacketCenter());
        emit updatedSpeed(simData.getPacketSpeed());
        emit updatedAngle(simData.getPacketAngle());

        lockedSimSpeed = true;
        drawClassicalParticle = false;
        hideQuantum = false;
        noRestart = true;
        setSensitivity(98);  // 98 is the default

        // once this stage is over, we send a signal to queue next dialogue
        startSimClock.setSingleShot(true);
        startSimClock.start(1000);
        tutTimer2.start(100);
    }
    else if (tutState == 3)
    {
        // buttons react only once tutState is past 1 and 2
        emit pausingSimulation();

        // pause and instruct the user to find the probabilities on each side of the wall
        noRestart = false;
        lockedSimSpeed = false;
    }
}


void GLWidget::setBrushHeight(int height)
{
    // mapping from [-2, 3] to [-5, 5]
    height *= 2;
    height -= 1;

    brushHeight = fabs(height)*height/12.0;
    simData.setBrushParams(brushHeight, brushPrecision);
    update();
}


void GLWidget::setBrushSize(int size)
{
    brushPrecision = double(-size/10.0 + 11 - 1.0)*24.5/9.0 + 0.5;
    simData.setBrushParams(brushHeight, brushPrecision);
    update();
}


void GLWidget::toggleBrush()
{
    if (!smoothBrush)  // switching from tiles mode, clear the preview, do not finalize
    {
        simData.setTiles(false);
        hoverState = -1;    // set mouse hovering to idle (we use timer interrupts instead)
    }
    else if (hoverState < 3)
    {
        hoverState = 0;
        simData.resetAnchorAndStretch();
    }

    smoothBrush = !smoothBrush;
    update();
}



// handling signals from simData (requests to update glbuffers)
void GLWidget::updateGridBuffers()
{
    gridVbo.bind();
    gridVbo.write(0, simData.getGridVertices(), simData.getGridVerticesLength()*sizeof(GLfloat));
}


void GLWidget::updateTileVbo()
{
    tileVbo.bind();
    tileVbo.write(0, simData.getTileVertices(), simData.getTileVerticesLength()*sizeof(GLfloat));
    glFinish();
}


void GLWidget::updateTileEbo()
{
    tileEbo.bind();
    tileEbo.write(0, simData.getTileElements(), simData.getTileElementsLength()*sizeof(GLuint));
}


void GLWidget::updateIndicatorBuffers()
{
    indicatorEbo.bind();
    indicatorVbo.bind();
    indicatorVbo.write(0, simData.getIndicatorVertices(), simData.getIndicatorVerticesLength()*sizeof(GLfloat));
}


void GLWidget::updateParticleBuffers()
{
    particleVbo.bind();
    particleVbo.write(0, simData.getParticleVertices(), simData.getParticleVerticesLength()*sizeof(GLfloat));
}


void GLWidget::updateNetBuffers()
{
    netVbo.bind();
    netVbo.write(0, simData.getNetVertices(), simData.getNetVerticesLength()*sizeof(GLfloat));
}


void GLWidget::updateMeshBuffers()
{
    funcMeshVbo.bind();
    funcMeshVbo.write(0, simData.getFuncMeshVertices(), simData.getFuncMeshVerticesLength()*sizeof(GLfloat));
}


void GLWidget::setSensitivity(unsigned _sensitivity)
{
    if (hideQuantum)
    {
        sensitivity = -1E8;  // sensitivity function scales as 1/(1 - sensitivity)
    }

    else
    {
        sensitivity = _sensitivity;
    }

    // will crash otherwise
    if (glInitialized)
    {
        shaderHot->bind();
        shaderHot->setUniformValue("sensitivity", float(sensitivity));
        shaderHot->release();

        shaderCool->bind();
        shaderCool->setUniformValue("sensitivity", float(sensitivity));
        shaderCool->release();

        shaderJet->bind();
        shaderJet->setUniformValue("sensitivity", float(sensitivity));
        shaderJet->release();
    }

    // for flat drawing mode, where we won't have the shaders helping us ...
    simData.setSensitivity(sensitivity);
    update();
}


void GLWidget::setPacketSpeed(double speed, bool preview)
{
    if (hoverState == 3)
        return;
    simData.setSpeed(speed);
    emit updatedSpeed(simData.getPacketSpeed());

    if (!preview)
        simData.confirmPacket();

    update();
}


void GLWidget::setPacketAngle(double angle, bool preview)
{
    if (hoverState == 3)
        return;
    simData.setAngle(angle);
    emit updatedAngle(simData.getPacketAngle());

    if(!preview)
        simData.confirmPacket();

    update();
}


void GLWidget::setXCen(double xCen, bool preview)
{
    if (hoverState == 3)
        return;
    simData.setXCen((double)xCen);
    emit updatedCenter(simData.getPacketCenter());

    if (!preview)
        simData.confirmPacket();

    update();
}


void GLWidget::setYCen(double yCen, bool preview)
{
    if (hoverState == 3)
        return;
    simData.setYCen((double)yCen);
    emit updatedCenter(simData.getPacketCenter());

    if (!preview)
        simData.confirmPacket();

    update();
}


void GLWidget::setPacketPrecision(double precision, bool preview)
{
    if (hoverState == 3)
        return;

    simData.setPrecision(precision);
    emit updatedPrecision(simData.getPacketPrecision());

    if (!preview)
        simData.confirmPacket();

    update();
}


void GLWidget::setHoverState(unsigned tabNum)
{
    curTabNum = tabNum;

    if (!simulationStarted())
    {
        if (tabNum == 0)
            hoverState = 0;
        else if (tabNum == 1)
        {
            simData.setTiles(false);
            hoverState = 6;
        }
        else if (tabNum == 2)
        {
            simData.setTiles(false);
            hoverState = 8;
        }

        update();
    }
}


// every 16ms ( for 60fps refresh rate) this slot is called
void GLWidget::refreshRequest()
{
    if (moving || !paused || key != 0)
        update();
}


void GLWidget::brushRequest()
{
    if (!underMouse())
        return;

    Qt::MouseButtons state = QApplication::mouseButtons();
    if (state == Qt::LeftButton && smoothBrush && hoverState < 3)
    {
        QVector2D GLpoint = scrnToGL(cursorPos.x(), cursorPos.y(), width(), height());
        simData.paintPotential(camera.getPosition(), camera.getMouseRay(GLpoint.x(), GLpoint.y()));
        update();
    }
}

void GLWidget::setEquation(const QString &s)
{
    string message = simData.setEquation(s);

    if (message != "all is well")
        emit error(message);
    if (message == "Values were clipped for stability" || message == "all is well")
    {
        // OLD VERSION, may possibly revisit as a graphics upgrade
        // funcMeshVbo.bind();
        // funcMeshVbo.write(0, simData.getFuncVertices(), simData.getGridVerticesLength()*sizeof(GLfloat));
        update();
    }
}


void GLWidget::setSimulationState(bool pause)
{
    if (pause == !paused)  // disagreement, must toggle
        togglePause();
}


// from button presses
void GLWidget::togglePause()
{
    if (hoverState != 3 && hoverState < 9) // first time pressing P
    {
        emit startingSimulation();
        hoverState = 9;  // mouse hovering is idle after simulation begins
        simData.setSimSpeed(1);
        simData.setTiles(false);
        update();
    }

    chicken.lock();
    paused = !paused;
    if (!paused)
        manager.wakeOne();
    chicken.unlock();

    justObserved = false;
}


void GLWidget::observe()
{
    if (!justObserved && simulationStarted() && (tutStage == -1 || tutStage == 4 ))
    {
        chicken.lock();
        simData.observe(24);  // if precision is too high, discretization effects occur
        paused = true;
        justObserved = true;
        chicken.unlock();
        update();
        emit observingWavefunction();
    }
}


void GLWidget::resetSimulation()
{
    if (noRestart)
        return;

    emit resetOccurred();
    chicken.lock();
    paused = true;
    simData.resetSimulation();

    // curTabNum was saved before we switched to simulation mode
    if (curTabNum == 0)
        hoverState = 0;
    else if (curTabNum == 1)
        hoverState = 6;
    else if (curTabNum == 2)
        hoverState = 8;

    updateTileVbo();
    chicken.unlock();

    update();
}


void GLWidget::resizeGL(int w, int h)
{
    camera.adjustToResize(w, h);
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    // idk, we do things on mouseRelease only (for now)
}


// see header file for more precise hoverState definitions
void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (hoverState == 11)
        return;

    // find the intersection of mouse ray with z = 0
    QPoint point = event->pos();
    QVector2D GLpoint = scrnToGL(point.x(), point.y(), width(), height());
    QVector3D intersect = camera.getIntersection(GLpoint.x(), GLpoint.y());

    if (event->button() == Qt::LeftButton)
    {
        // 0, 1, 2 is the cycle for tile placement, 3 for idle
        if (hoverState < 3)
        {
            if (!smoothBrush)
            {
                hoverState = (hoverState + 1)%3;

                // just passed through an entire cycle (implying we finished setting one tile)
                if (hoverState == 0)
                {
                    simData.confirmTile();
                }

                // define the screen anchor for setting potential values
                if (hoverState == 2)
                {
                    screenAnchor = event->pos();
                }
            }
            simData.updateTileOrder(false);
        }


        // 4, 5 is for location/precision, 6 is a releasable idle
        // 5 is an unused slot at the moment
        else if (hoverState >= 4 && hoverState <= 7)
        {
            // confirm particle center : move to idling state
            if (hoverState == 4)
            {
                hoverState = 6;
                simData.confirmPacket();
            }
            // confirm arrow
            else if (hoverState == 7)
            {
                simData.confirmPacket();
                hoverState = 6;
            }

            // 4, 7 share the same idling state(6) : if we are at 6, must branch off accordingly
            // to bring out of idling state, click should have been within the vicinity
            // the arrow gets higher priority (as it is smaller)
            else if (hoverState == 6)
            {
                if (simData.withinArrowVicinity(intersect))
                {
                    hoverState = 7;
                    simData.saveCurrentState();
                    simData.setVelocity(intersect);
                }
                else if (simData.withinPacketVicinity(intersect))
                {
                    hoverState = 4;
                    simData.saveCurrentState();  // this is the snapshot that we can return to later
                    simData.setCenter(intersect);
                    emit updatedCenter(simData.getPacketCenter());
                }
            }
        }

        else if (hoverState == 3)
            hoverState = 9;

        else if (hoverState == 9 || hoverState == 10)
        {
            if (hoverState == 9)
                hoverState = 10;
            else if (hoverState == 10)
                hoverState = 3;
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        // resets so that user can choose a new anchor
        if (hoverState < 3)
            hoverState = 0;

        if (hoverState == 9 || hoverState == 10)
            hoverState = 9;

        // right click reset to previous state
        if (hoverState >= 4 && hoverState <= 7)
        {
            hoverState = 6;

            simData.restoreSaved();
            emit updatedCenter(simData.getPacketCenter());
            emit updatedSpeed(simData.getPacketSpeed());
            emit updatedAngle(simData.getPacketAngle());
        }
    }

    update();
}


// for documentation of hoverstates see above function, or header file
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    // find the intersection of mouse ray with z = 0
    cursorPos = event->pos();
    QVector2D GLPoint = scrnToGL(cursorPos.x(), cursorPos.y(), width(), height());
    QVector3D intersect = camera.getIntersection(GLPoint.x(), GLPoint.y());

    // don't bother flooding everything with updates
    if (hoverState == 3 || hoverState == 6 || hoverState == 8 || hoverState == 11)
        return;

    if (hoverState < 3 && smoothBrush)
    {
        simData.setBrush(camera.getPosition(), camera.getMouseRay(GLPoint.x(), GLPoint.y()));
        brushRequest();
    }
    else if (hoverState == 0)
    {
        simData.setAnchor(camera.getPosition(), camera.getMouseRay(GLPoint.x(), GLPoint.y()));
    }
    else if (hoverState == 1)
    {
        simData.setStretch(camera.getPosition(), camera.getMouseRay(GLPoint));
    }
    else if (hoverState == 2)
    {
        simData.setPotentialFromStretch(GLPoint.y() - 0);   // anchor is always based off the center
    }
    else if (hoverState == 4)
    {
        simData.setCenter(intersect);
        emit updatedCenter(simData.getPacketCenter());
    }
    else if (hoverState == 7)
    {
        simData.setVelocity(intersect);
        emit updatedAngle(simData.getPacketAngle());
        emit updatedSpeed(simData.getPacketSpeed());
    }
    else if (hoverState == 9 && !hideQuantum)
    {
        simData.setNetAnchor(camera.getPosition(), camera.getMouseRay(GLPoint));
    }
    else if (hoverState == 10 && !hideQuantum)
    {
        simData.setNetStretch(camera.getPosition(), camera.getMouseRay(GLPoint));
    }

    update();
}


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    chicken.lock();
    int prevKey = key;
    key = event->key();
    chicken.unlock();

    // global quit, let it propagate to parent window
    if (key == Qt::Key_Escape)
        event->ignore();

    else if (key == Qt::Key_I)
    {
        drawPotentials = !drawPotentials;
    }

    // make observation
    else if (key == Qt::Key_O)
    {
        //observe();
    }

    // reset
    else if (key == Qt::Key_B)
    {
        resetSimulation();
    }
    else if (key == Qt::Key_1)
        drawMode = 'P';
    else if (key == Qt::Key_2)
        drawMode = 'R';
    else if (key == Qt::Key_3)
        drawMode = 'I';
    else if (key == Qt::Key_P)
    {
        chicken.lock();
        if (!camera.isRightAbove())
        {
            moving = true;
            if (!supervisor2.isRunning())
                supervisor2 = QtConcurrent::run(&camera, &Camera::moveToAbove, &chicken, &moving);
        }
        chicken.unlock();
    }
    else if (key == Qt::Key_C)
    {
        if (probsCmap == 'H')
            probsCmap = 'J';
        else
            probsCmap = 'H';
    }

    update();

    // to allow camera movements when in a paused state:
    // wake only at first press (to prevent crashes from holding down keys)
    chicken.lock();
    if (paused && prevKey == 0 && key != 0)
        manager.wakeOne();

    chicken.unlock();
}


void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    chicken.lock();

    // this is to signify to function repeatKey that we released our hold
    key = 0;

    if (event->key() == Qt::Key_M)
        event->ignore();

    chicken.unlock();
}


void GLWidget::cleanup()
{
    makeCurrent();

    vao.destroy();
    gridVbo.destroy();
    gridEbo.destroy();
    tileVbo.destroy();
    tileEbo.destroy();
    indicatorVbo.destroy();
    indicatorEbo.destroy();
    particleVbo.destroy();
    particleEbo.destroy();
    netVbo.destroy();
    netEbo.destroy();
    shadeVbo.destroy();
    shadeEbo.destroy();

    delete shader;
    delete shaderNoTransforms;
    delete shaderHot;
    delete shaderCool;
    delete shaderJet;
    shader = NULL;
    shaderNoTransforms = NULL;
    shaderHot = NULL;
    shaderCool = NULL;
    shaderJet = NULL;

    doneCurrent();
}



//----------------------------------------------------------------------
// GENERAL FUNCTIONS
//----------------------------------------------------------------------



// to allow the widget to update at 60fps (workaround for slow key autorepeat settings)
void GLWidget::repeatKey()
{
    bool cameraMoved = true;

    // movement commands
    if (key == Qt::Key_W)
        camera.moveUp();
    else if (key == Qt::Key_S)
        camera.moveDown();
    else if (key == Qt::Key_A)
        camera.moveLeft();
    else if (key == Qt::Key_D)
        camera.moveRight();
    else if (key == Qt::Key_Z)
        camera.zoomIn();
    else if (key == Qt::Key_X)
        camera.zoomOut();
    else
        cameraMoved = false;

    if (cameraMoved)
    {
        QVector2D GLPoint = scrnToGL(cursorPos.x(), cursorPos.y(), width(), height());
        simData.setBrush(camera.getPosition(), camera.getMouseRay(GLPoint.x(), GLPoint.y()));
    }
}



void GLWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);
    initializeOpenGLFunctions();
    glClearColor(0.16, 0.16, 0.2, 0.0);

    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // load shader from C-strings (written in GLSL near the bottom of this file)
    shader = new QOpenGLShaderProgram;
    shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    shader->bindAttributeLocation("vertexPosition", 0);
    shader->bindAttributeLocation("inColor", 1);
    shader->link();

    shaderNoTransforms = new QOpenGLShaderProgram;
    shaderNoTransforms->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource2);
    shaderNoTransforms->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader);
    shaderNoTransforms->bindAttributeLocation("vertexPosition", 0);
    shaderNoTransforms->bindAttributeLocation("inColor", 1);
    shaderNoTransforms->link();

    shaderHot = new QOpenGLShaderProgram;
    shaderHot->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderNoColor);
    shaderHot->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderHot);
    shaderHot->bindAttributeLocation("vertexPosition", 0);
    shaderHot->link();

    shaderCool = new QOpenGLShaderProgram;
    shaderCool->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderNoColor);
    shaderCool->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderCool);
    shaderCool->bindAttributeLocation("vertexPosition", 0);
    shaderCool->link();

    shaderJet = new QOpenGLShaderProgram;
    shaderJet->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderNoColor);
    shaderJet->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderJet);
    shaderJet->bindAttributeLocation("vertexPosition", 0);
    shaderJet->link();

    // set up the VBOs and EBOs
    // wavefunction grid
    gridVbo.create();
    gridVbo.bind();
    gridVbo.allocate(simData.getGridVertices(), simData.getGridVerticesLength() * sizeof(GLfloat));
    gridEbo.create();
    gridEbo.bind();
    gridEbo.allocate(simData.getGridElements(), simData.getGridElementsLength() * sizeof(GLuint));

    // discrete potential tiles
    tileVbo.create();
    tileVbo.bind();
    tileVbo.allocate(simData.getTileVertices() , simData.getTileVerticesLength() * sizeof(GLfloat));
    tileEbo.create();
    tileEbo.bind();
    tileEbo.allocate(simData.getTileElements(), simData.getTileElementsLength() * sizeof(GLuint));

    // any 2D indicators to help the user
    indicatorVbo.create();
    indicatorVbo.bind();
    indicatorVbo.allocate(simData.getIndicatorVertices(), simData.getIndicatorVerticesLength() * sizeof(GLfloat));

    // the ebo for indicators is needed only for the brush, at the moment
    indicatorEbo.create();
    indicatorEbo.bind();
    indicatorEbo.allocate(simData.getBrushElements(), simData.getBrushElementsLength() * sizeof(GLuint));

    particleVbo.create();
    particleVbo.bind();
    particleVbo.allocate(simData.getParticleVertices(), simData.getParticleVerticesLength() * sizeof(GLfloat));

    particleEbo.create();
    particleEbo.bind();
    particleEbo.allocate(simData.getParticleElements(), simData.getParticleElementsLength() * sizeof(GLuint));

    netVbo.create();
    netVbo.bind();
    netVbo.allocate(simData.getNetVertices(), simData.getNetVerticesLength() * sizeof(GLfloat));
    netEbo.create();
    netEbo.bind();
    netEbo.allocate(simData.getNetElements(), simData.getNetElementsLength() * sizeof(GLuint));

    funcMeshVbo.create();
    funcMeshVbo.bind();
    funcMeshVbo.allocate(simData.getFuncMeshVertices(), simData.getFuncMeshVerticesLength() * sizeof(GLfloat));
    funcMeshEbo.create();
    funcMeshEbo.bind();
    funcMeshEbo.allocate(simData.getFuncMeshElements(), simData.getFuncMeshElementsLength() * sizeof(GLfloat));

    initializeShadeData();
    shadeVbo.create();
    shadeVbo.bind();
    shadeVbo.allocate(shadeVertices.constData(), shadeVertices.length()* sizeof(GLfloat));
    shadeEbo.create();
    shadeEbo.bind();
    shadeEbo.allocate(shadeElements.constData(), shadeElements.length()* sizeof(GLuint));

    // OLD VERSION: asynchronous computations/drawing
    // run a separate thread, just for updating data
    //supervisor1 = QtConcurrent::run(this, &GLWidget::advanceSimulation);

    shaderHot->bind();
    shaderHot->setUniformValue("sensitivity", float(sensitivity));
    shaderHot->release();

    shaderCool->bind();
    shaderCool->setUniformValue("sensitivity", float(sensitivity));
    shaderCool->release();

    shaderJet->bind();
    shaderJet->setUniformValue("sensitivity", float(sensitivity));
    shaderJet->release();

    glInitialized = true;
}


void GLWidget::initializeShadeData()
{
    // it's really just 1 square
    shadeVertices.resize(4*6);
    shadeElements.resize(6);

    // x from -1 to 1, y from -1 to 1 (these coordinates cover the entire screen)
    // vertex size is 6, 2 for position and 4 for rgba
    for (int i = 0; i < 4; ++i)
    {
        int index = i*6;
        if (i == 0 || i == 1)     // top 2 vertices
            shadeVertices[index+1] = 1.0;
        else
            shadeVertices[index+1] = -1.0;

        if (i == 0 || i == 3)    // left 2 vertices
        {
            shadeVertices[index] = -1.0;
            shadeVertices[index+5] = 0.4;
        }
        else
        {
            shadeVertices[index] = -0.975;
            shadeVertices[index+5] = 0.0;
        }

        shadeVertices[index+2] = 0.0;
        shadeVertices[index+3] = 0.0;
        shadeVertices[index+4] = 0.0;
    }

    // note that vertices are initialized in clock-wise order, from top-left
    // drawing order
    shadeElements[0] = 0;
    shadeElements[1] = 1;
    shadeElements[2] = 3;
    shadeElements[3] = 3;
    shadeElements[4] = 1;
    shadeElements[5] = 2;
}


// slightly different vertex layout than the other ones
void GLWidget::setShadeVertexAttributes()
{
    shadeEbo.bind();
    shadeVbo.bind();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(2 * sizeof(GLfloat)));

    shadeVbo.release();
}


// not the correct way to do things (lags performance, slightly/negligibly):
// either 1) squash all vertices into one vertex buffer (bad)
// or     2) create a different vao for each vbo/ebo set
// both unpleasant, so this will be the temporary solution
void GLWidget::setGridVertexAttributes(bool flat)
{
    gridEbo.bind();
    gridVbo.bind();

    if (flat)
        setVertexAttributes();
    else
        setVertexAttributes(false);

    gridVbo.release();
}


void GLWidget::setTileVertexAttributes()
{
    tileVbo.bind();
    tileEbo.bind();
    setVertexAttributes();
    tileVbo.release();
}


void GLWidget::setFuncMeshVertexAttributes()
{
    funcMeshEbo.bind();
    funcMeshVbo.bind();
    setVertexAttributes();
    funcMeshVbo.release();
}


void GLWidget::setIndicatorVertexAttributes()
{
    indicatorEbo.bind();
    indicatorVbo.bind();
    setVertexAttributes();
    indicatorVbo.release();
}


void GLWidget::setParticleVertexAttributes()
{
    particleEbo.bind();
    particleVbo.bind();
    setVertexAttributes();
    particleVbo.release();
}


void GLWidget::setNetVertexAttributes()
{
    netEbo.bind();
    netVbo.bind();
    setVertexAttributes();
    netVbo.release();
}


void GLWidget::setVertexAttributes(bool hasColorInfo)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if (hasColorInfo)
    {
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
    }
    else
    {
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    }
}


void GLWidget::paintGL()
{
    repeatKey();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // synchronous animation
    if (!paused && simData.getRemainingTime() > 0)
        simData.advanceSimulation(&chicken, hideQuantum);
    if ((hoverState == 3 || hoverState >= 9) && !camera.isRightAbove() && !hideQuantum)
        simData.updateGrid(drawMode, false, probsCmap); // boolean is 'flat=' flag
    else
        simData.updateGrid(drawMode, true, probsCmap);

    if (probsCmap == 'J' && drawMode == 'P')
        simData.updateParticle(camera.getPosition(), Color(1.0, 0.5, 0.5));
    else
        simData.updateParticle(camera.getPosition(), Color(0, 0, 1.0));

    // reading data shared with other threads
    chicken.lock();

    // use our shader, update matrix values
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);
    shader->bind();
    shader->setUniformValue("proj", camera.getProj());
    shader->setUniformValue("view", camera.getView());
    shader->setUniformValue("opacity", 0.7f);

    if (hoverState != 3 && hoverState < 9)
    {
        setFuncMeshVertexAttributes();
        glDrawElements(GL_LINES, simData.getFuncMeshElementsLength(), GL_UNSIGNED_INT, 0);

        if (drawPotentials)
        {
            shader->setUniformValue("opacity", 0.45f);
            setTileVertexAttributes();
            glDrawElements(GL_TRIANGLES, simData.getTileElementsLength(), GL_UNSIGNED_INT, 0);
        }

        if (hoverState >= 4 && hoverState <= 7)  // setting initial properties
        {
            shader->setUniformValue("opacity", 1.0f);
            setIndicatorVertexAttributes();
            glDrawArrays(GL_LINE_STRIP, 0, 2);
            glDrawArrays(GL_TRIANGLES, 2, 3);
        }

        // draw the simulation grid
        shader->setUniformValue("opacity", 0.5f);
        setGridVertexAttributes();
        glDrawElements(GL_TRIANGLES, simData.getGridElementsLength(), GL_UNSIGNED_INT, 0);

        if (hoverState < 3 && drawPotentials)
        {
            glDisable(GL_DEPTH_TEST);
            if (!smoothBrush)  // potential tile placing, draw the outlining points
            {
                setTileVertexAttributes();
                shader->setUniformValue("opacity", 1.0f);
                glDrawArrays(GL_POINTS, simData.getTileVerticesLength()/VERTEX_SIZE - 8, 8);
            }
            else
            {
                // set attributes for all of the indicators, but we're only drawing the brush here
                shader->setUniformValue("opacity", 0.3f);
                setIndicatorVertexAttributes();
                glDrawElements(GL_TRIANGLES, simData.getBrushElementsLength(), GL_UNSIGNED_INT, 0);
            }
        }
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        if (!camera.isRightAbove())
        {
            shader->release();
            if (drawMode == 'P')
            {
                if (probsCmap == 'H')
                {
                    shaderHot->bind();
                    shaderHot->setUniformValue("proj", camera.getProj());
                    shaderHot->setUniformValue("view", camera.getView());

                    setGridVertexAttributes(false);
                    glDrawElements(GL_TRIANGLES, simData.getGridElementsLength(), GL_UNSIGNED_INT, 0);
                    shaderHot->release();
                }
                else
                {
                    shaderJet->bind();
                    shaderJet->setUniformValue("proj", camera.getProj());
                    shaderJet->setUniformValue("view", camera.getView());

                    setGridVertexAttributes(false);
                    glDrawElements(GL_TRIANGLES, simData.getGridElementsLength(), GL_UNSIGNED_INT, 0);
                    shaderJet->release();
                }
            }
            else
            {
                shaderCool->bind();
                shaderCool->setUniformValue("proj", camera.getProj());
                shaderCool->setUniformValue("view", camera.getView());

                setGridVertexAttributes(false);
                glDrawElements(GL_TRIANGLES, simData.getGridElementsLength(), GL_UNSIGNED_INT, 0);
                shaderCool->release();
            }
            shader->bind();
        }
        else   // flat mode, does not rely on fragment shader for color decision
        {
            shader->setUniformValue("opacity", 0.75f);
            setGridVertexAttributes(true);
            glDrawElements(GL_TRIANGLES, simData.getGridElementsLength(), GL_UNSIGNED_INT, 0);
        }

        // draw probability net
        if (hoverState != 9)
        {
            setNetVertexAttributes();
            if (!camera.isRightAbove())
            {
                // probability net is the last 'tile' (1 past the outline), but we refrain from drawing the top face (last 6 elements)
                shader->setUniformValue("opacity", 0.6f);
                glDrawElements(GL_TRIANGLES, simData.getNetElementsLength(), GL_UNSIGNED_INT, 0);
            }
            else
            {
                glDrawArrays(GL_LINE_LOOP, 0, 4);
            }
        }

        if (drawPotentials)
        {
            shader->setUniformValue("opacity", 0.1f);
            setTileVertexAttributes();
            glDrawElements(GL_TRIANGLES, simData.getTileElementsLength(), GL_UNSIGNED_INT, 0);
        }

        if (!camera.isRightAbove())
        {
            shader->setUniformValue("opacity", 0.5f);
            setFuncMeshVertexAttributes();
            glDrawElements(GL_LINES, simData.getFuncMeshElementsLength(), GL_UNSIGNED_INT, 0);
        }
    }

    if (drawClassicalParticle)
    {
        glDisable(GL_DEPTH_TEST);
        shader->setUniformValue("opacity", 1.0f);
        setParticleVertexAttributes();
        glDrawElements(GL_TRIANGLES, simData.getParticleElementsLength(), GL_UNSIGNED_INT, 0);
    }

    shader->release();
    shaderNoTransforms->bind();
    glDisable(GL_DEPTH_TEST);
    setShadeVertexAttributes();
    glDrawElements(GL_TRIANGLES, shadeElements.length(), GL_UNSIGNED_INT, 0);
    shaderNoTransforms->release();
    chicken.unlock();
}


// deprecated:
// in initial tests, asynchronous drawing and simulation-advancing was faster,
// but it is better to use synchronous operations when the load on simData::advanceSimulation is larger
// this function should be run in a separate thread
void GLWidget::advanceSimulation()
{
    // the thread sleeps when manager.wait is called
    // the thread may be woken (even when paused) when view is changed
    QElapsedTimer timer;

    while(active)
    {
        timer.start();
        if (paused || simData.getRemainingTime() <= 0)
        {
            QMutex* fill = NULL;
            if (key == NULL)
                manager.wait(fill);
        }
        else  // not paused and still have time, advance simulation
        {
            chicken.lock();
            simData.advanceSimulation(&chicken);
            chicken.unlock();
        }

        // updating faster than 60 times a second is not presentable
        while (timer.elapsed() < MS_PER_FRAME);
    }
}



// -------------------------------------------------------
// SHADER SOURCE FILES
// -------------------------------------------------------


void GLWidget::initializeShaderCode()
{
    // vanilla fragment shader, will use color given
    fragmentShader =
            "#version 140\n"
            "// Output data \n"
            "in vec4 color;\n"
            "out vec4 outColor;\n"
            "void main()\n"
            "{\n"
            "	outColor = color;\n "
            "}\n";


    // these fragment shaders correspond to different colormaps
    fragmentShaderHot =
            "#version 140\n"
            "in vec3 worldPos; \n"
            "uniform float sensitivity; \n"
            "out vec4 outColor;\n"
            "void main()\n"
            "{\n"
            "vec3 color = vec3(0,0,0); \n"
            "float vScaled = worldPos.z/(1.0 - sensitivity/100.0); \n"

            // red
            "if (vScaled < 0.5) \n"
                "color.x = 2.0*vScaled; \n"
            "else \n"
                "color.x = 1.0; \n"

            // green
            "if (vScaled < 0.25) \n"
                "color.y = 0.0; \n"
            "else if (vScaled > 0.75) \n"
                "color.y = 1.0; \n"
            "else \n"
                "color.y = 2.0*(vScaled - 0.25); \n"

            // blue
            "if (vScaled < 0.5) \n"
                "color.z = 0.0; \n"
            "else \n"
                "color.z = 2.0*(vScaled - 0.5) + 0.0; \n"
            "outColor = vec4(color, 1.0); \n "
            "}\n";


    fragmentShaderCool =
            "#version 140\n"
            "in vec3 worldPos; \n"
            "uniform float sensitivity; \n"
            "out vec4 outColor;\n"
            "void main()\n"
            "{\n"

            // scale v to a value from 0 to 1 to make it harder to screw up
            "float vMax = 2.0*(1.0 - sensitivity/100.0); \n"
            "float vScaled = (worldPos.z + vMax)/(2.0*vMax); \n"
            "vec3 color = vec3(0,0,0); \n"
            "color.x = vScaled; \n"
            "color.y = -vScaled + 1.0; \n"
            "color.z = 1.0; \n"
            "outColor = vec4(color, 1.0); \n "
            "}\n";


    fragmentShaderJet =
            "#version 140\n"
            "in vec3 worldPos; \n"
            "uniform float sensitivity; \n"
            "out vec4 outColor;\n"
            "void main()\n"
            "{\n"
            "vec3 color = vec3(0,0,0); \n"
            "float vScaled = worldPos.z/(1.0 - sensitivity/100.0); \n"
            "vScaled = min(vScaled, 1.0); \n"

            // up to first stop point: 11.25%
            "if (vScaled < 0.1125)\n"
            "{\n"
                "color.r = 0; \n"
                "color.g = 0; \n"
                "color.b = 4 * vScaled + 0.55; \n"
            "}\n"

            // up to second stop point: 36.25%
            "else if (vScaled < 0.3625)\n"
            "{\n"
                "color.r = 0;\n"
                "color.g = 4 * (vScaled - 0.25) + 0.55;\n"
                "color.b = 1;\n"
            "}\n"

            // up to third stop point: 61.25%
            "else if (vScaled < 0.6125)\n"
            "{\n"
                "color.r  = 4 * (vScaled - 0.5) + 0.55;\n"
                "color.g = 1;\n"
                "color.b = -4 * (vScaled - 0.75) - 0.55;\n"
            "}\n"

            // up to last stop point: 86.25%
            "else if (vScaled < 0.8625)\n"
            "{\n"
                "color.r = 1;\n"
                "color.g = -4 * (vScaled - 1) - 0.55;\n"
                "color.b = 0;\n"
            "}\n"

            // to the end
            "else\n"
            "{\n"
                "color.r = -4 * (vScaled - 1.25) - 0.55;\n"
                "color.g = 0;\n"
                "color.b = 0;\n"
            "}\n"
            "outColor = vec4(color, 1.0); \n "
            "}\n";


    vertexShaderSource =
            "#version 140\n"
            "in vec3 vertexPos;\n"
            "in vec3 inColor;\n"
            "out vec4 color;\n"
            "out vec3 worldPos;\n"
            "uniform mat4 view;\n"
            "uniform mat4 proj;\n"
            "uniform float opacity;\n"
            "void main(){\n"
            "color = vec4(inColor, opacity);\n"
            "worldPos = vec3(vertexPos);\n"
            "gl_Position = proj*view*vec4(vertexPos,1.0);\n"
            "}\n";


    // for colormaps where color is determined in the frag-shader
    vertexShaderNoColor =
            "#version 140\n"
            "in vec3 vertexPos;\n"
            "out vec3 worldPos;\n"
            "uniform mat4 view;\n"
            "uniform mat4 proj;\n"
            "void main(){\n"
            "worldPos = vec3(vertexPos);\n"
            "gl_Position = proj*view*vec4(vertexPos,1.0);\n"
            "}\n";


    // to create the shade on the left side of the opengl window
    vertexShaderSource2 =
            "#version 140\n"
            "in vec2 vertexPos;\n"
            "in vec4 inColor;\n"
            "out vec4 color;\n"
            "out vec3 worldPos; \n"
            "void main(){\n"
            "color = inColor;\n"
            "worldPos = vec3(vertexPos, 0.0); \n"
            "gl_Position = vec4(vertexPos, 0.0, 1.0);\n"
            "}\n";
}
