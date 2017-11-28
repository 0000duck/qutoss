#include "helpers.h"


void slideWidget(QWidget* target, const QPoint& to, float speedup)
{
    QPoint from = target->geometry().topLeft();
    QVector2D dir = QVector2D(to - from);

    // use an interval from [0,1] as our output, when multiplied with dir
    float endTime = PI/2.0;
    float dt = speedup*endTime/60.0;
    QElapsedTimer timer;

    for (float t = 0; t < endTime; t += dt)
    {
        timer.start();
        float funcVal = sin(t)*sin(t);
        QVector2D pos = QVector2D(from.x(), from.y()) + funcVal*dir;
        target->setGeometry(pos.x(), pos.y(), target->width(), target->height());
        while(timer.elapsed() < MS_PER_FRAME);
    }
}

void print(const char *line)
{
    cout << line << endl;
}

void print(const QString &line)
{
    cout << line.toStdString() << endl;
}

void print(const string &line)
{
    cout << line << endl;
}

/*
Equations that govern this method:
http://scitation.aip.org/docserver/fulltext/aip/journal/cip/5/6/1.168415.pdf?expires=1468545176&id=id&accname=guest&checksum=26EA13A98DB17B56AC305A59F34126C0

dR/dt = HI
dI/dt = -HR
H = -del^2/2m + V

three-point approximation for del^2 (using e.g phi for the operator)
del^2_phi = [phi(r + deltar) - 2*phi(r) + phi(r - deltar)]/(deltar)^2
*/


cdouble computeInitial(double x, double y, const Packet& packet)
{
    double xV0 = roundToPrecision(packet.speed*cos(packet.angle*DEG_TO_RAD), 2);
    double yV0 = roundToPrecision(packet.speed*sin(packet.angle*DEG_TO_RAD), 2);
    cdouble val = 0;
    double tightenFactor = packet.precision;
    double normFactor = sqrt(PI/(2.0*tightenFactor));

    x -= packet.xCen;
    y -= packet.yCen;

    val = exp(tightenFactor*(-x*x-y*y))*cos(x*xV0 + y*yV0);
    val += sqrt(cdouble(-1))*exp(tightenFactor*(-x*x-y*y))*sin(x*xV0 + y*yV0);
    val /= normFactor;

    return val;
}


double computeNext(Mode mode, int x, int y, double dR, double dT, const QVector<QVector<Point>>& data)
{
    double laplacianApprox;
    double valBefore;

    // xl = value at data[x-1][y], xll = value at data[x-2][y]
    // yt = value at data[x][y-1], etc.
    // note that the endpoints have an infinite potential behind them
    double xl = 0, xr = 0, yt = 0, yb = 0, centerVal = 0;
    double xll = 0, xrr = 0, ytt = 0, ybb = 0;
    double xlll = 0, xrrr = 0, yttt = 0, ybbb = 0;
    double size = data.size();

    if (mode == R)
    {
        valBefore = data[x][y].reCur;
        centerVal = data[x][y].imCur;
        if (x > 0)
        {
            xl = data[x-1][y].imCur;
            if (x - 1 > 0)
            {
                xll = data[x-2][y].imCur;
                if (x - 2 > 0)
                    xlll = data[x-3][y].imCur;
            }
        }
        if (x < size-1)
        {
            xr = data[x+1][y].imCur;
            if (x + 1 < size - 1)
            {
                xrr = data[x+2][y].imCur;
                if (x + 2 < size - 1)
                    xrrr = data[x+3][y].imCur;
            }

        }
        if (y > 0)
        {
            yt = data[x][y-1].imCur;
            if (y - 1 > 0)
            {
                ytt = data[x][y-2].imCur;
                if (y - 2 > 0)
                    yttt = data[x][y-3].imCur;
            }

        }
        if (y < size-1)
        {
            yb = data[x][y+1].imCur;
            if (y + 1 < size - 1)
            {
                ybb = data[x][y+2].imCur;
                if (y + 2 < size - 1)
                    ybbb = data[x][y+3].imCur;
            }
        }
    }
    else if (mode == I)
    {
        valBefore = data[x][y].imCur;
        centerVal = data[x][y].reCur;
        if (x > 0)
        {
            xl = data[x-1][y].reCur;
            if (x - 1 > 0)
            {
                xll = data[x-2][y].reCur;
                if (x - 2 > 0)
                    xlll = data[x-3][y].reCur;
            }
        }
        if (x < size-1)
        {
            xr = data[x+1][y].reCur;
            if (x + 1 < size-1)
            {
                xrr = data[x+2][y].reCur;
                if (x + 2 < size - 1)
                    xrrr = data[x+3][y].reCur;
            }
        }
        if (y > 0)
        {
            yt = data[x][y-1].reCur;
            if (y - 1 > 0)
            {
                ytt = data[x][y-2].reCur;
                if (y - 2 > 0)
                    yttt = data[x][y-3].reCur;
            }
        }
        if (y < size-1)
        {
            yb = data[x][y+1].reCur;
            if (y + 1 < size - 1)
            {
                ybb = data[x][y+2].reCur;
                if (y + 2 < size - 1)
                    ybbb = data[x][y+3].reCur;
            }
        }
    }

    laplacianApprox = ((xlll + xrrr + yttt + ybbb)/90.0 -3.0*(xll + xrr + ytt + ybb)/20.0 + 3.0*(xl + xr + yt + yb)/2.0 - 49.0*centerVal/9.0)/(dR*dR);

    if (mode == R)
        return valBefore + dT*(-laplacianApprox/2.0 + data[x][y].V*centerVal);
    if (mode == I)
        return valBefore - dT*(-laplacianApprox/2.0 + data[x][y].V*centerVal);

    return 0.0;
}



