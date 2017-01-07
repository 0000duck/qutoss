#include "data.h"

// 0 - 7 denote the 8 vertices of the rectangle
// 0 - 3 denote bottom 4 vertices, 4- 7 the top 4
static QVector<QVector<int>> faces = {{7,6,2,3}, {6,5,1,2},{5,4,0,1},{4,7,3,0},{4,5,6,7}};

Data::Data() : samplesPerSide(unsigned(sqrt(NUM_SAMPLES))),
               dR(SIDE_LENGTH/(samplesPerSide-1))
{
    QVector<Point> colVec(sqrt(NUM_SAMPLES));
    QVector<QVector<Point>> copy(sqrt(NUM_SAMPLES), colVec);
    QVector<int> colVec2(sqrt(NUM_SAMPLES));
    QVector<QVector<int>> copy2(sqrt(NUM_SAMPLES), colVec2);
    gridData = copy;
    simpsonCoeffs = copy2;
    gridVertices.resize(unsigned(numVerticesAfterScaling(NUM_SAMPLES, resolution)*VERTEX_SIZE));
    gridElements.resize(unsigned(pow(4.0,(resolution-1.0))*NUM_SAMPLES*6));

    // tile vertices has capacity for rectangular tiles for every sample on dataGrid
    // +1 for the red outlining points
    tileVertices.resize(unsigned(4*NUM_SAMPLES*VERTEX_SIZE + 8*VERTEX_SIZE));
    tileElements.resize(unsigned(6*NUM_SAMPLES));
    funcMeshVertices.resize(unsigned(NUM_SAMPLES*VERTEX_SIZE));

    // number of sides x 2
    funcMeshElements.resize(unsigned(4*(samplesPerSide*samplesPerSide - samplesPerSide)));
    indicatorVertices.resize(unsigned((5 + CIRCLE_SIDES_HD + 1)*VERTEX_SIZE));
    particlePerimeter.resize(unsigned(CIRCLE_SIDES));
    brushPerimeter.resize(unsigned(CIRCLE_SIDES_HD));
    particleVertices.resize(unsigned((CIRCLE_SIDES + 1)*VERTEX_SIZE));
    netVertices.resize(unsigned(8*VERTEX_SIZE));
    netElements.resize(6*4);
    particleElements.resize(CIRCLE_SIDES*3);   // require CIRCLE_SIDES number of triangles to form a circle
    brushElements.resize(CIRCLE_SIDES_HD*3);
    buckets = vector<double>(samplesPerSide*samplesPerSide);

    initGridData();
    initMeshVertices();
    initGridVertices();
    initGridElementData();
    initFuncMeshElementData();
    initTileVertexData();
    initTileElementData();
    initNetElementData();
    initParticleElementData();
    initBrushElementData();
    initSimpsonCoeffs();
    updateBrushRadius();
    resetAnchorAndStretch();

    // initial equation to make things look a little more populated ... or not
    setEquation("0.0");

    // initialize green arrow
    setAngle(0);
    setSpeed(0);


    // colors for the points
    indicatorVertices[3] = 0;
    indicatorVertices[4] = 1.0;
    indicatorVertices[5] = 0;
    indicatorVertices[9] = 0;
    indicatorVertices[10] = 1.0;
    indicatorVertices[11] = 0;
}


void Data::resetSimulation()
{
    initGridData();
    elapsedTime = 0;
    setTiles(false);
    updateArrow();
}


void Data::restoreSaved()
{
    initialPacket = initialPacketSaved;
    updateArrow();
}


void Data::useFixedSettings()
{
    float xStart = -SIDE_LENGTH/6.0f;
    float xEnd = xStart + SIDE_LENGTH/40.0f;
    double desiredPot = 22;

    for (int i = 0; i < gridData.size(); ++i)
        for (int j = 0; j < gridData[0].size(); ++j)
        {
            float xCoord = gridData[i][j].x;
            if (xCoord >= xStart && xCoord <= xEnd)
                gridData[i][j].V = clipPotential(desiredPot);
            else
                gridData[i][j].V = 0;
        }

    setTiles(false, true);
    updateTileOrder(false);
    setCenter(0.2*SIDE_LENGTH/2.0, 0.0, false);
    setSpeed(6.5);
    setAngle(180);
}


void Data::updateArrow()
{
    double distance = 0.25*SIDE_LENGTH*initialPacket.speed/double(MAX_SPEED);
    QVector3D direction(cos(DEG_TO_RAD*initialPacket.angle), sin(DEG_TO_RAD*initialPacket.angle), 0.0);
    QVector3D center(initialPacket.xCen, initialPacket.yCen, 0.0);
    initGridData();

    arrowCenter = direction*distance + center;
    setArrow(arrowCenter);
}


void Data::setCenter(double xCen, double yCen, bool preview)
{
    setCenter({float(worldToGL(xCen)), float(worldToGL(yCen)), 0.0}, preview);
}


void Data::setXCen(double xCen)
{
    setCenter({float(worldToGL(xCen)), float(worldToGL(initialPacket.yCen)), 0.0});
}


void Data::setYCen(double yCen)
{
    setCenter({float(worldToGL(initialPacket.xCen)), float(worldToGL(yCen)), 0.0});
}


void Data::setSpeed(double speed)
{
    // must clip values to be appropriate
    speed = max(0.0, speed);
    speed = min((double)MAX_SPEED, speed);
    initialPacket.speed = speed;

    updateArrow();
}


void Data::setAngle(double angleDeg)
{
    initialPacket.angle = angleDeg;
    updateArrow();
}


void Data::setPrecision(double precision)
{
    precision = max(precision, 0.2);
    precision = min(precision, 4.0);
    initialPacket.precision = precision;

    // reset the center ...
    initialPacket.xCen = 0.0;
    initialPacket.yCen = 0.0;
    initGridData();
    emit updateGridBuffers();
}


string Data::setEquation(const QString &s)
{
    tileNum = 0;
    parser.setEquation(s.toStdString());
    string message = parser.errorCheck(gridData);

    if (!message.empty() && message != "Values were clipped for stability") // latter is not a critical message worth aborting for
        return message;

    for (int x = 0; x < gridData.size(); ++x)
        for (int y = 0; y < gridData[0].size(); ++y)
        {
            // truncate values if necessary, and if so, notify the user
            double xWorld = gridData[x][y].x;
            double yWorld = gridData[x][y].y;
            gridData[x][y].V =  parser.evaluateEquation(xWorld, yWorld);
            gridData[x][y].V = clipPotential(gridData[x][y].V);
            gridData[x][y].VPreview = gridData[x][y].V;
            int index = (x*samplesPerSide + y)*6;
            //funcMeshVertices[index+2] = potentialToGL(gridData[x][y].V);
            //funcMeshVertices[index+3] = 0;
            //funcMeshVertices[index+4] = 1.0;
            //funcMeshVertices[index+5] = 0;
        }

    setTiles(false, true);
    updateTileOrder();

    if (message.empty())
        return "all is well";
    else
        return message;
}


void Data::updateBrushRadius()
{
    brushRadius = 0;
    double spread = -brushPrecision + 25.5;

    // solve for brushRadius
    // fabs(brushHeight) must be greater than ROUND_TO_ZERO
    brushRadius = factor*spread*sqrt(-log(ROUND_TO_ZERO/fabs(brushHeight)));
}


void Data::paintPotential(const QVector3D &start, const QVector3D &direction)
{
    updateBrushRadius();

    // technically this function doesn't need these arguments (it can access them itself),
    // but for the sake of flexibility ...
    paintPotentialHelper(brushCenter , brushPrecision, brushHeight, brushRadius);
}


void Data::paintPotentialHelper(const QVector3D &worldP, double precision, double height, double radius)
{
    // find the bounding box, based on the brushRadius member variable:
    // needed for setTiles as an optimization
    int xMin = max(0, int((worldP.x() - brushRadius + SIDE_LENGTH/2.0)/dR));
    int xMax = min(int(samplesPerSide-1), int((worldP.x() + brushRadius + SIDE_LENGTH/2.0)/dR));
    int yMin = max(0, int((worldP.y() - brushRadius + SIDE_LENGTH/2.0)/dR));
    int yMax = min(int(samplesPerSide-1), int((worldP.y() + brushRadius + SIDE_LENGTH/2.0)/dR));

    // setTiles will only update things between anchor and stretch
    anchor.setX(xMin);
    anchor.setY(yMin);
    stretch.setX(xMax);
    stretch.setY(yMax);
    double spread = -brushPrecision + maxPrecision + minPrecision;

    // use a BFS to add the potentials
    QVector<QVector<int>> moves = {{1,0},{-1,0},{0,-1},{0,1}};
    int xCen = (xMax + xMin)/2.0;
    int yCen = (yMax + yMin)/2.0;
    queue<QPoint> toVisit;
    unordered_set<int> onQueue;

    toVisit.push(QPoint(xCen, yCen));
    onQueue.insert(xCen*samplesPerSide + yCen);

    while(!toVisit.empty())
    {
        QPoint cur = toVisit.front();
        double xdis = worldP.x() - gridData[cur.x()][cur.y()].x;
        double ydis = worldP.y() - gridData[cur.x()][cur.y()].y;
        gridData[cur.x()][cur.y()].V += height*exp(-(xdis*xdis + ydis*ydis)/pow(factor*spread,2.0));
        gridData[cur.x()][cur.y()].V = clipPotential(gridData[cur.x()][cur.y()].V);
        gridData[cur.x()][cur.y()].VPreview = gridData[cur.x()][cur.y()].V;

        // add new ones to the toVisit list, if not already there
        for (int i = 0; i < moves.size(); ++i)
        {
            int nextX = cur.x() + moves[i][0];
            int nextY = cur.y() + moves[i][1];

            if (min(nextX, nextY) < 0 || max(nextX, nextY) >= gridData.size())
                continue;
            if (xdis*xdis + ydis*ydis > brushRadius*brushRadius)
                break;

            QPoint next(nextX, nextY);
            int index = next.x()*gridData.size() + next.y();
            if (onQueue.find(index) == onQueue.end())
            {
                onQueue.insert(index);
                toVisit.push(next);
            }
        }

        toVisit.pop();
    }

    setTiles(false);
}


