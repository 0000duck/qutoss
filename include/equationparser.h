#ifndef EQUATIONPARSER_H
#define EQUATIONPARSER_H

#include <QPoint>
#include <QVector>
#include <QString>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>
#include "ae/ae.h"
#include "helpers.h"

using namespace std;

/* This file:
 * - takes raw string input from the user and alters it s.t ae library can further read it
 *   - i.e xy => x*y, sinx => sin(x), e^x => exp(x)
 * - interprets error output from ae (if any) and returns a more easily readable version
 * - most of the core work is done by the ae library / lua interpreter
*/

class EquationParser
{
public:
    EquationParser();
    ~EquationParser();
    float evaluateEquation(float x, float y);
    void setEquation(string s) { standardized = standardize(s);  data = standardized.c_str(); }
    string errorCheck(const QVector<QVector<Point>>& gridData);

private:
    string standardize(string raw);
    string standardized;
    const char* data;  // makes evaluation slightly faster, if needed
    string original;  // original, raw input
};

#endif // EQUATIONPARSER_H
