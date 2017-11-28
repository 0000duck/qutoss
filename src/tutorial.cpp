#include "tutorial.h"

Tutorial::Tutorial(GLWidget* _simulation, QWidget* _parent)
{
    simulation = _simulation;
    parent = _parent;
    tutText1 = new QTextBrowser(parent);
    tutText2 = new QTextBrowser(parent);
    tutText1->setObjectName("tutText1");
    tutText2->setObjectName("tutText2");
    initHelpText();
}


void Tutorial::setDimensions()
{
    QRect model = simulation->geometry();
    height = model.height()*0.17;
    width = model.width()*0.7;
    topleftX = model.center().x() - width/2.0;
    tutText1->setGeometry(topleftX, -height, width, height);
    tutText2->setGeometry(topleftX, -height, width, height);
}


void Tutorial::readyText()
{
    if (curState != -1)
    {
        if (tutText1Active)
        {
            tutText2->setText(*curTextp);
        }
        else
        {
            tutText1->setText(*curTextp);
        }
    }
}


void Tutorial::transition(bool forward)
{
    prevState = curState;
    if (forward)
    {
        if (curState == -1)
        {
            ++curState;
            curTextp = helpText[curState].begin();
        }
        else
        {
            ++curTextp;
            if (curTextp == helpText[curState].end())
            {
                if (curState < helpText.size()-1)
                    curTextp = helpText[++curState].begin();
                else    // end tutorial
                    curState = -1;
            }
        }
    }
    else
    {
        if (curState != -1)
        {
            if (curTextp == helpText[curState].begin())
            {
                if (curState != 0)
                    curTextp = helpText[--curState].begin();
            }
            --curTextp;
        }
    }

    readyText();

    // issue the appropriate commands to GlWidget, to lock down certain buttons
    if (prevState != curState)
        simulation->readyTutStage(curState);

    if ((curState == 1 || curState == 2) && curTextp == helpText[curState].begin())
        lockAdvance = true;
    else
        lockAdvance = false;

    // run the animation in a different thread
    QFuture<void> supervisor = QtConcurrent::run(this, &Tutorial::animateTransition);
}


// animates the swap between tutText1 and tutText2
void Tutorial::animateTransition()
{
    if (prevState != -1)
    {
        if (tutText1Active)
            slideWidget(tutText1, QPoint(topleftX, -1.1*height), 6.0);
        else
            slideWidget(tutText2, QPoint(topleftX, -1.1*height), 6.0);
    }

    if (curState != -1)
    {
        if (tutText1Active)
            slideWidget(tutText2, QPoint(topleftX, -0.1*height), 2.0);
        else
            slideWidget(tutText1, QPoint(topleftX, -0.1*height), 2.0);
    }

    tutText1Active = !tutText1Active;
}


void Tutorial::initHelpText()
{
    helpText.resize(4);

    // each element of helpText corresponds to a certain state
    helpText[0].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Welcome to the tutorial! Here we have a sandbox simulator of a particle trapped by walls. "
                          "<br><br><i> Press M for next dialogue </i>"
                          "</span>");

    helpText[0].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Use WASD keys to swing around, Z and X to zoom in and out, P to go to 'perch'."
                          "<br><br><i> Press M for next dialogue </i>"
                          "</span>");

    helpText[0].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Using the 'Potential' tab, shape the environment. Intuitively, higher regions 'repel'"
                          " and lower ones 'attract'."
                          "<br><br><i> Press M for next dialogue </i>"
                          "</span>");

    helpText[0].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Using the 'Initial' tab, set the particle's initial velocity and position."
                          " Enter values, <b>or</b> click the particle/green arrow."
                          "<br><br><i> Press M for next dialogue </i>"
                          "</span>");

    helpText[0].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Continue experimenting until the grey tiles (potential energy) make sense to you. "
                          "<br><br><i> Press M to proceed </i>"
                          "</span>");

    helpText[1].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Here we fire the particle towards a wall that's too high"
                          " to pass through. <br> What happens next should be no surprise to you."
                          "</span>");

    helpText[1].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "As a matter of fact, it is <b>impossible</b> for a <b>classical</b> particle at this speed"
                          " to go past a potential barrier of this height. "
                          "<br><br><i> Press M to proceed </i>"
                          "</span>");

    helpText[2].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "This time we use a wavepacket to"
                          " represent our particle.<br>The height/brightness indicate the <b>probability "
                          "of observing</b> the particle at that position."
                          "</span>");

    helpText[2].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Evidently, <b>quantum tunneling</b> has occurred. "
                          "It's possible that we find the particle on the other side."
                          "<br><br><i> Press M to proceed </i>"
                          "</span>");

    helpText[3].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Investigate by clicking on-screen to set the green net."
                          " The probability of finding the particle within is stated on the left."
                          "<br><br><i> Press M for next dialogue </i>"
                          "</span>");

    helpText[3].push_back("<FONT COLOR = '#bbbbbb'>"
                          "<br>"
                          "<span style = \"font-size: lFontpx;\">"
                          "Continue exploring both classical and quantum"
                          " behaviours. Your intuition will be refreshed and challenged."
                          "<br><br><i> Press M to end tutorial </i>"
                          "</span>");
}