void Data::resetAnchorAndStretch()
{
    prevStretch = stretch;
    prevAnchor = anchor;
    anchor = QPoint(0,0);
    stretch = QPoint(0,0);
    previewField();
}


pair<QPoint, QPoint> Data::getAnchorAndStretch(const QPoint &anchorBlockID, const QPoint &stretchBlockID, int spacing)
{
    // the extremes that will define our new points
    QPoint newAnchor(anchorBlockID.x(), anchorBlockID.y());
    QPoint newStretch(stretchBlockID.x(), stretchBlockID.y());

    if (anchorBlockID.x() > stretchBlockID.x())
    {
        if (anchorBlockID.x() + 2*spacing - 1 <= gridData.size() - 1)
            newAnchor.setX(anchorBlockID.x() + spacing - 1);
        else
            newAnchor.setX(gridData.size() - 1);
    }
    else
    {
        if (stretchBlockID.x() + 2*spacing - 1 <= gridData.size() - 1)
            newStretch.setX(stretchBlockID.x() + spacing - 1);
        else
            newStretch.setX(gridData.size() - 1);
    }
    if (anchorBlockID.y() > stretchBlockID.y())
    {
        if (anchorBlockID.y() + 2*spacing - 1 <= gridData[0].size() - 1)
            newAnchor.setY(anchorBlockID.y() + spacing - 1);
        else
            newAnchor.setY(gridData[0].size() - 1);
    }
    else
    {
        if (stretchBlockID.y() + 2*spacing - 1 <= gridData[0].size() - 1)
            newStretch.setY(stretchBlockID.y() + spacing - 1);
        else
            newStretch.setY(gridData[0].size() - 1);
    }

    return make_pair(newAnchor, newStretch);
}


void Data::setAnchor(const QVector3D& start, const QVector3D& direction)
{
    potential = 0.2*MAX_POTENTIAL;
    prevAnchor = anchor;
    prevStretch = stretch;

    anchorBlockID = findClosestIndices(start, direction, placementSpacing);

    if (anchorBlockID.x() >= gridData.size()-1)
        anchorBlockID.setX(anchorBlockID.x() - placementSpacing + 1);
    if (anchorBlockID.y() >= gridData[0].size()-1)
        anchorBlockID.setY(anchorBlockID.y() - placementSpacing + 1);

    stretchBlockID = anchorBlockID;

    // get the appropriate bounds for stretch and anchor
    pair<QPoint, QPoint> couple = getAnchorAndStretch(anchorBlockID, stretchBlockID, placementSpacing);
    anchor = couple.first;
    stretch = couple.second;
    previewField();
}


void Data::setStretch(const QVector3D& start, const QVector3D& direction)
{
    potential = 0.2*MAX_POTENTIAL;
    prevAnchor = anchor;
    prevStretch = stretch;

    stretchBlockID = findClosestIndices(start, direction, placementSpacing);

    if (stretchBlockID.x() >= gridData.size()-1)
        stretchBlockID.setX(stretchBlockID.x() - placementSpacing);
    if (stretchBlockID.y() >= gridData[0].size()-1)
        stretchBlockID.setY(stretchBlockID.y() - placementSpacing);

    pair<QPoint, QPoint> couple = getAnchorAndStretch(anchorBlockID, stretchBlockID, placementSpacing);
    anchor = couple.first;
    stretch = couple.second;
    previewField();
}


void Data::setBrush(const QVector3D &start, const QVector3D &direction)
{
    QPoint closest = findClosestIndices(start, direction, 1);
    brushCenter = {float(indexToWorld(closest.x(), dR)), float(indexToWorld(closest.y(), dR)), 0.0};
    updateBrush();
}


// to store the vertices for the probability net, we'll use the space from tileVertices
void Data::setNetAnchor(const QVector3D& start, const QVector3D& direction)
{
    netAnchor = findClosestIndicesFlat(start, direction, 1);
    netStretch = findClosestIndicesFlat(start, direction, 1);
    setNet(netAnchor.x(), netStretch.x(), netAnchor.y(), netStretch.y(), MAX_POTENTIAL/10.0);
    emit updateNetBuffers();
}


void Data::setNetStretch(const QVector3D& start, const QVector3D& direction)
{
    netStretch = findClosestIndicesFlat(start, direction, 1);
    setNet(netAnchor.x(), netStretch.x(), netAnchor.y(), netStretch.y(), MAX_POTENTIAL/10.0);
    emit updateNetBuffers();
}


void Data::setPotentialFromStretch(double vertStretch)
{
    potential = 0.2*MAX_POTENTIAL + vertStretch*4.0*MAX_POTENTIAL;
    previewField();
}

bool Data::withinPacketVicinity(const QVector3D &GLPoint)
{
    QPoint closest = findClosestIndicesFlat(GLPoint, 1);
    int x = closest.x();
    int y = closest.y();

    double prob = pow(gridData[x][y].imCur, 2.0) + pow(gridData[x][y].reCur, 2.0);

    if (prob >= ROUND_TO_ZERO)
        return true;
    return false;
}


bool Data::withinArrowVicinity(const QVector3D &GLPoint)
{
    QVector3D worldPoint = GLToWorld(GLPoint);
    double maxDistance = 0.5;

    if (worldPoint.distanceToPoint(arrowCenter) <= maxDistance)
        return true;
    return false;
}


// p is in GLcoordinates [-1, 1]
void Data::setCenter(const QVector3D& p, bool preview)
{
    // compute the approx. radius of the current Packet
    center = findClosestIndicesFlat(QVector3D(initialPacket.xCen, initialPacket.yCen, 0), 1, true) ;  // use the current center
    int i = center.x();

    // there's guaranteed to be a direction to search along which we will not go out of bounds;
    // the radius can be defined given value of ROUND_TO_ZERO
    if (initialPacket.xCen <= 0)
    {
        while (i < gridData.size() - 1)
        {
            if (fabs(gridData[i][center.y()].reCur) < ROUND_TO_ZERO && fabs(gridData[i][center.y()].imCur) < ROUND_TO_ZERO)
                break;
            ++i;
        }
    }
    else
    {
        while (i > 0)
        {
            if (fabs(gridData[i][center.y()].reCur) < ROUND_TO_ZERO && fabs(gridData[i][center.y()].imCur) < ROUND_TO_ZERO)
                break;
            --i;
        }
    }

    // update center
    // clip coordinates here ... (the center cannot be too close to the edges, or can it?)
    double radius = fabs(indexToWorld(i, dR) - initialPacket.xCen);
    center = findClosestIndicesFlat(p, 1);
    double xCen = indexToWorld(center.x(), dR);
    double yCen = indexToWorld(center.y(), dR);
    double lowBound = -SIDE_LENGTH/2 + radius;
    double highBound = SIDE_LENGTH/2 - radius;

    xCen = min(xCen, highBound);
    xCen = max(xCen, lowBound);
    yCen = min(yCen, highBound);
    yCen = max(yCen, lowBound);

    initialPacket.xCen = xCen;
    initialPacket.yCen = yCen;
    initGridData();
    updateArrow();

    if (!preview)
        confirmPacket();
}


void Data::setVelocity(const QVector3D &p)
{
    QVector3D worldP = GLToWorld(p);
    QVector3D center(initialPacket.xCen, initialPacket.yCen, 0.0);
    QVector3D direction = (worldP - center).normalized();
    double stretch = worldP.distanceToPoint(center);
    double maxRadius = 0.25*SIDE_LENGTH;

    // 0.25*SIDE_LENGTH is mapped to MAX_SPEED = 6.0 (look at computations.h for details)
    if (stretch > maxRadius)
    {
        // calculate intersection of radius with line formed by worldP and center:
        arrowCenter = maxRadius*direction + center;
        stretch = maxRadius;
    }
    else
        arrowCenter = worldP;

    // compute the speed and angle
    double angleDeg = RAD_TO_DEG*atan(direction.y()/direction.x());

    if (direction.x() < 0)
        angleDeg += 180;
    else if (direction.y() < 0 && direction.x() >= 0)
        angleDeg += 360;

    initialPacket.angle = angleDeg;
    initialPacket.speed = stretchToSpeed(stretch);
    initGridData();

    setArrow(arrowCenter);
}