template<typename T> void fftImage(const QVector<QVector<T>>& in, QVector<QVector<T>>& out)
{

}


// recursively performs fft, but is restricted to go along
// 1) x direction, 2) either u or v
// in other words, this helper performs a 1-D fft along specified axes
// returns the fft of the elements of in, specified by 'start' and 'stepSize'
template<typename T> QVector<T> fftHelper(const QVector<T>& in, int start, int stepSize, char mode)
{
    cdouble i = sqrt(cdouble(-1));

    // base case
    if (start + stepSize >= in.size())  // in other words, there is only 1 element
        return {in[start]};

    // get the even and odd sequences that help constitute all elements of this one
    // assuming that we are using a power of 2 : that would help a lot
    QVector<T> evenCoeffs = fftHelper(in, start, stepSize*2, mode);
    QVector<T> oddCoeffs = fftHelper(in, start + stepSize, stepSize*2, mode);
    QVector<T> ret(evenCoeffs.size() + oddCoeffs.size());
    double period = in.size();

    // TESTING
    if (evenCoeffs.size() != oddCoeffs.size() || ret.size()*stepSize != in.size())
        cout << "something went wrong ..." << endl;

    // compute each element based on evenCoeffs and oddCoeffs
    for (int k = 0; k < evenCoeffs.size(); ++k)
    {
        T factor = exp(-i*cdouble(2.0*PI*double(k*stepSize)/period));
        ret[k] = evenCoeffs[k] + oddCoeffs[k]*factor;
        ret[k + evenCoeffs.size()] = evenCoeffs[k] - oddCoeffs[k]*factor;
    }

    return ret;
}

// instantiated templates, ready for use : cdouble is just complex<double>
template QVector<cdouble> fftHelper<cdouble>(const QVector<cdouble>& in, int start, int stepSize, char mode);
//template QVector<cdouble> fftHelper<cdouble>(const QVector<cdouble>& in, int start, int stepSize, char mode);


double interpolateBicubic(double ll, double l, double r, double rr, double d)
{
    return (0.5*(rr-ll)+1.5*(l-r))*d*d*d + (ll-2.5*l+2*r-0.5*rr)*d*d + 0.5*(r-ll)*d +l;
}


double interpolateBicubic2D(const QVector<QVector<double> > &values, double xDist, double yDist)
{
    double x0 = interpolateBicubic(values[0][0], values[1][0], values[2][0], values[3][0], xDist);
    double x1 = interpolateBicubic(values[0][1], values[1][1], values[2][1], values[3][1], xDist);
    double x2 = interpolateBicubic(values[0][2], values[1][2], values[2][2], values[3][2], xDist);
    double x3 = interpolateBicubic(values[0][3], values[1][3], values[2][3], values[3][3], xDist);
    return interpolateBicubic(x0, x1, x2, x3, yDist);
}

