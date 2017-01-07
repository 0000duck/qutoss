#ifndef WINDOW_H
#define WINDOW_H

#include <QDesktopWidget>
#include <QWidget>
#include <QSlider>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QTabWidget>
#include <QTextBrowser>
#include <QLinearGradient>
#include <QBrush>
#include <QFile>
#include <QLabel>
#include <QSizePolicy>
#include <QTextCursor>
#include <QColor>
#include <QDesktopServices>
#include <QUrl>
#include <QCheckBox>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "glwidget.h"
#include "widgetaddons.h"
#include "tutorial.h"


using namespace std;


class Window : public QWidget //, public QLayout
{
    Q_OBJECT
public:
    explicit Window(QWidget *parent = 0);

signals:


public slots:

    void handleFirstRun();  // on first run of the program, automatically queue tutorial

    // a request to advance the tutorial forward
    // takes signal emitted by GlWidget when a tutorial stage is complete and needs automatic dialogue advance,
    void advanceTutDialogue(bool fromSimulation);

    // useful only if we are showing options from the lineedit: this feature is currently not present
    void lineReceivedFocus();

    // link manager
    void linkClicked(const QUrl &link);

    // control of buttons (possibly disabled) as GUI changes
    void tabChanged(unsigned tabNum);

    // handlers for button clicks
    void switchToTiles();
    void switchToBrush();
    void pause();
    void resume();

    // multiple signals may be connected to this slot (our universal message displayer, if you will)
    void displayError(string message);

    // spinbox only:
    // level of indirection required as spinBox's editingDone signal carries no information
    // alternative to this mess is to subclass QSpinBox ... eh
    void setXCen();
    void setYCen();
    void setPacketSpeed();
    void setPacketAngle();
    void setPacketPrecision();

    // echo calls, to display changes back onto widget
    void packetCenterChanged(QVector3D center);
    void packetSpeedChanged(double speed);
    void packetAngleChanged(double angle);
    void packetPrecisionChanged(double precision);

    // change text to match the tabs
    void simulationStarted();  // first time pressing P
    void resetOccurred();      // simulation is reset
    void updateInfo();  // probability, energy, any useful information on the system...

private:
    int previousArea = 0;
    int targetHeight;  // the height of the window
    double scale;    // calculated based on targetHeight/800.0
    double initialWidth, initialHeight;   // measured with dimensions of sideText
    int currentTab = 0;
    bool initialResize = true;

    QVBoxLayout* tabOneLayout;
    QVBoxLayout* tabTwoLayout;
    QVBoxLayout* simulationTabLayout;
    QHBoxLayout* hlayout;

    QVector<QPushButton*> buttons;  // make it easier to dynamically modify all of them
    QColor grayBlue = QColor(39,83,122);
    QTimer alarmClock;  // rings every once in a while, uploads current probability value
    QSlider* simSpeedSlider, *sensitivitySlider, *brushSizeSlider, *brushHeightSlider;
    GLWidget* simulation;
    LineEdit* editableLine, *probability, *simulationTime;
    QTreeWidget* choicesPopup;    // top-level popup
    QPushButton* playButton, *restartButton, *switchToTilesButton;
    QPushButton* switchToBrushButton, *observeButton, *pauseButton, *classicalButton;
    QPushButton* observeFiller, *restartFiller;   // these buttons don't do anything

    // a toggle may be achieved by either pressing the button or moving to a tab that disables it
    QStackedWidget* placementTogglers, *pauseTogglers, *observeTogglers, *restartTogglers;
    QDoubleSpinBox *speed, *angle, *xCen, *yCen, *precision;
    QTabWidget* tabs, *simTabs;
    QWidget* tabOne, *tabTwo, *tabThree; // displayed only in setup stage
    QWidget* simulationTab; // simulationTab is displayed only after simulation has started
    QWidget* buttonPanel;
    QTextBrowser* sideText, *tutText;
    QWidget* sideTextContainer;
    QTimer startTutorialTimer;  // will call handlFirstRun upon timeout

    // labelled versions (after pairing original widgets with a QLabel)
    // I do not approve of the -Widget suffix, but also at lost for a better word ...
    // -Labelled perhaps?
    QWidget* speedWidget, *angleWidget, *xCenWidget, *yCenWidget, *precisionWidget;
    QWidget* simSpeedSliderWidget, *sensitivitySliderWidget, *placementTogglersWidget, *simulationTimeWidget;
    QWidget* brushSizeSliderWidget, *brushHeightSliderWidget, *probabilityWidget, *showClassicalWidget;
    QHBoxLayout* buttonPanelLayout;
    QVBoxLayout* sideBarLayout;
    QStackedLayout* sideBarTopLayout;   // tabs and simulation panel
    QString projectOverview;
    QString tabOneHelp, tabTwoHelp, tabThreeHelp, simulationHelp, generalControls;
    QString speedTip, angleTip, centerTip, precisionTip, equationTip, brushSizeTip, brushHeightTip;
    QString timeTip, probabilityTip, speedupTip, sensitivityTip;
    QString observeTip, classicalTip, restartTip, fillerTip, continuousTip, discreteTip;
    QString styleSheet;
    QString tabOneText, tabTwoText, simulationTabText, credits, accuracy, instructions;
    QVector<QString> choices;
    QVector<QString*> allHelpText;

    // parameters/keywords to replace in css code at run-time
    int ttFontSizeVal, sFontSizeVal, msFontSizeVal, mFontSizeVal, lFontSizeVal, xlFontSizeVal;
    int ttTableWidthVal, tableWidthVal, sliderHeightVal, borderRadiusVal, handleHeightVal, halfHandleHeightVal, scrollbarWidthVal;
    QString ttFontSize, sFontSize, msFontSize, mFontSize, lFontSize, xlFontSize;
    QString ttTableWidth, tableWidth, sliderHeight, borderRadius, handleHeight, halfHandleHeight, scrollbarWidth;
    Tutorial tutorial;

    // functions
    void setObjectNames();   // to help link things with stylesheet
    void applyExpandingTabsStyle();  // applies to member 'tabs'
    void resizeEvent(QResizeEvent* event)  Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void rescaleFonts(double scaling);
    void rescaleButtons(double scaling);
    void clearLayout(QLayout* layout);
    void createSpinBoxes();
    void createButtons();
    void createSimSpeedSlider();
    void createSensitivitySlider();
    void createBrushSizeSlider();
    void createBrushHeightSlider();
    void createEditableLine();
    void createInfoAndTimeLine();
    //void createChoicesPopup();
    void createTabs();

    // returns a widget containing current widget + label on its left
    QWidget* addLabel(QWidget* noName, string name, QString toolTip = "None");

    void initFonts();   // dependent on screen size
    void initStrings();

    QDoubleSpinBox* createSpinBox(double min, double max, double initial = 0);

    void createTextBox();
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void setEquation() { simulation->setEquation(editableLine->text());}
    //void setShadow(QWidget* target);
};


#endif // WINDOW_H