void Data::setArrow(const QVector3D &endPoint)
{
    double speedDif = fabs(initialPacketSaved.speed - initialPacket.speed);
    double angleDif = fabs(initialPacketSaved.angle - initialPacket.angle);
    bool preview = !(angleDif <= EPSILON && speedDif <= EPSILON);

    // point 1
    indicatorVertices[0] = worldToGL(initialPacket.xCen);
    indicatorVertices[1] = worldToGL(initialPacket.yCen);
    indicatorVertices[2] = 0.0001;

    // point 2
    indicatorVertices[6] = worldToGL(endPoint[0]);
    indicatorVertices[7] = worldToGL(endPoint[1]);
    indicatorVertices[8] = 0.0001;

    QVector<QVector3D> triangle = getTriangle(endPoint, initialPacket.angle, 0.5, 0.5);

    for (int i = 0; i < 3; ++i)
    {
        int index = (i + 2)*VERTEX_SIZE;
        indicatorVertices[index] = worldToGL(triangle[i][0]);
        indicatorVertices[index+1] = worldToGL(triangle[i][1]);
        indicatorVertices[index+2] = 0.0004;  // so that drawing doesn't overlap with the grid at z = 0.0

        if (!preview)
        {
            indicatorVertices[index+3] = 0.0;
            indicatorVertices[index+4] = 1.0;
            indicatorVertices[index+5] = 0.0;
        }
        else
        {
            indicatorVertices[index+3] = 1.0;
            indicatorVertices[index+4] = 0.3;
            indicatorVertices[index+5] = 0.0;
        }
    }

    if (!preview)
    {
        // colors for the points : bright green
        indicatorVertices[3] = 0;
        indicatorVertices[4] = 1.0;
        indicatorVertices[5] = 0;
        indicatorVertices[9] = 0;
        indicatorVertices[10] = 1.0;
        indicatorVertices[11] = 0;
    }
    else
    {
        indicatorVertices[3] = 1.0;
        indicatorVertices[4] = 0.3;
        indicatorVertices[5] = 0.0;
        indicatorVertices[9] = 1.0;
        indicatorVertices[10] = 0.3;
        indicatorVertices[11] = 0.0;
    }

    emit updateIndicatorBuffers();
}


void Data::previewField()
{
    addPotential(anchor.x(), stretch.x(), anchor.y(), stretch.y(), potential, true);
    setTiles(true);
}


void Data::confirmTile()
{
    addPotential(anchor.x(), stretch.x(), anchor.y(), stretch.y(), potential, false);
    setTiles(false);
    potential = 0.2*MAX_POTENTIAL;
}


void Data::confirmPacket()
{
    initialPacketSaved = initialPacket;
    updateArrow();
}


// helper function to confirmTile and previewField
void Data::addPotential(int x1, int x2, int y1, int y2, double potential, bool preview)
{
    // sync preview and current
    // this implies that we only ever preview 1 block at a time (as we reset each time)
    int startX = min(prevStretch.x(), prevAnchor.x());
    int startY = min(prevStretch.y(), prevAnchor.y());
    int endX = max(prevStretch.x(), prevAnchor.x());
    int endY = max(prevStretch.y(), prevAnchor.y());

    for (int i = startX ; i <= endX; ++i)
        for (int j = startY; j <= endY; ++j)
            gridData[i][j].VPreview = gridData[i][j].V;

    for (int i = min(x1, x2); i <= max(x1, x2); ++i)
        for (int j = min(y1, y2); j <= max(y1, y2); ++j)
        {
            if (!preview)
            {
                gridData[i][j].V += potential;
                gridData[i][j].V = clipPotential(gridData[i][j].V);
            }
            else
            {
                gridData[i][j].VPreview = gridData[i][j].V;
                gridData[i][j].VPreview += potential;
                gridData[i][j].VPreview = clipPotential(gridData[i][j].VPreview);
            }
        }
}


void Data::setMesh(bool preview)
{
    for (int i = 0; i < gridData.size(); ++i)
        for (int j = 0; j < gridData[0].size(); ++j)
        {
            double height;
            int index = (i*gridData.size() + j)*VERTEX_SIZE;
            if (preview)
                height = potentialToGL(gridData[i][j].VPreview);
            else
                height = potentialToGL(gridData[i][j].V);

            funcMeshVertices[index + 2] = height;
            funcMeshVertices[index + 3] = 0;
            funcMeshVertices[index + 4] = 1.0;
            funcMeshVertices[index + 5] = 0;
        }

    emit updateMeshBuffers();
}


void Data::updateTileOrder(bool preview)
{
    tileOrganizer.clear();

    // nlogn insertion, but sorted to make our lives easier
    for (int i = 0; i < gridData.size(); ++i)
        for (int j = 0; j < gridData[0].size(); ++j)
        {
            int index = i*gridData.size() + j;
            int val;

            if (preview)
                val = rint(gridData[i][j].VPreview*100);
            else
                val = rint(gridData[i][j].V*100);

            tileOrganizer.insert(make_pair(val, index));
        }

    // update the element buffer accordingly
    int numTiles = 0;
    for (multimap<int,int>::iterator it = tileOrganizer.begin(); it != tileOrganizer.end(); ++it)
    {
        int currentIndexElement = numTiles*6;
        int currentVertexNum = (it->second)*4;

        // first triangle
        tileElements[currentIndexElement] = currentVertexNum + 0;
        tileElements[currentIndexElement + 1] = currentVertexNum + 1;
        tileElements[currentIndexElement + 2] = currentVertexNum + 3;

        // second triangle
        tileElements[currentIndexElement + 3] = currentVertexNum + 1;
        tileElements[currentIndexElement + 4] = currentVertexNum + 2;
        tileElements[currentIndexElement + 5] = currentVertexNum + 3;
        ++numTiles;
    }

    emit updateTileEbo();
}


void Data::setTiles(bool preview, bool setAll)
{
    tileNum = 0;

    if (!setAll)
    {
        // undo old changes that werent confirmed
        for (int i = min(prevAnchor.x(), prevStretch.x()); i <= max(prevAnchor.x(), prevStretch.x()); ++i)
            for (int j = min(prevAnchor.y(), prevStretch.y()); j <= max(prevAnchor.y(), prevStretch.y()); ++j)
            {
                tileNum = i*samplesPerSide + j;
                setTile(tileNum, i, i, j, j, gridData[i][j].V, false);
            }

        for (int i = min(anchor.x(), stretch.x()); i <= max(anchor.x(), stretch.x()); ++i)
            for (int j = min(anchor.y(), stretch.y()); j <= max(anchor.y(), stretch.y()); ++j)
            {
                tileNum = i*samplesPerSide + j;

                if (preview)
                    setTile(tileNum, i, i, j, j, gridData[i][j].VPreview, false);
                else
                    setTile(tileNum, i, i, j, j, gridData[i][j].V, false);
            }
    }
    else
    {
        for (int i = 0; i < gridData.size(); ++i)
            for (int j = 0; j < gridData[0].size(); ++j)
            {
                tileNum = i*samplesPerSide + j;

                if (preview)
                    setTile(tileNum, i, i, j, j, gridData[i][j].VPreview, false);
                else
                    setTile(tileNum, i, i, j, j, gridData[i][j].V, false);
            }
    }

    if (preview)
        setTile(samplesPerSide*samplesPerSide, anchor.x(), stretch.x(), anchor.y(), stretch.y(), 0, true);

    emit updateTileVbo();
}


// helper functions to GLWidget::initData()
void Data::initGridData()
{
    int samplesPerSide = sqrt(NUM_SAMPLES);
    elapsedTime = 0.0;

    for (int x = 0; x < samplesPerSide; ++x)
    {
        double xGLPos = indexToGL(x, samplesPerSide);
        double xWorldPos = GLToWorld(xGLPos);
        for (int y = 0; y < samplesPerSide; ++y)
        {
            // note that x by itself is an index
            double yGLPos = indexToGL(y, samplesPerSide);
            double yWorldPos = GLToWorld(yGLPos);
            int index;

            index = (x*samplesPerSide + y)*6;
            cdouble cval = computeInitial(xWorldPos, yWorldPos, initialPacket);

            gridData[x][y].reBefore = cval.real();
            gridData[x][y].reCur = gridData[x][y].reBefore;
            gridData[x][y].imBefore = cval.imag();
            gridData[x][y].imCur = gridData[x][y].imBefore;
            gridData[x][y].x = xWorldPos;
            gridData[x][y].y = yWorldPos;
        }
    }

    particle.xCen = initialPacket.xCen;
    particle.yCen = initialPacket.yCen;
    particle.xVel = roundToPrecision(initialPacket.speed*cos(initialPacket.angle*DEG_TO_RAD), 2);
    particle.yVel = roundToPrecision(initialPacket.speed*sin(initialPacket.angle*DEG_TO_RAD), 2);
}


void Data::initGridVertices()
{
    // note that not every point in gridVertices is represented by a sample :
    // depending on the resolution setting, some vertices will be extrapolated
    int sideLength = sqrt(numVerticesAfterScaling(NUM_SAMPLES, resolution));

    for (int x = 0; x < sideLength; ++x)
        for (int y = 0; y < sideLength; ++y)
        {
            int index = sideLength*x + y;
            gridVertices[index*6] = worldToGL(indexToWorld(x, dR/2.0));
            gridVertices[index*6 + 1] = worldToGL(indexToWorld(y, dR/2.0));
        }
}


void Data::initMeshVertices()
{
    // find appropriate shift
    double enlargeFactor = 18.0;
    double xStart = indexToGL(0, samplesPerSide);
    double xStartEnlarged = xStart*enlargeFactor;
    double dist = xStart - xStartEnlarged;
    double gridDistEnlarged = enlargeFactor*abs(indexToGL(0, samplesPerSide) - indexToGL(1, samplesPerSide));
    double indexEnlarged = floor(dist/gridDistEnlarged);
    double xClosestEnlarged = xStartEnlarged + indexEnlarged*gridDistEnlarged;
    double shift = xStart - xClosestEnlarged - worldToGL(0.5*dR);

    for (int x = 0; x < samplesPerSide; ++x)
    {
        double xGLPos = indexToGL(x, samplesPerSide);
        for (int y = 0; y < samplesPerSide; ++y)
        {
            // note that x by itself is an index
            double yGLPos = indexToGL(y, samplesPerSide);
            int index;

            index = (x*samplesPerSide + y)*VERTEX_SIZE;

            funcMeshVertices[index] = enlargeFactor*xGLPos + shift;
            funcMeshVertices[index+1] = enlargeFactor*yGLPos + shift;
            funcMeshVertices[index+2] = -0.01;

            // color needs to be interpolated depending on distance from center
            Color cur;
            double maxDist = 6.0;
            double distArg = min(sqrt(pow(enlargeFactor*xGLPos + shift, 2.0) + pow(enlargeFactor*yGLPos + shift, 2.0))/maxDist, 1.0);

            // interpolate to background color
            cmap.getLinearInterpolated(distArg, Color(0.0, 0.0, 0.0), Color(0.16, 0.16, 0.2), cur);
            funcMeshVertices[index+3] = cur.r;
            funcMeshVertices[index+4] = cur.g;
            funcMeshVertices[index+5] = cur.b;
        }
    }
}