//-------------------------------------------------------------------------------------------
// CONVERSIONS
//-------------------------------------------------------------------------------------------


// conversion from screen coordinates to world coordinates:
// world coordinates range from -SIDE_LENGTH/2 to SIDE_LENGTH/2
// screen coordinates range from -1.0 to 1.0
// these are not by default normalized
double GLToWorld(double screenCoord)
{
    return (screenCoord)*SIDE_LENGTH/2.0;
}

QVector3D GLToWorld(const QVector3D &glCoord)
{
    return {float(GLToWorld(glCoord[0])), float(GLToWorld(glCoord[1])), float(GLToWorld(glCoord[2]))};
}

// as of now this function is not used
double worldToGL(double worldCoord)
{
    return (worldCoord)/(SIDE_LENGTH/2.0);
}

QVector3D worldToGL(const QVector3D &worldCoord)
{
    return { float(worldToGL(worldCoord[0])), float(worldToGL(worldCoord[1])), float(worldToGL(worldCoord[2]))};
}

// conversion from a x, y index to world coordinates
double indexToWorld(int index, double dR)
{
    double delta = index*dR;
    return delta - SIDE_LENGTH/2.0;
}

// used for converting a z-coordinate to an appropriate potential value
// z-values range from -1 to 1
// potentials range from -MAX_POTENTIAL to MAX_POTENTIAL
double indexToPotential(int z)
{
    return z*MAX_POTENTIAL;
}

// maps from [-MAX_POTENTIAL , MAX_POTENTIAL] to [-0.5, 0.5]
double potentialToGL(double potential)
{
    return double(potential)/(2.0*MAX_POTENTIAL) + 0.001 ;
}

double potentialToWorld(double potential)
{
    return GLToWorld(potentialToGL(potential));
}

double worldToPotential(double world)
{
    return GLToPotential(worldToGL(world));
}

double GLToPotential(double gl)
{
    return double((gl - 0.001)*2.0*MAX_POTENTIAL);
}

double indexToGL(int index, int samplesPerSide)
{
    return double(index)*(2.0/(double(samplesPerSide)-1.0)) - 1.0;
}

QVector2D scrnToGL(int xScrn, int yScrn, int w, int h)
{
    double normalizedX = 2.0f * double(xScrn)/ double(w) - 1.0;
    double normalizedY = 1.0 - 2.0f * double(yScrn)/ double(h);
    return QVector2D(normalizedX, normalizedY);
}

double stretchToSpeed(double stretch)
{
    // can stretch as far as 0.25*SIDE_LENGTH
    return stretch*MAX_SPEED/(0.25*SIDE_LENGTH);
}


//-------------------------------------------------------------------------------------------
// COLORS
//-------------------------------------------------------------------------------------------

// load more complex colorplots from file
ColorMapper::ColorMapper()
{
    // load inferno, there are 256 datapoints for each of r,g,b for an argument b/w 0 - 1
    infernoPath = ":/other/cmaps/inferno_cmap";
    QFile infernoTxt(infernoPath.c_str());
    infernoTxt.open(QFile::ReadOnly);

    QTextStream ts(&infernoTxt);
    double temp;

    // load red
    for (int i = 0; i < 256; ++i)
    {
        Color c;
        ts >> c.r;
        inferno.insert(make_pair(i, c));
    }

    // load green
    for (int i = 0; i < 256; ++i)
    {
        ts >> temp;
        unordered_map<int,Color>::iterator it = inferno.find(i);
        it->second.g = temp;
    }

    // load blue
    for (int i = 0; i < 256; ++i)
    {
        ts >> temp;
        unordered_map<int,Color>::iterator it = inferno.find(i);
        it->second.b = temp;
    }
}


// Code written here is based off of matplotlib colorplots
// these ones are particularly easy to handcode, as they are piece-wise linear
// others that are not so will be loaded from a file

/*
 Return a RGB colour value given a scalar v in the range [vmin,vmax]
 In this case each colour component ranges from 0 (no contribution) to
 1 (fully saturated), modifications for other ranges is trivial.
 The colour is clipped at the end of the scales if v is outside
 the range [vmin,vmax]
 */

