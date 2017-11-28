#ifndef TUTORIAL_H
#define TUTORIAL_H

#include <QVector2D>
#include <QTextBrowser>
#include <QPoint>
#include <QVector>
#include <QString>
#include <QtConcurrent>
#include <QFuture>
#include "glwidget.h"

/*
 * This file contains:
 * class Tutorial, which has states, info, functions, necessary to run a tutorial
*/

class Tutorial
{
public:
    Tutorial() {;}
    Tutorial(GLWidget* simulation, QWidget* parent);

    // it is dangerous to call this routine before simulation has its own dimensions set up,
    // hence why this is not called in the constructor
    void setDimensions();
    QVector<list<QString>>* getHelpText() { return &helpText;}
    int getCurState() { return curState;}
    bool userCannotAdvance() { return lockAdvance;}
    void transition(bool forward);

private:

    // -1: out of commission, nothing is shown
    // 0: classic sandbox mode, flat surface
    // 1: load a thin potential barrier, manually set things, and throw wavepacket at it
    // 2: paused, user can only play with green net
    // 3: make comments on apparent differences between quantum and classical
    int curState = -1, prevState = -1;
    int width, height, topleftX;

    // reserved for when we are playing animations and don't want the user to tamper with us
    bool lockAdvance = false;

    // the only component that is actually shown :
    // here a scheme similar to double buffer swapping is used
    // (mainly to circumvent Qt issues with multithreading QTextBrowser)
    bool tutText1Active = true;
    QTextBrowser* tutText1, *tutText2;

    // other than geometry info, we need some control over its behaviour, e.g locking certain buttons
    GLWidget* simulation;
    QWidget* parent;

    // each element of helpText is a list, corresponding to the curState of the tutorial
    QVector<list<QString>> helpText;
    list<QString>::iterator curTextp;

    void animateTransition();
    void readyText();
    void initHelpText();
};

#endif // TUTORIAL_H