void Data::initGridElementData()
{
    int k = 0;
    int sideLength = sqrt(numVerticesAfterScaling(NUM_SAMPLES, resolution));

    for (int i = 0; i < sideLength - 1; ++i)
        for (int j = 0; j < sideLength - 1; ++j)
        {
            int index = sideLength*i + j;

            // for element where i and j are both odd/even, use down-slant square configuration
            // for element where i and j are different parity, use up-slant square configuration
            if (i%2 != j%2)
            {
                // first triangle
                gridElements[k*6] = index;
                gridElements[k*6+1] = index + sideLength;
                gridElements[k*6+2] = index + 1;

                // second triangle
                gridElements[k*6+3] = index + sideLength + 1;
                gridElements[k*6+4] = index + 1;
                gridElements[k*6+5] = index + sideLength;
            }
            else
            {
                // first triangle
                gridElements[k*6] = index;
                gridElements[k*6+1] = index + sideLength + 1;
                gridElements[k*6+2] = index + 1;

                // second triangle
                gridElements[k*6+3] = index;
                gridElements[k*6+4] = index + sideLength;
                gridElements[k*6+5] = index + sideLength + 1;
            }
            ++k;
        }
}


void Data::advanceSimulation(QMutex* chicken, bool onlyClassical)
{
    // first time, apply all changes in velocity and position
    if (elapsedTime == 0)
        initGridData();

    Color color;
    double refVal = 0;
    Mode mode;

    QFuture<void> supervisor = QtConcurrent::run(this, &Data::advanceSimulationClassical);

    // run for a certain amount of timesteps (to speed up the animation)
    // there is a limit to this depending on the CPU: frame lag will eventually appear if i is too great
    for (int i = 0; i < (int)simSpeed; ++i)
    {
        elapsedTime += TIME_STEP;

        if (!onlyClassical)
            verletQuantum(TIME_STEP);
    }

    // wait for classicalSimulation update to finish
    while(!supervisor.isFinished()); // { cout << "classical simulation is being slow..." << endl; }
}


void Data::verletQuantumParcel(double timeStep, int id, int numParcels)
{
    updateGridParcel(R, timeStep*0.5, id, numParcels);
    updateGridParcel(I, timeStep, id, numParcels);
    updateGridParcel(R, timeStep*0.5, id, numParcels);
}


void Data::updateGridParcel(Mode mode, double dT, int id, int numParcels)
{
    int yStart, yEnd;

    // careful, as total number of samples may not be divisible by numParcels
    // we will a flexible amount of work for id = numParcels - 1
    int width = gridData[0].size()/numParcels;
    if (id != numParcels - 1)
    {
        yStart = id*width;
        yEnd = (id+1)*width - 1;
    }
    else
    {
        yStart = id*width;
        yEnd = gridData[0].size() - 1;
    }

    if (mode == R)
    {
        for (int x = 0; x < samplesPerSide; ++x)
            for (int y = yStart; y <= yEnd; ++y)
                gridData[x][y].reCur = ::computeNext(R, x, y, dR, dT, gridData);
    }
    else
    {
        for (int x = 0; x < samplesPerSide; ++x)
            for (int y = yStart; y <= yEnd; ++y)
                gridData[x][y].imCur = ::computeNext(I, x, y, dR, dT, gridData);
    }
}


// a 4-th order symplectic algorithm
void Data::pefrlQuantum(double timeStep)
{
    // x = x + ξhv (38a)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*squiggle, gridData);

    // v = v + (1 − 2λ)hF(x)/2 (38b)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].imCur = ::computeNext(I, x, y, dR, timeStep*(1.0 - 2.0*lambda)/2.0, gridData);

    // x = x + χhv (38c)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*zeta, gridData);

    // v = v + λhF(x) (38d)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].imCur = ::computeNext(I, x, y, dR, timeStep*lambda, gridData);

    // x = x + (1 − 2(χ + ξ))hv (38e)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*(1.0 - 2.0*(zeta+squiggle)), gridData);

    // v = v + λhF(x) (38f)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].imCur = ::computeNext(I, x, y, dR, timeStep*lambda, gridData);

    // x = x + χhv (38g)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*zeta, gridData);

    // v = v + (1 − 2λ)h*F(x)/2.0 (38h)
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].imCur = ::computeNext(I, x, y, dR, timeStep*(1.0 - 2.0*lambda)/2.0, gridData);

    // x = x + ξhv
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*squiggle, gridData);
}


// if any frame lag appears, it will primarily be because of this bottleneck
void Data::verletQuantum(double timeStep)
{
    /*
    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*0.5, gridData);

    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].imCur = ::computeNext(I, x, y, dR, timeStep, gridData);

    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
            gridData[x][y].reCur = ::computeNext(R, x, y, dR, timeStep*0.5, gridData);
    */

    // to make things any faster, use gpGPU (openCL)
    QFuture<void> t = QtConcurrent::run(this, &Data::updateGridParcel, R, timeStep*0.5, 0, 2);
    updateGridParcel(R, timeStep*0.5, 1, 2);
    while (t.isRunning());

    t = QtConcurrent::run(this, &Data::updateGridParcel, I, timeStep, 0, 2);
    updateGridParcel(I, timeStep, 1, 2);
    while (t.isRunning());

    t = QtConcurrent::run(this, &Data::updateGridParcel, R, timeStep*0.5, 0, 2);
    updateGridParcel(R, timeStep*0.5, 1, 2);
    while (t.isRunning());
}


void Data::updateGrid(char drawMode, bool flat, char probsCmap)
{
    Color color;

    // only have to worry about centers, preview for arrow is handled by setArrow
    double totDif = fabs(initialPacket.xCen - initialPacketSaved.xCen) + fabs(initialPacket.yCen - initialPacketSaved.yCen);
    bool preview = totDif >= EPSILON;
    int sideLength = sqrt(numVerticesAfterScaling(NUM_SAMPLES, resolution));
    int sideScale = pow(2.0, resolution-1);

    for (int x = 0; x < samplesPerSide; ++x)
        for (int y = 0; y < samplesPerSide; ++y)
        {
            double refVal = 0;
            if (drawMode == 'P')
            {
                refVal = pow(gridData[x][y].imCur,2.0) + pow(gridData[x][y].reCur,2.0);
                gridData[x][y].refVal = refVal;

                if (probsCmap == 'H')
                    cmap.computeColor(refVal, color, 0, jetMax, 'h');
                else
                    cmap.computeColor(refVal, color, 0, jetMax, 'j');
            }
            else if (drawMode == 'R')
            {
                refVal = gridData[x][y].reCur;
                cmap.computeColor(refVal, color, -2.0*jetMax, 2.0*jetMax, 'c');
            }
            else if (drawMode == 'I')
            {
                refVal = gridData[x][y].imCur;
                cmap.computeColor(refVal, color, -2.0*jetMax, 2.0*jetMax, 'c');
            }

            int index = sideScale*(sideLength*x + y);
            if (preview)
                color = color.highlight();

            gridVertices[index*6 + 2] = refVal;
            gridVertices[index*6 + 3] = color.r;
            gridVertices[index*6 + 4] = color.g;
            gridVertices[index*6 + 5] = color.b;
        }

    interpolateBicubic(drawMode, probsCmap, preview);

    // flatten, if required (we cannot do this any earlier, as extrapolation needed the data)
    if (flat)
        for (int i = 0; i < sideLength; ++i)
            for (int j = 0; j < sideLength; ++j)
            {
                int index = i*sideLength + j;
                gridVertices[index*6 + 2] = 0.0;
            }

    emit updateGridBuffers();
}