// arg is between 0 and 1; this function computes an appropriate color (in rgb-space) given arg
void ColorMapper::computeColor(double arg, Color& color, double min, double max, char cmap)
{
    if (arg > max)
        arg = max;
    else if (arg < min)
        arg = min;

    if (cmap == 'j')
        getColorJet(arg, min, max, color);
    else if (cmap == 'h')
        getColorHot(arg, min, max, color);
    else if (cmap == 'c')
        getColorCool(arg, min, max, color);
    else if (cmap == 's')
        getColorSpring(arg, min, max, color);
    else if (cmap == 'g')
        getColorGray(arg, min, max, color);
    else if (cmap == 'a')
        getColorAutumn(arg, min, max, color);
    else if (cmap == 'w')
        getColorWinter(arg, min, max, color);
    else if (cmap == 'i')
        getColorInferno(arg, min, max, color);
}


void ColorMapper::getLinearInterpolated(double normalizedArg, const Color &colorFrom, const Color &colorTo, Color &out)
{
    out.r = colorFrom.r*(1.0 - normalizedArg) + colorTo.r*normalizedArg;
    out.g = colorFrom.g*(1.0 - normalizedArg) + colorTo.g*normalizedArg;
    out.b = colorFrom.b*(1.0 - normalizedArg) + colorTo.b*normalizedArg;
}


void ColorMapper::getColorInferno(double v, double vmin, double vmax, Color &color)
{
    double vMapped = (v - vmin)/(vmax - vmin);
    int vRounded = rint(vMapped*255.0);
    color = inferno.find(vRounded)->second;
}


void ColorMapper::getColorJet(double v,double vmin,double vmax, Color& c)
{
    if (v < vmin)
        v = vmin;
    if (v > vmax)
        v = vmax;

    double vScaled = (v- vmin)/(vmax - vmin);

    // up to first stop point: 11.25%
    if (vScaled < 0.1125)
    {
        c.r = 0;
        c.g = 0;
        c.b = 4 * vScaled + 0.55;
    }

    // up to second stop point: 36.25%
    else if (vScaled < 0.3625)
    {
        c.r = 0;
        c.g = 4 * (vScaled - 0.25) + 0.55;
        c.b = 1;
    }

    // up to third stop point: 61.25%
    else if (vScaled < 0.6125)
    {
        c.r  = 4 * (vScaled - 0.5) + 0.55;
        c.g = 1;
        c.b = -4 * (vScaled - 0.75) - 0.55;
    }

    // up to last stop point: 86.25%
    else if (vScaled < 0.8625)
    {
        c.r = 1;
        c.g = -4 * (vScaled - 1) - 0.55;
        c.b = 0;
    }

    // to the end
    else
    {
        c.r = -4 * (vScaled - 1.25) - 0.55;
        c.g = 0;
        c.b = 0;
    }
}

// our default: afm-hot from matplotlib
void ColorMapper::getColorHot(double v, double vmin, double vmax, Color &color)
{
    // this one is easier to do, separately by rgb
    // scale v to a value from 0 to 1 to make it harder to screw up
    double vScaled = (v - vmin)/(vmax - vmin) ;

    // red
    if (vScaled < 0.5)
        color.r = 2.0*vScaled;
    else
        color.r = 1.0;

    // green
    if (vScaled < 0.25)
        color.g = 0;
    else if (vScaled > 0.75)
        color.g = 1.0;
    else
        color.g = 2.0*(vScaled - 0.25);

    // blue
    if (vScaled < 0.5)
        color.b = 0.0;
    else
        color.b = 2.0*(vScaled - 0.5);
}


// more linear (more consistent when openGL is forced to interpolate for us)
void ColorMapper::getColorWinter(double v, double vmin, double vmax, Color &color)
{
    double vScaled = (v - vmin)/(vmax - vmin);

    color.r = 0;
    color.g = vScaled;
    color.b = -0.5*vScaled + 1.0;
}

void ColorMapper::getColorCool(double v, double vmin, double vmax, Color &color)
{
    double vScaled = (v - vmin)/(vmax - vmin);

    color.r = vScaled;
    color.g = -vScaled + 1.0;
    color.b = 1.0;
}

void ColorMapper::getColorSpring(double v, double vmin, double vmax, Color &color)
{
    double vScaled = (v - vmin)/(vmax - vmin);

    color.r = 1.0;
    color.g = vScaled;
    color.b = -vScaled + 1.0;
}