void Data::interpolateBicubic(char drawMode, char probsCmap, bool preview)
{
    Color color;
    int sideLength = sqrt(numVerticesAfterScaling(NUM_SAMPLES, resolution));
    int sideScale = pow(2.0, resolution-1);
    QVector<QVector<double>> values = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

    for (int i = 0; i < gridData.size()-1; ++i)
        for (int j = 0; j < gridData[0].size()-1; ++j)
        {
            // the current vertex that we are is f1,1 : in 1D our interpolant requires f0, f1, f2, f3
            // accordingly, in 2D we need a 4x4 array of values.
            // initialize the values matrix that we need to interpolate in 2d
            for (int k = i-1; k <= i + 2; ++k)
                for (int l = j-1; l <= j + 2; ++l)
                {
                    if (!(k < 0 || k >= gridData.size() || l < 0 || l >= gridData.size()))
                    {
                        if (drawMode == 'P')
                            values[k - i + 1][l - j + 1] = gridData[k][l].refVal;
                        else if (drawMode == 'R')
                            values[k - i + 1][l - j + 1] = gridData[k][l].reCur;
                        else
                            values[k - i + 1][l - j + 1] = gridData[k][l].imCur;
                    }
                    else
                    {
                        values[k - i + 1][l - j + 1] = 0.0;
                    }
                }

            // most cells will require interpolation of 1) top edge mid-point, 2) left edge mid-point, 3) center
            double topVal = interpolateBicubic2D(values, 0.5, 0);
            double leftVal = interpolateBicubic2D(values, 0, 0.5);
            double centerVal = interpolateBicubic2D(values, 0.5, 0.5);
            double rightVal, botVal;

            // extra points needed at the edges
            if (i == gridData.size() - 2)
                rightVal = interpolateBicubic2D(values, 1.0, 0.5);
            if (j == gridData[0].size() - 2)
                botVal = interpolateBicubic2D(values, 0.5, 1.0);

            // plant these values into the appropriate indices in our vertex data
            int iHighRes = i*2, jHighRes = j*2;
            int indexHighRes = iHighRes*sideLength + jHighRes;
            int leftSideIndex = indexHighRes + 1;
            int topSideIndex = indexHighRes + sideLength;
            int centerIndex = indexHighRes + 1 + sideLength;
            int botIndex = indexHighRes + sideLength + 2;
            int rightIndex = indexHighRes + sideLength*2 + 1;

            gridVertices[leftSideIndex*6 + 2] = leftVal;
            gridVertices[topSideIndex*6 + 2] = topVal;
            gridVertices[centerIndex*6 + 2] = centerVal;

            if (i == gridData[0].size() - 2)
                gridVertices[rightIndex*6 + 2] = rightVal;
            if (j == gridData.size() - 2)
                gridVertices[botIndex*6 + 2] = botVal;

            char cmapCode;
            double max = jetMax, min = 0;

            // set the colors
            if (drawMode == 'P')
            {
                if (probsCmap == 'H')
                    cmapCode = 'h';
                else
                    cmapCode = 'j';
            }
            else
            {
                cmapCode = 'c';
                max = 2.0*jetMax;
                min = -2.0*jetMax;
            }

            cmap.computeColor(topVal, color, min, max, cmapCode);
            if (preview)
                color = color.highlight();
            gridVertices[topSideIndex*6 + 3] = color.r;
            gridVertices[topSideIndex*6 + 4] = color.g;
            gridVertices[topSideIndex*6 + 5] = color.b;

            cmap.computeColor(leftVal, color, min, max, cmapCode);
            if (preview)
                color = color.highlight();
            gridVertices[leftSideIndex*6 + 3] = color.r;
            gridVertices[leftSideIndex*6 + 4] = color.g;
            gridVertices[leftSideIndex*6 + 5] = color.b;

            cmap.computeColor(centerVal, color, min, max, cmapCode);
            if (preview)
                color = color.highlight();
            gridVertices[centerIndex*6 + 3] = color.r;
            gridVertices[centerIndex*6 + 4] = color.g;
            gridVertices[centerIndex*6 + 5] = color.b;

            if (i == gridData.size() - 2)
            {
                cmap.computeColor(rightVal, color, min, max, cmapCode);
                if (preview)
                    color = color.highlight();
                gridVertices[rightIndex*6 + 3] = color.r;
                gridVertices[rightIndex*6 + 4] = color.g;
                gridVertices[rightIndex*6 + 5] = color.b;
            }

            if (j == gridData[0].size() - 2)
            {
                cmap.computeColor(botVal, color, min, max, cmapCode);
                if (preview)
                    color = color.highlight();
                gridVertices[botIndex*6 + 3] = color.r;
                gridVertices[botIndex*6 + 4] = color.g;
                gridVertices[botIndex*6 + 5] = color.b;
            }
        }
}


// deprecated but possibly useful: this code is outstandingly unreadable
void Data::interpolateBilinear(char drawMode, char probsCmap, bool preview)
{
    Color color;
    int sideLength = sqrt(numVerticesAfterScaling(NUM_SAMPLES, resolution));
    int sideScale = pow(2.0, resolution-1);

    // NOTE: has not been upgraded to work with any resolution other than 2x (pixel density)
    // fill in the extrapolated points
    // can only extrapolate in stages
    int gridDataElement = 0;
    for (int i = 0; i < sideLength; ++i)
        for (int j = 0; j < sideLength; ++j)
        {
            // the points that we already marked, or are as of now indeterminate points
            if (i%sideScale == 0 && j%sideScale == 0)
            {
                ++gridDataElement;
                continue;
            }
            if (i%sideScale != 0 && j%sideScale != 0)
                continue;

            int index = i*sideLength + j;   // index of the vertex
            double refVal = 0;

            // out of access should not be possible with this approach ...
            // extrapolate using known above-below vertices
            if (i%sideScale == 0)
            {
                int topIndex = index - 1;
                int bottomIndex = index + 1;
                refVal = (gridVertices[topIndex*6 + 2] + gridVertices[bottomIndex*6 + 2]) / 2.0;
            }

            // extrapolate using known left-right vertices
            else
            {
                int leftIndex = index - sideLength;
                int rightIndex = index + sideLength;
                refVal = (gridVertices[leftIndex*6 + 2] + gridVertices[rightIndex*6 + 2])/ 2.0;
            }

            if (drawMode == 'P')
            {
                if (probsCmap == 'H')
                    cmap.computeColor(refVal, color, 0, jetMax, 'h');
                else
                    cmap.computeColor(refVal, color, 0, jetMax, 'j');
            }
            else if (drawMode == 'I')
                cmap.computeColor(refVal, color, -jetMax*2.0, jetMax*2.0, 'c');
            else if (drawMode == 'R')
                cmap.computeColor(refVal, color, -jetMax*2.0, jetMax*2.0, 'c');

            if (preview)
                color = color.highlight();

            gridVertices[index*6 + 2] = refVal;
            gridVertices[index*6 + 3] = color.r;
            gridVertices[index*6 + 4] = color.g;
            gridVertices[index*6 + 5] = color.b;
        }

    // extrapolate to the vertices along each hypotenuse
    for (int i = 0; i < samplesPerSide - 1; ++i)
        for (int j = 0; j < samplesPerSide - 1; ++j)
        {
            int index = sideScale*(sideLength*i + j) + 1 + sideLength;
            int topIndex = index - 1;
            int leftIndex = index - sideLength;
            int botIndex = index + 1;
            int rightIndex = index + sideLength;
            int topLeftIndex = index - 1 - sideLength;
            int botLeftIndex = index + 1 - sideLength;
            int topRightIndex = index - 1 + sideLength;
            int botRightIndex = index + 1 + sideLength;

            // compare 2 average values, take the one whose absolute value is higher:
            // this endorses convex-natured, not concave tiles (it's also more likely to be correct)
            double refVal1 = (gridVertices[topLeftIndex*6 + 2] + gridVertices[botRightIndex*6 + 2])/2.0;
            double refVal2 = (gridVertices[topRightIndex*6 + 2] + gridVertices[botLeftIndex*6 + 2])/2.0;
            double refVal;
            if (fabs(refVal1) > fabs(refVal2))
                refVal = refVal1;
            else
                refVal = refVal2;

            if (drawMode == 'P')
            {
                if (probsCmap == 'H')
                    cmap.computeColor(refVal, color, 0, jetMax, 'h');
                else
                    cmap.computeColor(refVal, color, 0, jetMax, 'j');
            }
            else if (drawMode == 'I')
                cmap.computeColor(refVal, color, -jetMax*2.0, jetMax*2.0, 'c');
            else if (drawMode == 'R')
                cmap.computeColor(refVal, color, -jetMax*2.0, jetMax*2.0, 'c');

            if (preview)
                color = color.highlight();

            gridVertices[index*6+2] = refVal;
            gridVertices[index*6+3] = color.r;
            gridVertices[index*6+4] = color.g;
            gridVertices[index*6+5] = color.b;
        }
}


// with outline = true, this function behaves slightly differently:
// - we dont add this tile to the element buffer, and the value of pot can be ignored
// - i.e, we simply retrieve the existing outline defined by the coordinates
void Data::setTile(int tileNum, int _x1, int _x2, int _y1, int _y2, double pot, bool outline)
{
    int currentIndexVertex = tileNum*4*VERTEX_SIZE;

    if (outline)
    {
        int x1 = min(_x1, _x2);
        int x2 = max(_x1, _x2);
        int y1 = min(_y1, _y2);
        int y2 = max(_y1, _y2);
        vector<vector<int>> indices = {{x1, y1}, {x2, y1}, {x2, y2}, {x1, y2}};
        for (int i = 0; i < 8; ++i)
        {
            double height;
            double xoffset, yoffset;
            if (i < 4) // bottom 4
                height = gridData[indices[i][0]][indices[i][1]].V;
            else       // top 4
                height = gridData[indices[i%4][0]][indices[i%4][1]].VPreview;

            if (i%4 == 0 || i%4 == 3)
                xoffset = -dR/SIDE_LENGTH;
            else
                xoffset = dR/SIDE_LENGTH;
            if (i%4 == 0 || i%4 == 1)
                yoffset = -dR/SIDE_LENGTH;
            else
                yoffset = dR/SIDE_LENGTH;

            tileVertices[currentIndexVertex + i*6] = indexToGL(indices[i%4][0], samplesPerSide)+xoffset;
            tileVertices[currentIndexVertex + i*6 + 1] = indexToGL(indices[i%4][1], samplesPerSide)+yoffset;
            tileVertices[currentIndexVertex + i*6 + 2] = potentialToGL(height);
            tileVertices[currentIndexVertex + i*6 + 3] = 1.0;
            tileVertices[currentIndexVertex + i*6 + 4] = 0;
            tileVertices[currentIndexVertex + i*6 + 5] = 0;
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            double xoffset, yoffset;

            if (i%4 == 0 || i%4 == 3)
                xoffset = -dR/SIDE_LENGTH;
            else
                xoffset = dR/SIDE_LENGTH;
            if (i%4 == 0 || i%4 == 1)
                yoffset = -dR/SIDE_LENGTH;
            else
                yoffset = dR/SIDE_LENGTH;

            tileVertices[currentIndexVertex + i*6] = indexToGL(_x1,samplesPerSide)+xoffset;//indexToGL(indices[i%4][0], samplesPerSide)+xoffset;
            tileVertices[currentIndexVertex + i*6 + 1] = indexToGL(_y1,samplesPerSide)+yoffset;//indexToGL(indices[i%4][1], samplesPerSide)+yoffset;
            tileVertices[currentIndexVertex + i*6 + 2] = potentialToGL(pot);
            tileVertices[currentIndexVertex + i*6 + 3] = cmap.potentialToColor(pot);
            tileVertices[currentIndexVertex + i*6 + 4] = cmap.potentialToColor(pot);
            tileVertices[currentIndexVertex + i*6 + 5] = cmap.potentialToColor(pot);
        }
    }
}