void ColorMapper::getColorAutumn(double v, double vmin, double vmax, Color &color)
{
    double vScaled = (v - vmin)/(vmax - vmin);

    color.r = 1.0;
    color.b = 0;
    color.g = vScaled;
}

void ColorMapper::getColorGray(double v, double vmin, double vmax, Color &color)
{
    double vScaled = (v - vmin)/(vmax - vmin);

    color.r = vScaled;
    color.b = vScaled;
    color.g = vScaled;
}


// gray color
double ColorMapper::potentialToColor(double height)
{
    return 0.7*potentialToGL(height) + 0.6;
}


//-------------------------------------------------------------------------------------------
// MISC
//-------------------------------------------------------------------------------------------


double roundToPrecision(double arg, int precision)
{
    double factor = pow(10.0, precision);
    arg *= factor;
    arg = rint(arg);
    return arg/factor;
}


// currently not used
template<typename T> int binarySearch(const vector<T>& info, T target)
{
    int leftPiv = 0, rightPiv = info.size()-1;

    while(leftPiv < rightPiv - 1)
    {
        int mid = (leftPiv + rightPiv)/2;
        T val = info[mid];

        if (val == target)
            return mid;
        if (val > target)
        {
            rightPiv = mid;
        }
        else
        {
            leftPiv = mid;
        }
    }

    return rightPiv;   // the first element that is greater than target
}

template int binarySearch<double>(const vector<double>&, double target);

double clipPotential(double potential)
{
    if (potential > MAX_POTENTIAL)
        potential = MAX_POTENTIAL;
    else if (potential < -MAX_POTENTIAL)
        potential = -MAX_POTENTIAL;
    return potential;
}

// one of those either use while loop or recursion problems
int numVerticesAfterScaling(int numOriginal, int scaling)
{
    if (scaling <= 1)
        return numOriginal;

    return pow(2.0*sqrt(numVerticesAfterScaling(numOriginal, scaling - 1)) - 1.0, 2.0);
}


QVector<QVector3D> getTriangle(const QVector3D& centerVal, double rotationDeg, double forwardStretch, double latStretch)
{
    // extend the centerVal along given direction
    QVector3D stepForward(centerVal[0] + cos(DEG_TO_RAD*rotationDeg), centerVal[1] + sin(DEG_TO_RAD*rotationDeg), 0.0);
    QVector3D direction = (stepForward - centerVal).normalized();
    QVector3D lateral = QVector3D::crossProduct(QVector3D(0, 0, 1.0), direction);

    forwardStretch /= 2.0;
    stepForward = centerVal + forwardStretch*direction;
    QVector3D sideOne = centerVal + lateral*latStretch - forwardStretch*direction;
    QVector3D sideTwo = centerVal - lateral*latStretch - forwardStretch*direction;
    return {stepForward, sideOne, sideTwo};
}


QVector<QVector3D> getCircle(const QVector3D& centerVal, double radius, int n, const QVector3D& axis)
{
    QVector<QVector3D> vertices(n);
    getCircle(centerVal, radius, n, vertices, axis);
    return vertices;
}


void getCircle(const QVector3D &centerVal, double radius, int n, QVector<QVector3D> &out, const QVector3D& axis)
{
    QVector3D unitAxis = axis.normalized();
    double normTheta = sqrt(unitAxis.x()*unitAxis.x() + unitAxis.y()*unitAxis.y());

    double dTheta = DEG_TO_RAD*(360.0/n);
    for (int i = 0; i < n; ++i)
    {
        double angle = i*dTheta;
        double x,y,z;

        // the corner case
        if (normTheta == 0)
        {
            x = radius*cos(angle) + centerVal.x();
            y = radius*sin(angle) + centerVal.y();
            z = 0.0;
        }
        else
        {
            x = radius*(-unitAxis.y()*cos(angle)/normTheta - unitAxis.z()*unitAxis.x()*sin(angle)/normTheta) + centerVal.x();
            y = radius*(unitAxis.x()*cos(angle)/normTheta - unitAxis.z()*unitAxis.y()*sin(angle)/normTheta) + centerVal.y();
            z = radius*normTheta*sin(angle);
        }

        out[i].setX(x);
        out[i].setY(y);
        out[i].setZ(z);
    }
}