void Data::setNet(int x1, int x2, int y1, int y2, double _height)
{
    vector<vector<int>> indices = {{x1, y1}, {x2, y1}, {x2, y2}, {x1, y2}};
    for (int i = 0; i < 8; ++i)
    {
        double height;
        double xoffset, yoffset;
        if (i < 4) // bottom 4
            height = 0;
        else       // top 4
            height = _height;

        if (i%4 == 0 || i%4 == 3)
            xoffset = -dR/SIDE_LENGTH;
        else
            xoffset = dR/SIDE_LENGTH;
        if (i%4 == 0 || i%4 == 1)
            yoffset = -dR/SIDE_LENGTH;
        else
            yoffset = dR/SIDE_LENGTH;

        netVertices[i*6] = indexToGL(indices[i%4][0], samplesPerSide)+xoffset;
        netVertices[i*6 + 1] = indexToGL(indices[i%4][1], samplesPerSide)+yoffset;
        netVertices[i*6 + 2] = potentialToGL(height);
        netVertices[i*6 + 3] = 0.0;
        netVertices[i*6 + 4] = 1.0;
        netVertices[i*6 + 5] = 0.0;
    }
}


void Data::initTileVertexData()
{
    tileNum = 0;
    for (int i = 0; i < gridData.size(); ++i)
        for (int j = 0; j < gridData[0].size(); ++j)
        {
            setTile(tileNum, i, i, j, j, gridData[i][j].VPreview, false);
            ++tileNum;
        }
}


void Data::initTileElementData()
{
    for (int tile = 0; tile*6 < tileElements.size(); ++tile)
    {
        int currentIndexElement = tile*6;
        int currentVertexNum = tile*4;

        // first triangle
        tileElements[currentIndexElement] = currentVertexNum + 0;
        tileElements[currentIndexElement + 1] = currentVertexNum + 1;
        tileElements[currentIndexElement + 2] = currentVertexNum + 3;

        // second triangle
        tileElements[currentIndexElement + 3] = currentVertexNum + 1;
        tileElements[currentIndexElement + 4] = currentVertexNum + 2;
        tileElements[currentIndexElement + 5] = currentVertexNum + 3;
    }
}


void Data::initFuncMeshElementData()
{
    int curElement = 0;
    int spacing = 1;

    // for each x, connect along y-axis
    for (int i = 0; i < gridData.size(); i += spacing)
        for (int j = 0; j < gridData[0].size() - 1; ++j)
        {
            int index = i*gridData.size() + j;
            funcMeshElements[curElement++] = index;
            funcMeshElements[curElement++] = index + 1;
        }

    // connect along x-axis
    for (int j = 0; j < gridData[0].size(); j += spacing)
        for (int i = 0; i < gridData.size() - 1; ++i)
        {
            int index = i*gridData.size() + j;
            funcMeshElements[curElement++] = index;
            funcMeshElements[curElement++] = index + gridData.size();
        }
}


void Data::initNetElementData()
{
    for (int i = 0; i < 5; ++i)
    {
        if (i != 4)
        {
            // first triangle
            netElements[i*6] = faces[i][0];
            netElements[i*6 + 1] = faces[i][1];
            netElements[i*6 + 2] = faces[i][3];

            // second triangle
            netElements[i*6 + 3] = faces[i][3];
            netElements[i*6 + 4] = faces[i][1];
            netElements[i*6 + 5] = faces[i][2];
        }
    }
}


void Data::initParticleElementData()
{
    for (int i = 0; i < CIRCLE_SIDES; ++i)
    {
        // one triangle for each side
        int index = i*3;
        particleElements[index] = 0;
        particleElements[index + 1] = i + 1 ;

        if (i + 1 == CIRCLE_SIDES)
            particleElements[index + 2] = 1;
        else
            particleElements[index + 2] = i + 2 ;
    }
}


// almost identical in form to previous function actually
void Data::initBrushElementData()
{
    int offset = 5;
    for (int i = 0; i < CIRCLE_SIDES_HD; ++i)
    {
        // one triangle for each side
        int index = i*3;
        brushElements[index] = 0 + offset;
        brushElements[index + 1] = i + 1 + offset;

        if (i + 1 == CIRCLE_SIDES_HD)
            brushElements[index + 2] = 1 + offset;
        else
            brushElements[index + 2] = i + 2 + offset;
    }
}


void Data::initSimpsonCoeffs()
{
    // note that there must be an odd number of grid points in order for this to work
    if (simpsonCoeffs.size()%2 == 0 || simpsonCoeffs[0].size()%2 == 0)
    {
        //cout << "Proper simpson's rule integration requires an odd number of grid points" << endl;
        return;
    }

    int iCoeff, jCoeff;
    for (int i = 0; i < simpsonCoeffs.size(); ++i)
        for (int j = 0; j < simpsonCoeffs[0].size(); ++j)
        {
            if (i == simpsonCoeffs.size()-1 || i == 0)
                iCoeff = 1;
            else if (i%2 == 0)
                iCoeff = 2;
            else
                iCoeff = 4;

            if (j == simpsonCoeffs[0].size()-1 || j == 0)
                jCoeff = 1;
            else if (j%2 == 0)
                jCoeff = 2;
            else
                jCoeff = 4;

            simpsonCoeffs[i][j] = iCoeff*jCoeff;
        }
}


void Data::updateParticle(const QVector3D& cameraPos, const Color& color)
{
    particleVertices[0] = worldToGL(particle.xCen);
    particleVertices[1] = worldToGL(particle.yCen);
    particleVertices[2] = 0.0;
    particleVertices[3] = color.r;
    particleVertices[4] = color.g;
    particleVertices[5] = color.b;

    // this is normalized within getCircle
    QVector3D center(particle.xCen, particle.yCen, 0.0);
    QVector3D axis = GLToWorld(cameraPos) - center;

    // output is stored in particlePerimeter
    getCircle(center, 0.1, CIRCLE_SIDES, particlePerimeter, axis);

    for (int i = 1; i < CIRCLE_SIDES + 1; ++i)
    {
        int index = i*VERTEX_SIZE;
        particleVertices[index] = worldToGL(particlePerimeter[i-1].x());
        particleVertices[index + 1] = worldToGL(particlePerimeter[i-1].y());
        particleVertices[index + 2] = worldToGL(particlePerimeter[i-1].z());
        particleVertices[index + 3] = color.r;
        particleVertices[index + 4] = color.g;
        particleVertices[index + 5] = color.b;
    }

    emit updateParticleBuffers();
}


void Data::setBrushParams(double _brushHeight, double _brushPrecision)
{
    brushHeight = _brushHeight;
    brushPrecision = _brushPrecision;
    updateBrush();
}


void Data::updateBrush()
{
    updateBrushRadius();

    // indicator vertices is shared between 3 different objects
    int offset = 5*VERTEX_SIZE;
    getCircle(brushCenter, brushRadius, CIRCLE_SIDES_HD, brushPerimeter, QVector3D(0.0, 0.0, 1.0));

    double xPos, yPos;
    double height = potentialToGL(getPotential(brushCenter.x(), brushCenter.y()));

    // might be near the boundaries
    height = min(height, potentialToGL(double(MAX_POTENTIAL)));
    height = max(height, -potentialToGL(double(MAX_POTENTIAL)));

    indicatorVertices[0 + offset] = worldToGL(brushCenter.x());
    indicatorVertices[1 + offset] = worldToGL(brushCenter.y());
    indicatorVertices[2 + offset] = height;
    indicatorVertices[3 + offset] = 1.0;
    indicatorVertices[4 + offset] = 0.0;
    indicatorVertices[5 + offset] = 0.0;

    for (int i = 1; i < CIRCLE_SIDES_HD + 1; ++i)
    {
        int index = i*VERTEX_SIZE;
        indicatorVertices[index + offset] = worldToGL(brushPerimeter[i-1].x());
        indicatorVertices[index + 1 + offset] = worldToGL(brushPerimeter[i-1].y());
        indicatorVertices[index + 2 + offset] = height;
        indicatorVertices[index + 3 + offset] = 0.2;
        indicatorVertices[index + 4 + offset] = 0.0;
        indicatorVertices[index + 5 + offset] = 0.0;
    }

    emit updateIndicatorBuffers();
}


// implemented using a BFS
QPoint Data::searchProximity(const QPoint &start, int target, int maxRadius)
{
    queue<QPoint> toVisit;
    unordered_set<int> onList;
    toVisit.push(start);
    QVector<QVector<int>> moves = { {0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    while (!toVisit.empty())
    {
        QPoint cur = toVisit.front();
        int x = cur.x();
        int y = cur.y();

        if (!(x < 0 || x >= gridData.size() || y < 0 || y >= gridData[0].size()))
            if (rint(gridData[x][y].V) == target)
                return cur;

        for (int i = 0; i < 4; ++i)
        {
            int newX = x + moves[i][0];
            int newY = y + moves[i][1];
            int index = newX*gridData.size() + newY; // uniqueness check

            // check for bounds, radius, and uniqueness
            if (newX >= 0 && newX < gridData.size() && newY >= 0 && newY < gridData[0].size())
                if (pow(newX-start.x(), 2.0) + pow(newY - start.y(), 2.0) < maxRadius*maxRadius)
                    if (onList.find(index) == onList.end())
                    {
                        toVisit.push(QPoint(newX, newY));
                        onList.insert(index);
                    }
        }
        toVisit.pop();
    }

    // fruitless
    return QPoint(-1, -1);
}


QPoint Data::findClosestIndices(const QVector3D &start, const QVector3D &direction, int spacing)
{
    list<QPoint> candidates;
    QPoint closest;
    QVector3D flatIntersect;
    double x, y;
    int xIndex, yIndex, maxIndexDist = 10;

    for (int h = int(-MAX_POTENTIAL); h <= int(MAX_POTENTIAL); ++h)
    {
        // solving for intersection with z = h:
        // a*direction[2] + start[2] = h   =>  a = (h-start[2])/direction[2]
        float a;

        if (direction[2] != 0)
            a = (potentialToGL(h)-start[2])/direction[2];
        else
            a = -(potentialToGL(h)-start[2])/1E-6;

        x = GLToWorld(a*direction[0] + start[0]);
        y = GLToWorld(a*direction[1] + start[1]);

        // normalize the coordinates to be within bounds
        x = max(x , -SIDE_LENGTH/2.0);
        x = min(x , SIDE_LENGTH/2.0);
        y = max(y, -SIDE_LENGTH/2.0);
        y = min(y, SIDE_LENGTH/2.0);

        if (h == 0)
            flatIntersect = {float(x), float(y), 0.0};

        // find the closest indices (similar to procedure used in findClosestIndicesFlat)
        xIndex = rint((x + SIDE_LENGTH/2.0)/dR);
        yIndex = rint((y + SIDE_LENGTH/2.0)/dR);

        // do a search within the proximity for grid points with this potential
        QPoint candidate = searchProximity(QPoint(xIndex, yIndex), h, maxIndexDist);

        // return val of -1 if search did not yield anything
        if (candidate.x() != -1)
        {
            candidates.push_back(candidate);
        }
    }

    // sketchy method (as we are not using surfaces in our computations)
    // procedure to get the best candidate:
    // 1) take all candidates within a certain distance from the line
    // 2) return the one closest to the camera
    // 3) if there aren't any, increase the threshold, and repeat from the top
    if (!candidates.empty())
    {
        QVector3D startWorld = GLToWorld(start);
        QVector3D dirWorld = GLToWorld(direction).normalized();
        bool foundWithin = false;

        for (int i = 1; i <= maxIndexDist; ++i)
        {
            double threshold = i*dR;

            // compile list of candidates within threshold distance of the line
            list<QPoint> within;
            for (list<QPoint>::iterator it = candidates.begin(); it != candidates.end(); ++it)
            {
                float x = gridData[it->x()][it->y()].x;
                float y = gridData[it->x()][it->y()].y;
                QVector3D candidate = {x, y, float(potentialToWorld(float(gridData[it->x()][it->y()].V)))};
                float distance = candidate.distanceToLine(startWorld, dirWorld);

                if (distance <= threshold)
                    within.push_back(*it);
            }

            // return the closest point within this list
            double minDist = numeric_limits<double>::max();
            for (list<QPoint>::iterator it = within.begin(); it != within.end(); ++it)
            {
                float x = gridData[it->x()][it->y()].x;
                float y = gridData[it->x()][it->y()].y;
                QVector3D candidate = {x, y, float(potentialToWorld(float(gridData[it->x()][it->y()].V)))};
                float distance = candidate.distanceToPoint(startWorld);

                if (distance < minDist)
                {
                    minDist = distance;
                    closest = QPoint(it->x(), it->y());
                    foundWithin = true;
                }
            }

            if (!within.empty())
                break;
        }

        // if nothing was found using the above method, just take the closest candidate
        if (!foundWithin)
        {
            double minDist = numeric_limits<double>::max();
            for (list<QPoint>::iterator it = candidates.begin(); it != candidates.end(); ++it)
            {
                float x = gridData[it->x()][it->y()].x;
                float y = gridData[it->x()][it->y()].y;
                QVector3D candidate = {x, y, float(potentialToWorld(float(gridData[it->x()][it->y()].V)))};
                float distance = candidate.distanceToPoint(startWorld);

                if (distance < minDist)
                {
                    closest = QPoint(it->x(), it->y());
                    minDist = distance;
                }
            }
        }

        // adjust according to spacing rules
        int adjustedX = spacing*rint(float(closest.x())/float(spacing));
        int adjustedY = spacing*rint(float(closest.y())/float(spacing));
        int endX = spacing*floor(float(gridData.size()-1)/float(spacing));
        int endY = spacing*floor(float(gridData[0].size()-1)/float(spacing));

        adjustedX = min(adjustedX, endX);
        adjustedX = max(adjustedX, 0);
        adjustedY = min(adjustedY, endY);
        adjustedY = max(adjustedY, 0);

        return QPoint(adjustedX, adjustedY);
    }
    else
    {
        return findClosestIndicesFlat(start, direction, spacing);
    }

}


// worldCoords is by default, false (therefore assuming that we will have to perform conversion)
QPoint Data::findClosestIndicesFlat(const QVector3D &coords, int spacingInt, bool worldCoords)
{
    QVector3D converted;
    if (!worldCoords)
        converted = GLToWorld(coords);
    else
        converted = coords;

    double spacing = spacingInt;
    int xIndex = spacing * rint ((converted.x() + SIDE_LENGTH/2.0)/(spacing*dR));
    int yIndex = spacing * rint ((converted.y() + SIDE_LENGTH/2.0)/(spacing*dR));
    int endX = spacing*floor(float(gridData.size()-1)/float(spacing));
    int endY = spacing*floor(float(gridData[0].size()-1)/float(spacing));

    xIndex = max(0, xIndex);
    xIndex = min(xIndex, endX);
    yIndex = max(0, yIndex);
    yIndex = min(yIndex, endY);

    return QPoint(xIndex, yIndex);
}


QPoint Data::findClosestIndicesFlat(const QVector3D &start, const QVector3D &direction, int spacing)
{
    // find the intersection :
    // a*direction[2] + start[2] = 0
    double a = -start[2]/direction[2];
    QVector3D intersect = start + a*direction;
    return findClosestIndicesFlat(intersect, spacing);
}


// finds the probability that the Packet is within the given rectangle
// tried both a Riemann sum and Simpson's Rule:
// improvements from Simpson's Rule were ... mediocre if any at all
double Data::getProbability(int x1, int x2, int y1, int y2)
{
    double sum = 0;

    for (int i = min(x1,x2); i <= max(x1, x2); ++i)
        for (int j = min(y1,y2); j <= max(y1,y2); ++j)
            sum += pow(gridData[i][j].reCur,2) + pow(gridData[i][j].imCur,2);

    double factor = dR*dR;
    return sum*factor;
}


bool Data::findRectangle(Block& rect, int x, int y, bool preview)
{
    if (gridData[x][y].covered || (gridData[x][y].V == 0 && !preview))
        return false;
    if (gridData[x][y].covered || (gridData[x][y].VPreview == 0 && preview))
        return false;

    double refPotential;

    if (!preview)
        refPotential = gridData[x][y].V;
    else
        refPotential = gridData[x][y].VPreview;

    int rIndex = x;
    int dIndex = y + 1;

    // go as far to the right as possible
    if (!preview)
    {
        while (rIndex < gridData.size() && gridData[rIndex][y].V == refPotential && !gridData[rIndex][y].covered)
            ++rIndex;
    }
    else
    {
        while (rIndex < gridData.size() && gridData[rIndex][y].VPreview == refPotential && !gridData[rIndex][y].covered)
            ++rIndex;
    }

    bool done = false;

    // go as far down as possible with this row
    for (; dIndex < gridData[0].size() && !done; ++dIndex)
        for (int i = x; i < rIndex && !done; ++i)
        {
            double currentV;
            if (!preview)
                currentV = gridData[i][dIndex].V ;
            else
                currentV = gridData[i][dIndex].VPreview;
            if (currentV != refPotential || gridData[i][dIndex].covered)
            {
                done = true;
                --dIndex;   // ignore the last row (as it was invalidated in some way)
            }
        }

    for (int i = x; i < rIndex; ++i)
        for (int j = y; j < dIndex; ++j)
            gridData[i][j].covered = true;

    // rectangle defined from [x,rIndex) and [y,dIndex)
    rect.xLeft = x;
    rect.xRight = rIndex - 1;
    rect.yLeft = y;
    rect.yRight = dIndex - 1;

    return true;
}


double Data::getPotential(double xWorld, double yWorld, bool discretized)
{
    // boundary is technically an infinite potential wall
    if (xWorld<= -SIDE_LENGTH/2.0 || xWorld>= SIDE_LENGTH/2.0 || yWorld<= -SIDE_LENGTH/2.0 || yWorld>= SIDE_LENGTH/2.0)
        return 100.0;

    int xIndex = rint((xWorld + SIDE_LENGTH/2.0)/(dR));
    int yIndex = rint((yWorld + SIDE_LENGTH/2.0)/(dR));

    if (discretized)
    {
        // use the potential as given by gridData
        return gridData[xIndex][yIndex].V;
    }

    // evaluate the equation and compare: this is obviously more costly than the above
    else
    {
        double val = parser.evaluateEquation(xWorld, yWorld);

        // if there is a certain error at this spot, it means the user added discrete potentials
        if (fabs(val - gridData[xIndex][yIndex].V) >= 1.0)
            val = gridData[xIndex][yIndex].V;

        return val;
    }
}


void Data::computeGradient(double xWorld, double yWorld, QVector2D &out)
{
    // set endpoints
    double stepSize = 0.51*dR;
    double xLeft = xWorld - stepSize;
    double xRight = xWorld + stepSize;
    double yTop = yWorld - stepSize;
    double yBot = yWorld + stepSize;

    // 2nd order error
    double gradX = (getPotential(xRight, yWorld, false) - getPotential(xLeft, yWorld, false))/(2.0*stepSize);
    double gradY = (getPotential(xWorld, yBot, false) - getPotential(xWorld, yTop, false))/(2.0*stepSize);

    out.setX(gradX);
    out.setY(gradY);
}


// run on a separate thread, concurrently with the other
// takes a surprising amount of work to simulate a classical particle to below 1% error
void Data::advanceSimulationClassical()
{
    for (int i = 0; i < (int)simSpeed; ++i)
    {
        int numRepeat = 80;
        for (int i = 0; i < numRepeat; ++i)
        {
            verletClassical(frCoefficient*TIME_STEP/numRepeat);
            verletClassical(frComplement*TIME_STEP/numRepeat);
            verletClassical(frCoefficient*TIME_STEP/numRepeat);
        }
    }

    // TESTING
    //double potentialEnergy = getPotential(particle.xCen, particle.yCen, false);
    //double kineticEnergy = 0.5*(pow(particle.xVel,2.0) + pow(particle.yVel,2.0));
    //cout << potentialEnergy + kineticEnergy << endl;
}


double Data::computeNext(Mode mode, int x, int y, double dT)
{
    double laplacianApprox;
    double valBefore;

    // xl = value at data[x-1][y], xll = value at data[x-2][y]
    // yt = value at data[x][y-1], etc.
    // note that the endpoints have an infinite potential behind them
    double xl = 0, xr = 0, yt = 0, yb = 0, centerVal = 0;
    double xll = 0, xrr = 0, ytt = 0, ybb = 0;
    double size = gridData.size();

    if (mode == R)
    {
        valBefore = gridData[x][y].reCur;
        centerVal = gridData[x][y].imCur;
        if (x > 0)
        {
            xl = gridData[x-1][y].imCur;
            if (x - 1 > 0)
                xll = gridData[x-2][y].imCur;
        }
        if (x < size-1)
        {
            xr = gridData[x+1][y].imCur;
            if (x + 1 < size - 1)
                xrr = gridData[x+2][y].imCur;
        }
        if (y > 0)
        {
            yt = gridData[x][y-1].imCur;
            if (y - 1 > 0)
                ytt = gridData[x][y-2].imCur;
        }
        if (y < size-1)
        {
            yb = gridData[x][y+1].imCur;
            if (y + 1 < size - 1)
                ybb = gridData[x][y+2].imCur;
        }
    }
    else if (mode == I)
    {
        valBefore = gridData[x][y].imCur;
        centerVal = gridData[x][y].reCur;
        if (x > 0)
        {
            xl = gridData[x-1][y].reCur;
            if (x - 1 > 0)
                xll = gridData[x-2][y].reCur;
        }
        if (x < size-1)
        {
            xr = gridData[x+1][y].reCur;
            if (x + 1 < size-1)
                xrr = gridData[x+2][y].reCur;
        }
        if (y > 0)
        {
            yt = gridData[x][y-1].reCur;
            if (y - 1 > 0)
                ytt = gridData[x][y-2].reCur;
        }
        if (y < size-1)
        {
            yb = gridData[x][y+1].reCur;
            if (y + 1 < size - 1)
                ybb = gridData[x][y+2].reCur;
        }
    }

    laplacianApprox = (-(xll + xrr + ytt + ybb)/12.0 + 4.0*(xl + xr + yt + yb)/3.0 - 5.0*centerVal)/(dR*dR);

    if (mode == R)
        return valBefore + dT*(-laplacianApprox/2.0 + gridData[x][y].V*centerVal);;
    if (mode == I)
        return valBefore - dT*(-laplacianApprox/2.0 + gridData[x][y].V*centerVal);;

    return 0.0;
}


void Data::verletClassical(double timeStep)
{
    // repeat for all particles
    QVector2D gradient;
    computeGradient(particle.xCen, particle.yCen, gradient);

    // velocity verlet
    particle.xVel += -gradient.x()*0.5*timeStep;
    particle.yVel += -gradient.y()*0.5*timeStep;

    particle.xCen += particle.xVel*timeStep;
    particle.yCen += particle.yVel*timeStep;

    computeGradient(particle.xCen, particle.yCen, gradient);
    particle.xVel += -gradient.x()*0.5*timeStep;
    particle.yVel += -gradient.y()*0.5*timeStep;
}


bool Data::findBoundingGrid(QVector<QPoint> &bounds, double xWorld, double yWorld)
{
    QPoint closestIndices = findClosestIndicesFlat({float(xWorld), float(yWorld), 0.0},1, true);
    QVector2D closest = { float(indexToWorld(closestIndices.x(), dR)), float(indexToWorld(closestIndices.y(), dR))};

    // bounding rect depends on where Packet is relative to 'closest'

    // to simplify code (but possibly worsen readability)
    // vector of all possible points that we may use to define the bounds
    QVector<QPoint> lattice(9);
    int xBase = closestIndices.x() - 1;
    int yBase = closestIndices.y() - 1;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            lattice[3*i + j] = QPoint(i + xBase, j + yBase);

    if (closest.x() - xWorld >= 0)
    {
        if (closest.y() - yWorld >= 0)  // quadrant 2
        {
            bounds[0] = lattice[0];
            bounds[1] = lattice[1];
            bounds[2] = lattice[3];
            bounds[3] = lattice[4];
        }
        else  // quadrant 3
        {
            bounds[0] = lattice[3];
            bounds[1] = lattice[4];
            bounds[2] = lattice[6];
            bounds[3] = lattice[7];
        }
    }
    else
    {
        if (closest.y() - yWorld >= 0)  // quadrant 1
        {
            bounds[0] = lattice[1];
            bounds[1] = lattice[2];
            bounds[2] = lattice[4];
            bounds[3] = lattice[5];
        }
        else   // quadrant 4
        {
            bounds[0] = lattice[4];
            bounds[1] = lattice[5];
            bounds[2] = lattice[7];
            bounds[3] = lattice[8];
        }
    }

    // check for out of bounds :
    // note that the bounds have top-left as first coord
    // bottom-right as last coord
    if (bounds[0].x() < 0 || bounds[0].y() < 0)
        return false;
    if (bounds[3].x() >= gridData.size() || bounds[3].y() >= gridData.size())
        return false;
    return true;
}


void Data::observe(double precision, double timeLimit)
{
    // fix the remaining time to 5 seconds
    elapsedTime = 0.95*timeLimit;
    setupBucketsPosition();
    QVector2D position = observePosition();

    double sum = 0;

    // here is a somewhat problematic thing :
    // we will describe the newly measured wavefunction with a Gaussian peak (I do not know how legit this is),
    // as describing it as a piecewise-radial function causes discretization artefacts
    for (int i = 0; i < samplesPerSide; ++i)
        for (int j = 0; j < samplesPerSide; ++j)
        {
            double xdis = gridData[i][j].x - position.x();
            double ydis = gridData[i][j].y - position.y();
            double distance = sqrt(pow(xdis,2) + pow(ydis,2));

            gridData[i][j].reCur *= exp(-distance*distance*precision)/sqrt(PI/(2.0*precision));
            gridData[i][j].imCur *= exp(-distance*distance*precision)/sqrt(PI/(2.0*precision));
            sum += dR*dR*(pow(gridData[i][j].reCur,2.0) + pow(gridData[i][j].imCur, 2.0));
        }

    for (int i = 0; i < samplesPerSide; ++i)
        for (int j = 0; j < samplesPerSide; ++j)
        {
            gridData[i][j].reCur *= sqrt(1.0/sum);
            gridData[i][j].imCur *= sqrt(1.0/sum);
        }
}


void Data::setupBucketsMomentum()
{
    // take the fft
}


void Data::setupBucketsPosition()
{
    double sum = 0;
    for (int i = 0; i < gridData.size(); ++i)
        for (int j = 0; j < gridData[0].size(); ++j)
        {
            double probDensity = pow(gridData[i][j].reCur,2) + pow(gridData[i][j].imCur,2);
            double probability = probDensity*dR*dR;
            int index = i*gridData.size() + j;

            sum += probability;
            buckets[index] = sum;
        }
}


// setupBuckets must have been called for observePosition to work on current wavefunction
QVector2D Data::observePosition()
{
    // get random element between 0 - 1:
    // 1) seed generator
    // 2) use generator to determine entropy used in uniform distribution sampling
    random_device basicRand;
    mt19937 generator(basicRand());
    uniform_real_distribution<> distribution(0,1);

    double randNum = distribution(generator);
    int index = binarySearch(buckets, randNum);

    // there's a possibility that the given index landed on a bucket with 0-width
    // on a personal note, I always miss these corner cases, hash-tables and array-indexing = my bane
    if (randNum != 0)
        while (index > 0 && buckets[index-1] == buckets[index])
                --index;
    else
        while (index < buckets.size() && buckets[index] == 0)
                ++index;

    int i = floor(double(index) / (double)gridData.size());
    int j = index % gridData[0].size();

    return { float(gridData[i][j].x), float(gridData[i][j].y)};
}


double Data::getProbFromIndex(int index)
{
    int x = floor(index/samplesPerSide);
    int y = index%samplesPerSide;
    return pow(gridData[x][y].reCur, 2.0) + pow(gridData[x][y].imCur, 2.0);
}




