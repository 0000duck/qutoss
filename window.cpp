#include "window.h"

// note that subclassing from QT classes means that we acquire
// automatic dynamic memory cleanup: any pointers added to this object's
// layout (whose owner is automatically 'this') are handled by QT
Window::Window(QWidget *parent) : QWidget(parent)
{
    QRect available = QApplication::desktop()->availableGeometry();
    targetHeight = min(0.97*available.width(), 0.97*available.height());
    scale = targetHeight/800.0;
    choices.resize(20);

    // the order of initialization is important here
    simulation = new GLWidget;
    initStrings();
    createSimSpeedSlider();
    createSensitivitySlider();
    createEditableLine();
    createInfoAndTimeLine();       // for displaying values while the simulation is running
    createButtons();
    createSpinBoxes();
    createTextBox();
    createBrushSizeSlider();
    createBrushHeightSlider();

    // updates the info slot upon timeout
    alarmClock.start(100);
    connect(&alarmClock, SIGNAL(timeout()), this, SLOT(updateInfo()));
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(updateInfo()));

    // add labels to these fields
    speedWidget = addLabel(speed, "speed", speedTip);
    angleWidget = addLabel(angle, "angle", angleTip);
    xCenWidget = addLabel(xCen, "center (x)", centerTip);
    yCenWidget = addLabel(yCen, "center (y)", centerTip);
    precisionWidget = addLabel(precision, "precision", precisionTip);
    simSpeedSliderWidget = addLabel(simSpeedSlider, "speedup", speedupTip);
    sensitivitySliderWidget = addLabel(sensitivitySlider, "sensitivity", sensitivityTip);
    probabilityWidget = addLabel(probability, "probability", probabilityTip);
    brushSizeSliderWidget = addLabel(brushSizeSlider, "brush size", brushSizeTip);
    brushHeightSliderWidget = addLabel(brushHeightSlider, "brush height", brushHeightTip);
    placementTogglersWidget = addLabel(placementTogglers, "toggle mode");
    simulationTimeWidget = addLabel(simulationTime, "remaining", timeTip);

    hlayout = new QHBoxLayout;
    sideBarLayout = new QVBoxLayout;
    sideBarTopLayout = new QStackedLayout;

    createTabs();

    // a stacked layout, so we can switch between the top whenever necessary
    sideBarTopLayout->addWidget(tabs);
    sideBarTopLayout->addWidget(simTabs);

    buttonPanelLayout = new QHBoxLayout;
    buttonPanelLayout->addWidget(restartTogglers);
    buttonPanelLayout->addWidget(observeTogglers);
    buttonPanelLayout->addWidget(classicalButton);
    buttonPanelLayout->addWidget(pauseTogglers);
    buttonPanelLayout->setContentsMargins(0,0,0,0);
    buttonPanelLayout->addStretch(1);
    buttonPanelLayout->setSpacing(1);

    buttonPanel = new QWidget;
    buttonPanel->setLayout(buttonPanelLayout);
    buttonPanel->setContentsMargins(0,0,0,0);

    // this block of code is just to shift the qTextEdit over (add margins on its left, and right)
    sideTextContainer = new QWidget;
    QHBoxLayout* sideTextLayout = new QHBoxLayout;
    sideTextLayout->addWidget(sideText);
    sideTextLayout->setContentsMargins(5*scale, 0, 2*scale, 0);
    sideTextContainer->setLayout(sideTextLayout);

    // assemble sideBar
    sideBarLayout->addLayout(sideBarTopLayout, 5);
    sideBarLayout->addWidget(buttonPanel);
    sideBarLayout->addWidget(sideTextContainer, 6);
    sideBarLayout->setContentsMargins(0,0,0,0);
    sideBarLayout->setSpacing(0);

    QWidget* sideBar = new QWidget;
    sideBar->setLayout(sideBarLayout);
    sideBar->setContentsMargins(0,0,0,0);

    // final layout, spacings
    hlayout->addWidget(sideBar);
    hlayout->addWidget(simulation);
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->setSpacing(0);

    // make it as large as possible
    simulation->setFixedHeight(targetHeight);
    simulation->setFixedWidth(targetHeight);

    // to guarantee even spacing between tabs, must use even number :
    // on older OSes, this is pretty important, as tabs will run out of space and invoke ugly things
    int width = 280*scale;
    width = (width/2)*2;
    sideBar->setFixedWidth(width);

    setLayout(hlayout);

    // all the other fonts were initialized at the beginning, but
    // tutorial must be initialized after layouts have all been set
    tutorial = Tutorial(simulation, this);
    QVector<list<QString>>* helpText = tutorial.getHelpText();
    for (int i = 0; i < helpText->size(); ++i)
    {
        for (list<QString>::iterator it = (*helpText)[i].begin(); it != (*helpText)[i].end(); ++it)
            allHelpText.push_back(&(*it));
    }
    initFonts();

    // connect widgets:
    // technically either one of the syntaxes below work, but for some reason the latter gives some issues...
    connect(xCen, SIGNAL(editingFinished()), this, SLOT(setXCen()));  // don't feel like subclassing just to implement a proper signal...
    connect(yCen, SIGNAL(editingFinished()), this, SLOT(setYCen()));
    connect(speed, SIGNAL(editingFinished()), this, SLOT(setPacketSpeed()));
    connect(angle, SIGNAL(editingFinished()), this, SLOT(setPacketAngle()));
    connect(precision, SIGNAL(editingFinished()), this, SLOT(setPacketPrecision()));

    // echoing calls to reflect changes
    connect(simulation, SIGNAL(updatedCenter(QVector3D)), this, SLOT(packetCenterChanged(QVector3D)));
    connect(simulation, SIGNAL(updatedSpeed(double)), this, SLOT(packetSpeedChanged(double)));
    connect(simulation, SIGNAL(updatedAngle(double)), this, SLOT(packetAngleChanged(double)));
    connect(simulation, SIGNAL(updatedPrecision(double)), this, SLOT(packetPrecisionChanged(double)));
    connect(simulation, SIGNAL(observingWavefunction()), this, SLOT(pause()));

    // buttons
    connect(switchToBrushButton, &QPushButton::released, this, &Window::switchToBrush);
    connect(switchToTilesButton, &QPushButton::released, this, &Window::switchToTiles);
    connect(observeButton, &QPushButton::released, simulation, &GLWidget::observe);
    connect(restartButton, &QPushButton::released, simulation, &GLWidget::resetSimulation);
    connect(restartButton, &QPushButton::released, this, &Window::pause);
    connect(playButton, &QPushButton::clicked, this, &Window::resume);
    connect(pauseButton, &QPushButton::clicked, this, &Window::pause);
    connect(classicalButton, &QPushButton::clicked, simulation, &GLWidget::toggleDrawClassical);

    // sliders, editableLine, tabs, simulation
    connect(brushSizeSlider, &QSlider::valueChanged, simulation, &GLWidget::setBrushSize);
    connect(brushHeightSlider, &QSlider::valueChanged, simulation, &GLWidget::setBrushHeight);
    connect(simSpeedSlider, &QSlider::valueChanged, simulation, &GLWidget::setSimSpeed);
    connect(sensitivitySlider, &QSlider::valueChanged, simulation, &GLWidget::setSensitivity);
    connect(editableLine, &LineEdit::returnPressed, this, &Window::setEquation);
    connect(simulation, &GLWidget::error, this, &Window::displayError);
    connect(tabs, &QTabWidget::currentChanged, simulation, &GLWidget::setHoverState);
    connect(tabs, &QTabWidget::currentChanged, this, &Window::tabChanged);
    connect(simulation, &GLWidget::startingSimulation, this, &Window::simulationStarted);
    connect(simulation, &GLWidget::resetOccurred, this, &Window::resetOccurred);
    connect(simulation, &GLWidget::pausingSimulation, this, &Window::pause);

    // GUI responsiveness
    connect(brushSizeSlider, &QSlider::valueChanged, this, &Window::switchToBrush);
    connect(brushHeightSlider, &QSlider::valueChanged, this, &Window::switchToBrush);
    connect(tabs, &QTabWidget::currentChanged, this, &Window::applyExpandingTabsStyle);
    connect(restartButton, &QPushButton::released, this, &Window::applyExpandingTabsStyle);
    connect(sideText, &QTextBrowser::anchorClicked, this, &Window::linkClicked);

    // tutorial stuff
    connect(simulation, &GLWidget::classicalSimComplete, this, &Window::advanceTutDialogue);
    connect(simulation, &GLWidget::quantumSimComplete, this, &Window::advanceTutDialogue);

    //connect(editableLine, &LineEdit::receivedFocus, this, &Window::lineReceivedFocus);
    //createChoicesPopup();
    simulation->setFocus();
    setObjectNames();
    setStyleSheet(styleSheet);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    // if its the first time running this program, the tutorial will start playing in a bit
    startTutorialTimer.setSingleShot(true);
    startTutorialTimer.start(2000);
    connect(&startTutorialTimer, SIGNAL(timeout()), this, SLOT(handleFirstRun()));
}



//----------------------------------------------------
// SLOTS
//----------------------------------------------------


void Window::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_M)
    {
        advanceTutDialogue(false);
    }

    simulation->keyReleaseEvent(event);
}


void Window::advanceTutDialogue(bool fromSimulation)
{
    if (fromSimulation)
    {
        tutorial.transition(true);
    }
    else
    {
        if (!(tutorial.userCannotAdvance()))
            tutorial.transition(true);
    }
}


void Window::lineReceivedFocus()
{
    int left, right, bot, top;
    editableLine->getContentsMargins(&left, &top, &right, &bot);
    int width = editableLine->width() - left - right;

    QTreeWidgetItemIterator it(choicesPopup);

    // set the text, according to contents in "choices"
    int i = 0;
    while (*it != NULL)
    {
        (*it)->setText(0, choices[i]);
        ++it;
        ++i;
    }

    // not exactly the global coordinate ... layout margins cause some weird behaviour
    QPoint topLeftGlobal = editableLine->mapToGlobal(QPoint(editableLine->x(), editableLine->y()));
    QPoint topLeft = this->mapFromGlobal(topLeftGlobal) -
                    QPoint(tabOneLayout->contentsMargins().left(), tabOneLayout->contentsMargins().top());
    int topPadding = editableLine->height() - bot;

    choicesPopup->setGeometry(topLeft.x() + left, topLeft.y() + topPadding, width, editableLine->height());
    choicesPopup->show();
}


void Window::linkClicked(const QUrl &link)
{
    if (link.toString() == "back")
    {
        if (simulation->simulationStarted())
            sideText->setText(simulationTabText);
        else if (currentTab == 0)
            sideText->setText(tabOneText);
        else if (currentTab == 1)
            sideText->setText(tabTwoText);
        else
            print("unknown fatality upon url click");
    }
    else if (link.toString() == "credits")
        sideText->setText(credits);
    else if (link.toString() == "accuracy")
        sideText->setText(accuracy);
    else if (link.toString() == "help")
        sideText->setText(instructions);
    else
        QDesktopServices::openUrl(link);
}


// switch to brush mode, and show tiles button
void Window::switchToBrush()
{
    if (placementTogglers->currentIndex() == 1)
        return;

    placementTogglers->setCurrentIndex(1);
    simulation->toggleBrush();

    if (!simulation->simulationStarted())
        tabs->setCurrentIndex(0);
}


void Window::switchToTiles()
{
    if (placementTogglers->currentIndex() == 0)
        return;

    placementTogglers->setCurrentIndex(0);
    simulation->toggleBrush();

    if (!simulation->simulationStarted())
        tabs->setCurrentIndex(0);
}


// pause and resume should not react when tutorial stages 1 and 2 are underway
void Window::pause()
{
    if (tutorial.getCurState() == 1 || tutorial.getCurState() == 2)
        return;

    pauseTogglers->setCurrentIndex(0);
    simulation->setSimulationState(true);
}


void Window::resume()
{
    if (tutorial.getCurState() == 1 || tutorial.getCurState() == 2)
        return;

    pauseTogglers->setCurrentIndex(1);
    simulation->setSimulationState(false);
    restartTogglers->setCurrentIndex(1);
    observeTogglers->setCurrentIndex(1);
}


void Window::updateInfo()
{
    stringstream ss;
    string probStr, timeStr;
    double percentage = 100.0*simulation->getNettedProbability();
    ss << setprecision(1) << fixed << percentage;
    ss >> probStr;
    probability->setText(probStr.c_str());

    ss.str("");
    ss.clear();
    double time = simulation->getRemainingTime();
    ss << setprecision(2) << fixed << time;
    ss >> timeStr;
    simulationTime->setText(timeStr.c_str());
}


void Window::simulationStarted()
{
    simulationTab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    sideBarTopLayout->setCurrentWidget(simTabs);
    sideText->setText(simulationTabText);
    simSpeedSlider->setValue(1);
    pauseTogglers->setCurrentIndex(1);
    restartTogglers->setCurrentIndex(1);
    observeTogglers->setCurrentIndex(1);
}


void Window::resetOccurred()
{
    sideBarTopLayout->setCurrentWidget(tabs);
    sideText->setText(tabOneText);
    pauseTogglers->setCurrentIndex(0);
    restartTogglers->setCurrentIndex(0);
    observeTogglers->setCurrentIndex(0);
}


// tabs can only change in setup mode (there's only 1 tab after simulation starts)
void Window::tabChanged(unsigned tabNum)
{
    if (simulation->simulationStarted())
        return;

    if (tabNum == 0)
        sideText->setText(tabOneText);
    else if (tabNum == 1)
        sideText->setText(tabTwoText);

    currentTab = tabNum;
}


void Window::setXCen()
{
    simulation->setXCen(xCen->value(), false);
}


void Window::setYCen()
{
    simulation->setYCen(yCen->value(), false);
}


void Window::setPacketSpeed()
{
    simulation->setPacketSpeed(speed->value(), false);
}


void Window::setPacketAngle()
{
    simulation->setPacketAngle(angle->value(), false);
}


void Window::setPacketPrecision()
{
    simulation->setPacketPrecision(precision->value(), false);
}


void Window::packetCenterChanged(QVector3D center)
{
    xCen->setValue(center.x());
    yCen->setValue(center.y());
}


void Window::packetSpeedChanged(double _speed)
{
    speed->setValue(_speed);
}


void Window::packetAngleChanged(double _angle)
{
    angle->setValue(_angle);
}


void Window::packetPrecisionChanged(double _precision)
{
    precision->setValue(_precision);
}


void Window::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        print("exiting ...");
        close();
    }
    else
        simulation->keyPressEvent(event);
}


// multiple signals may be connected to this slot (our universal message displayer, if you will)
void Window::displayError(string message)
{
    if (message != "Values were clipped for stability")
    {
        message = "  " + message;
        editableLine->saveCurrentEntry();
        editableLine->setError(true);  // error flag is automatically cleared when the user edits again
        editableLine->clear();
        editableLine->clearFocus();
        editableLine->setPlaceholderText(message.c_str());
    }
}


void Window::mouseReleaseEvent(QMouseEvent *event)
{
    //choicesPopup->hide();
    sideText->setFocus();
}


// this is actually the "dirty" way to do things:
// a correct method of maintaining aspect ratios would be to write a custom layout manager ...
void Window::resizeEvent(QResizeEvent *event)
{
    if (initialResize)
    {
        initialResize = false;

        initialWidth = sideText->width();
        initialHeight = sideText->height();
        previousArea = width()*height();
        setFixedSize(size());
        tutorial.setDimensions();
    }

    // TESTING : pre-liminary resizing issues
    /*
    cout << "resizing ..." << endl;

    double refLength;
    int area = width()*height();

    if (area < previousArea)
    {
        refLength = min(2.0*double(width())/3.0 + 1, double(height()));
        cout << "trying to minimize" << endl;
    }
    else if (area > previousArea)
    {
        refLength = max(2.0*double(width())/3.0 + 1, double(height()));
        cout << "trying to maximize" << endl;
    }
    else
        return;

    sideText->setFixedWidth(refLength/2.0);  // round-off means potential white space
    tabs->setFixedWidth(refLength/2.0);
    buttonPanel->setFixedWidth(refLength/2.0);

    // simulation portion must always be a square in shape
    simulation->setFixedWidth(refLength);
    simulation->setFixedHeight(refLength);

    double wScaling = double(sideText->width())/initialWidth;
    double hScaling = double(sideText->height())/initialHeight;
    rescaleFonts(max(hScaling, wScaling));
    rescaleButtons(max(hScaling, wScaling));
    previousArea = width()*height();

    repaint();*/
}


void Window::handleFirstRun()
{
    QFile file(QCoreApplication::applicationDirPath() + "/savedFlags.txt");
    if (!file.exists())
    {
        print("this file does not exist");
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        print("error occurred while trying to read saved flags");
        return;
    }
    QTextStream ts(&file);
    QString contents;
    ts >> contents;
    file.close();

    if (contents.size() == 0)
    {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write("1");
        file.close();
        tutorial.transition(true);
    }
}



//----------------------------------------------------
// INITIALIZATION HELPERS
//----------------------------------------------------


void Window::createTabs()
{
    tabOneLayout = new QVBoxLayout;
    tabTwoLayout = new QVBoxLayout;
    simulationTabLayout = new QVBoxLayout;

    // add elements to individual tabs
    tabOneLayout->addWidget(editableLine, 3);
    tabOneLayout->addWidget(brushHeightSliderWidget, 2);
    tabOneLayout->addWidget(brushSizeSliderWidget, 2);
    tabOneLayout->addWidget(placementTogglersWidget, 2);
    tabOneLayout->setContentsMargins(0,10*scale,0,15*scale);
    tabOneLayout->setSpacing(0);

    tabTwoLayout->addWidget(xCenWidget);
    tabTwoLayout->addWidget(yCenWidget);
    tabTwoLayout->addWidget(precisionWidget);
    tabTwoLayout->addWidget(speedWidget);
    tabTwoLayout->addWidget(angleWidget);
    tabTwoLayout->setContentsMargins(0,10*scale,0,10*scale);
    tabTwoLayout->setSpacing(0);

    simulationTabLayout->addWidget(simulationTimeWidget);
    simulationTabLayout->addWidget(probabilityWidget);
    simulationTabLayout->addWidget(sensitivitySliderWidget);
    simulationTabLayout->addWidget(simSpeedSliderWidget);
    simulationTabLayout->setContentsMargins(0,20*scale,0,20*scale);
    simulationTabLayout->setSpacing(0);

    // holder widgets for layouts
    tabOne = new QWidget;
    tabTwo = new QWidget;
    simulationTab = new QWidget;

    tabOne->setLayout(tabOneLayout);
    tabTwo->setLayout(tabTwoLayout);
    simulationTab->setLayout(simulationTabLayout);

    // if you're looking for the tab that's in effect during simulation
    // look at routine "simulationStarted"
    // for its contents, look just above these comments
    tabs = new QTabWidget;
    tabs->addTab(tabOne, "Potential");
    tabs->addTab(tabTwo, "Initial");
    tabs->installEventFilter(new TabResizeFilter(tabs));
    tabs->setAutoFillBackground(true);
    tabs->setContentsMargins(0,0,0,0);
    tabs->setDocumentMode(true);
    tabs->setTabPosition(QTabWidget::North);

    QFont tabFont( "Arial", QFont::Medium);
    tabFont.setPixelSize(xlFontSizeVal*1.2);
    tabs->setFont(tabFont);

    simTabs = new QTabWidget;  // as of now, just a single tab
    simTabs->addTab(simulationTab, "Simulation");
    simTabs->installEventFilter(new TabResizeFilter(simTabs));
    simTabs->setAutoFillBackground(true);
    simTabs->setContentsMargins(0,0,0,0);
    simTabs->setDocumentMode(true);
    simTabs->setObjectName("simTabs");
    tabFont.setPixelSize(1.3*xlFontSizeVal);
    simTabs->setFont(tabFont);
}

/*
void Window::createChoicesPopup()
{
    choicesPopup = new QTreeWidget(this);
    choicesPopup->setColumnCount(1);
    choicesPopup->header()->hide();
    choicesPopup->setAttribute(Qt::WA_MacShowFocusRect, 0);
    setShadow(choicesPopup);

    HTMLDelegate* delegate = new HTMLDelegate();
    choicesPopup->setItemDelegate(delegate);

    for (int i = 0; i < choices.size(); ++i)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(choicesPopup);
        item->setText(0, "fill");       // this doesn't matter at all actually
        item->setTextAlignment(0, Qt::AlignLeft);
    }

    choicesPopup->resizeColumnToContents(0);
    choicesPopup->hide();


    // create some actual choices
    choices[0] = "<FONT SIZE = 4 FONT COLOR = '#c8c8c8'>harmonic oscillator";
    choices[1] = "<FONT SIZE = 4 FONT COLOR = '#c8c8c8'>coulomb potential";
}*/


void Window::createTextBox()
{
    sideText = new QTextBrowser;

    QTextCursor font = sideText->textCursor();
    sideText->setFrameStyle(QFrame::NoFrame);
    sideText->setText(tabOneText);
    sideText->setReadOnly(true);
    sideText->setOpenExternalLinks(false);
    sideText->setOpenLinks(false);
}


// label is added on the left of the field
QWidget* Window::addLabel(QWidget *noName, string name, QString toolTip)
{
    if (noName == NULL)
    {
        cout << "No widget was passed into Window::addLabel" << endl;
        return noName;
    }

    QLabel* label = new QLabel;
    QWidget* package = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout;

    name = name + "  ";
    QFont f( "Typewriter", QFont::Light);
    f.setPixelSize(mFontSizeVal);
    label->setText(name.c_str());
    label->setFont(f);
    label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
    noName->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
    layout->addWidget(label, 1);
    layout->addWidget(noName, 1);
    layout->setContentsMargins(20*scale, 0, 20*scale, 0);
    layout->setSpacing(0);

    package->setLayout(layout);

    if (toolTip != "None")
    {
        package->setToolTip(toolTip);
        package->setToolTipDuration(80000); // in milliseconds
    }

    return package;
}


// note that min/max reflect that maximum values that can be typed,
// but not the actual min/max that can be entered/set (most likely clipped by callbacks)
QDoubleSpinBox* Window::createSpinBox(double min, double max, double initial)
{
    QDoubleSpinBox* ret = new QDoubleSpinBox;
    ret->setFixedHeight(25*scale);
    QFont f("Typewriter", QFont::Light);
    f.setPixelSize(msFontSizeVal);
    ret->setFont(f);

    ret->setMinimum(min);
    ret->setMaximum(max);
    ret->setSingleStep(0.0);  // disable +/- arrows
    ret->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ret->setAttribute(Qt::WA_MacShowFocusRect, 0);  // turne blue focus outline off
    ret->setValue(initial);
    return ret;
}


void Window::createSpinBoxes()
{
    speed = createSpinBox(0, 100);
    angle = createSpinBox(-360.0, 360.0);
    xCen = createSpinBox(-100, 100);
    yCen = createSpinBox(-100, 100);
    precision = createSpinBox(0, 100, 1.0);
}


void Window::createButtons()
{
    int l = 45*scale;
    playButton = new QPushButton;
    playButton->setIconSize(QSize(l,l));
    playButton->setFixedSize(QSize(l,l));
    buttons.push_back(playButton);

    pauseButton = new QPushButton;
    pauseButton->setIconSize(QSize(l,l));
    pauseButton->setFixedSize(QSize(l,l));
    buttons.push_back(pauseButton);

    switchToBrushButton = new QPushButton;
    switchToBrushButton->setIconSize(QSize(2*l,l));
    switchToBrushButton->setFixedSize(QSize(2*l,l));
    switchToBrushButton->setToolTip(continuousTip);
    buttons.push_back(switchToBrushButton);

    switchToTilesButton = new QPushButton;
    switchToTilesButton->setIconSize(QSize(2*l,l));
    switchToTilesButton->setFixedSize(QSize(2*l,l));
    switchToTilesButton->setToolTip(discreteTip);
    buttons.push_back(switchToTilesButton);

    restartButton = new QPushButton;
    restartButton->setIconSize(QSize(l,l));
    restartButton->setFixedSize(QSize(l,l));
    restartButton->setToolTip(restartTip);
    buttons.push_back(restartButton);

    restartFiller = new QPushButton;
    restartFiller->setIconSize(QSize(l,l));
    restartFiller->setFixedSize(QSize(l,l));
    restartFiller->setToolTip(fillerTip);
    buttons.push_back(restartFiller);

    observeButton = new QPushButton;
    observeButton->setIconSize(QSize(l,l));
    observeButton->setFixedSize(QSize(l,l));
    observeButton->setToolTip(observeTip);
    buttons.push_back(observeButton);

    classicalButton = new QPushButton;
    classicalButton->setIconSize(QSize(l,l));
    classicalButton->setFixedSize(QSize(l,l));
    classicalButton->setToolTip(classicalTip);
    buttons.push_back(classicalButton);

    observeFiller = new QPushButton;
    observeFiller->setIconSize(QSize(l,l));
    observeFiller->setFixedSize(QSize(l,l));
    observeFiller->setToolTip(fillerTip);
    buttons.push_back(observeFiller);

    placementTogglers = new QStackedWidget;
    placementTogglers->addWidget(switchToBrushButton);
    placementTogglers->addWidget(switchToTilesButton);
    placementTogglers->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    placementTogglers->setContentsMargins(0,0,0,0);

    pauseTogglers = new QStackedWidget;
    pauseTogglers->addWidget(playButton);
    pauseTogglers->addWidget(pauseButton);
    pauseTogglers->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pauseTogglers->setContentsMargins(0,0,0,0);
    simulation->setSimulationState(true);

    observeTogglers = new QStackedWidget;
    observeTogglers->addWidget(observeFiller);
    observeTogglers->addWidget(observeButton);
    observeTogglers->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    observeTogglers->setContentsMargins(0,0,0,0);

    restartTogglers = new QStackedWidget;
    restartTogglers->addWidget(restartFiller);
    restartTogglers->addWidget(restartButton);
    restartTogglers->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    restartTogglers->setContentsMargins(0,0,0,0);
}


void Window::createBrushHeightSlider()
{
    brushHeightSlider = new QSlider(Qt::Horizontal, this);
    brushHeightSlider->setRange(-2, 3);
    brushHeightSlider->setValue(3);
    simulation->setBrushHeight(3);
    brushHeightSlider->setFocusPolicy(Qt::StrongFocus);
}


void Window::createBrushSizeSlider()
{
    brushSizeSlider = new QSlider(Qt::Horizontal, this);
    brushSizeSlider->setRange(10,100);
    simulation->setBrushSize(10);
    brushSizeSlider->setFocusPolicy(Qt::StrongFocus);
}


void Window::createSimSpeedSlider()
{
    simSpeedSlider = new QSlider(Qt::Horizontal, this);
    simSpeedSlider->setRange(1, 20);
    simSpeedSlider->setSingleStep(1);
}


void Window::createSensitivitySlider()
{
    sensitivitySlider = new QSlider(Qt::Horizontal, this);
    sensitivitySlider->setRange(0, 98);   // will be normalized to [0, 0.98]
    sensitivitySlider->setSingleStep(1);
    sensitivitySlider->setValue(98);
    simulation->setSensitivity(98);
}


void Window::createEditableLine()
{
    editableLine = new LineEdit;
    editableLine->setAttribute(Qt::WA_MacShowFocusRect, false);    // turn blue focus outline off
    editableLine->setAttribute(Qt::WA_TranslucentBackground, false);
    editableLine->setPlaceholderText(" Enter equation ...");
    editableLine->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    // margins need to scale with screen size
    editableLine->setContentsMargins(20*scale, 25*scale, 20*scale, 15*scale);

    QFont f( "Typewriter", QFont::Light);
    f.setPixelSize(25*scale);
    editableLine->setFont(f);
    editableLine->setToolTip(equationTip);
}


void Window::createInfoAndTimeLine()
{
    QFont f("Typewriter", QFont::Light);
    f.setPixelSize(mFontSizeVal);

    probability = new LineEdit();
    probability->setFixedHeight(30*scale);
    probability->setFixedWidth(125*scale);
    probability->setFont(f);
    probability->setAttribute(Qt::WA_MacShowFocusRect, 0);
    probability->setReadOnly(true);

    simulationTime = new LineEdit();
    simulationTime->setFixedHeight(30*scale);
    simulationTime->setFixedWidth(125*scale);
    simulationTime->setFont(f);
    simulationTime->setAttribute(Qt::WA_MacShowFocusRect, 0);
    simulationTime->setReadOnly(true);
}


// will only be used by choicePopup for now
/*
void Window::setShadow(QWidget* target)
{
    QGraphicsDropShadowEffect *bodyShadow = new QGraphicsDropShadowEffect();
    bodyShadow->setBlurRadius(9.0);
    bodyShadow->setColor(QColor(30, 50, 90, 255));//160));
    bodyShadow->setOffset(0, 1.5);
    target->setGraphicsEffect(bodyShadow);
}*/


// not used anywhere at the moment
void Window::clearLayout(QLayout *layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != NULL)
    {
        if (item->layout() != NULL)
            clearLayout(item->layout());
        delete item;
    }
}



// could easily make this function recursive ...
void Window::rescaleFonts(double scaling)
{
    sideText->setFontPointSize(ceil(16.0*double(scaling)));
    QString original = sideText->toPlainText();
    sideText->setText(original);
}


void Window::rescaleButtons(double scaling)
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setFixedSize(45.0*scaling, 45.0*scaling);
    editableLine->setFixedHeight(20.0*scaling);
}


void Window::applyExpandingTabsStyle()
{
    ::applyExpandingTabsStyle(tabs);
}


void Window::setObjectNames()
{
    tabOne->setObjectName("tabOne");
    tabTwo->setObjectName("tabTwo");
    simulationTab->setObjectName("simTab");
    tabs->setObjectName("tabs");
    editableLine->setObjectName("equationText");
    sideTextContainer->setObjectName("sideTextContainer");

    // buttons
    observeFiller->setObjectName("observeFiller");
    restartFiller->setObjectName("restartFiller");
    observeButton->setObjectName("observeButton");
    pauseButton->setObjectName("pauseButton");
    playButton->setObjectName("playButton");
    pauseButton->setObjectName("pauseButton");
    restartButton->setObjectName("restartButton");
    switchToBrushButton->setObjectName("switchToBrushButton");
    switchToTilesButton->setObjectName("switchToTilesButton");
    classicalButton->setObjectName("classicalButton");
    placementTogglers->setObjectName("placementTogglers");

    // combined widgets
    buttonPanel->setObjectName("buttonPanel");
    speedWidget->setObjectName("speed");
    angleWidget->setObjectName("angle");
}



//--------------------------------------------------------
// HELP MESSAGES, FONTS
//--------------------------------------------------------


// called at the end of initStrings, replaces keywords, e.g 'lFontSize' with the font size
void Window::initFonts()
{
    // will be calculated based on width/length of window
    // the units used will be in pixels
    ttFontSizeVal = rint(20.0*scale);
    sFontSizeVal = rint(15.0*scale);
    msFontSizeVal = rint(16.0*scale);
    mFontSizeVal = rint(17.0*scale);
    lFontSizeVal = rint(20.0*scale);
    xlFontSizeVal = rint(24.0*scale);

    // not actually sure what units these guys are in
    tableWidthVal = rint(275*scale);
    ttTableWidthVal = rint(450*scale);
    sliderHeightVal = rint(5*scale);
    handleHeightVal = rint(13*scale);
    halfHandleHeightVal = rint(6*scale);
    borderRadiusVal = rint(2*scale);
    scrollbarWidthVal = rint(7*scale);

    ttFontSize = to_string(ttFontSizeVal).c_str();
    sFontSize = to_string(sFontSizeVal).c_str();
    msFontSize = to_string(msFontSizeVal).c_str();
    mFontSize = to_string(mFontSizeVal).c_str();
    lFontSize = to_string(lFontSizeVal).c_str();
    xlFontSize = to_string(xlFontSizeVal).c_str();
    tableWidth = to_string(tableWidthVal).c_str();
    ttTableWidth = to_string(ttTableWidthVal).c_str();
    sliderHeight = to_string(sliderHeightVal).c_str();
    handleHeight = to_string(handleHeightVal).c_str();
    halfHandleHeight = to_string(halfHandleHeightVal).c_str();
    borderRadius = to_string(borderRadiusVal).c_str();
    scrollbarWidth = to_string(scrollbarWidthVal).c_str();

    // replace keywords
    for (int i = 0; i < allHelpText.size(); ++i)
    {
        // it is important to replace the longest ones first, especially if one of their substrings is another keyword...
        *allHelpText[i] = allHelpText[i]->replace("tableWidth", tableWidth);
        *allHelpText[i] = allHelpText[i]->replace("ttTableWidth", ttTableWidth);
        *allHelpText[i] = allHelpText[i]->replace("sliderHeight", sliderHeight);
        *allHelpText[i] = allHelpText[i]->replace("handleHeight", handleHeight);
        *allHelpText[i] = allHelpText[i]->replace("halfHandleHeight", halfHandleHeight);
        *allHelpText[i] = allHelpText[i]->replace("borderRadius", borderRadius);
        *allHelpText[i] = allHelpText[i]->replace("scrollbarWidth", scrollbarWidth);
        *allHelpText[i] = allHelpText[i]->replace("msFont", msFontSize);
        *allHelpText[i] = allHelpText[i]->replace("xlFont", xlFontSize);
        *allHelpText[i] = allHelpText[i]->replace("ttFont", ttFontSize);
        *allHelpText[i] = allHelpText[i]->replace("sFont", sFontSize);
        *allHelpText[i] = allHelpText[i]->replace("mFont", mFontSize);
        *allHelpText[i] = allHelpText[i]->replace("lFont", lFontSize);
    }
}


void Window::initStrings()
{
    allHelpText = {&tabOneHelp, &tabTwoHelp, &simulationHelp, &generalControls, &projectOverview, &speedTip, &angleTip, &centerTip, &precisionTip,
                   &equationTip, &brushSizeTip, &brushHeightTip,
                   &timeTip, &probabilityTip, &speedupTip, &sensitivityTip, &observeTip, &classicalTip, &restartTip, &fillerTip, &continuousTip,
                   &discreteTip, &credits, &accuracy, &instructions, &styleSheet};

    QFile file(":/other/styleSheetBlue.qss");
    file.open(QFile::ReadOnly);
    styleSheet = QLatin1String(file.readAll());
    file.close();

    // majority of text color is rgb(72, 94, 112) -> in hex -> rgb(48, 5e, 70)
    tabOneHelp =    "<FONT COLOR='#485e70'>"
                    "<br><span style = \"font-size: lFontpx; color:#4b6276\"><b> CONTROLS </b></span> <br>"
                    "<table width = tableWidth>"
                    "<tr style = \"font-size: mFontpx\">"
                       "<th align = left> MOUSE <br></th>"
                    "</tr>"
                    "<tr style = \"font-size: msFontpx\">"
                       "<td> Left Click </span></td>"
                       "<td> <i>add potential walls</i> </span></td>"
                    "</tr>"
                    "<tr style = \"font-size: msFontpx\">"
                        "<td> Right Click </td>"
                        "<td> <i>cancel</i> </td>"
                    "</tr></table><br><br>";

    tabTwoHelp =    "<FONT COLOR='#485e70'>"
                    "<br><span style = \"font-size: lFontpx; color:#4b6276\"><b> CONTROLS </b></span> <br>"
                    "<table width = tableWidth>"
                    "<tr style = \"font-size: mFontpx\">"
                       "<th align = left>MOUSE <br></th>"
                    "</tr>"
                    "<tr style = \"font-size: msFontpx\">"
                       "<td>Left Click </td>"
                       "<td><i>set center/velocity</i> </td>"
                    "</tr>"
                     "<tr style = \"font-size: msFontpx\">"
                        "<td> Right Click </td>"
                        "<td> <i>cancel</i> </td>"
                    "</tr></table><br><br>";

    simulationHelp = "<FONT COLOR='#485e70'>"
                    "<br><span style = \"font-size: lFontpx; color:#4b6276\"><b> CONTROLS </b></span> <br>"
                    "<table width = tableWidth>"
                    "<tr style = \"font-size: mFontpx\">"
                       "<th align = left> MOUSE <br></th>"
                    "</tr>"
                    "<tr style = \"font-size: msFontpx\" >"
                       "<td> Left Click </td>"
                       "<td> <i>set probability net</i> </td>"
                    "</tr>"
                     "<tr style = \"font-size: msFontpx\">"
                        "<td> Right Click </td>"
                        "<td> <i>cancel</i> </td>"
                    "</tr></table><br><br>";

    generalControls =
                  "<table width = tableWidth>"
                "<tr style = \"font-size: mFontpx;\">"
                  "<th align = left> KEYBOARD<br></th>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>M</td>"
                  "<td><i>start tutorial</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td> W/A/S/D </td>"
                  "<td><i>swing camera</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>Z/X</td>"
                  "<td><i>zoom in/out</i> </td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>P</td>"
                  "<td><i>go to perch</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>I</td>"
                  "<td><i>view potential</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>1</td>"
                  "<td><i>probability density</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td> 2 </td>"
                  "<td><i>real component</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>3</td>"
                  "<td><i>imag component</i></td>"
                "</tr>"
                "<tr style = \"font-size: msFontpx\" >"
                  "<td>C</td>"
                  "<td><i>toggle colormaps</i></td>"
                "</tr>"
                "</table>"
                "<br>";

    projectOverview = "<FONT COLOR='#485e70'>"
                      "<span style = \"font-size: lFontpx; color:#4b6276\"> <br><b> OVERVIEW </b></span><br><br>"
                      "<span style = \"font-size: sFontpx;\"> A 2D particle is released."
                      "&nbsp; Its location is not known, but the probability of a measurement yielding a certain position is."
                      "&nbsp; <br> Customize its environment and initial conditions.<br><br><br></span>"
                      ""
                      "<span style = \"font-size: lFontpx; color:#4b6276\"> <b> DETAILS </b></span><br><br>"
                      "<span style = \"font-size:msFontpx;\">"
                      "<a href=\"help\"><span style=\" text-decoration:none ; color:#696B9E;\">Instructions</a>"
                      "<br><a href=\"accuracy\"><span style=\" text-decoration:none ; color:#696B9E;\">Simulation Accuracy</a>"
                      "<br><a href=\"credits\"><span style=\" text-decoration:none ;  color:#696B9E;\">Credits</a>"
                      "</span><br><br><br>";

    // tool tips
    brushSizeTip = " <span style = \"font-size: ttFontpx;\">"
                   " Applies only to continuous potential placement mode"
                   " </span>"
                   " ";

    brushHeightTip = " <span style = \"font-size: ttFontpx;\"> "
                     " Left half of slider = <b>'-'</b> <br>"
                     " Right half of slider = <b>'+'</b> <br><br>"
                     " Applies only to continuous potential placement mode"
                     "</span>";

    timeTip = " <span style = \"font-size: ttFontpx;\">"
              " Remaining simulation time in seconds, <b>not</b> world time"
              "</span>";

    probabilityTip = " <span style = \"font-size: ttFontpx;\">"
                     "Net probability of finding the particle within the green wall."
                     " Click and drag to change the wall as desired."
                     "<br><br> <b>Note:</b><br>"
                     "Small, erroneous fluctuations, can be expected (although uncommon)."
                     "</span>";

    speedTip = " <span style = \"font-size: ttFontpx;\">"
               " Setting <i>group velocity</i>: <br>"
               " Set the speed of the wavepacket."
               " May be done by clicking on screen."
               "</span>";

    angleTip = " <span style = \"font-size: ttFontpx;\">"
               " Setting <i>group velocity</i>: <br>"
               " Set the angle of the wavepacket."
               " May be done by clicking on the arrow"
               "</span>";

    centerTip = " <span style = \"font-size: ttFontpx;\">"
                " Set the initial center of the wavepacket."
                " May be done by clicking on the wavepacket."
                "</span>";


    precisionTip = " <span style = \"font-size: ttFontpx;\">"
                   " Set the extent of knowledge of the particle's"
                   " initial location. <br><br>"
                   " As per the <i>Uncertainty Principle</i>, "
                   " higher precisions lead to greater uncertainty in momentum."
                   "</span>";


    equationTip = " <span style = \"font-size: ttFontpx;\">"
                  " <u>Clears</u> current barriers, sets up a new one according to given equation. <br>"
                  " Potential is the z-axis, input variables can <i>only</i> be 'x', 'y', 'r'. <br><br>"
                  " <b>Examples:</b>  <br>"
                  "<table width = ttTableWidth>"
                  "<tr>"
                      "<td> 0.7r^2 - 30 </td>"
                      "<td style='text-align:right;vertical-align:middle'> <i>harmonic oscillator</i> </td>"
                  "</tr>"
                  "<tr>"
                      "<td> -30/r + 30 </td>"
                      "<td style='text-align:right;vertical-align:middle'> <i>coulomb potential</i> </td>"
                  "</tr>"
                  "<tr>"
                      "<td> 30cos(abs(x-yy)) </td>"
                      "<td style='text-align:right;vertical-align:middle'> <i>???</i> </td>"
                  "</tr></table>"
                  "<br><br>"
                  " <b>Hint</b>: &nbsp;&nbsp; cut-off potentials are 30 and -30"
                  "</span>";

    speedupTip = "<span style = \"font-size: ttFontpx;\">"
                 "Speed up the simulation.<br><br>"
                 "<b>Note:</b><br> "
                 " Accuracy is kept the same, but more computational power is used. "
                 "Going past your CPUs' limit will not speed things up"
                 "</span>";


    sensitivityTip = "<span style = \"font-size: ttFontpx;\">"
                     "Change the sensitivity of current color-map.<br>"
                     " Adjust to help see relative values more clearly."
                     "</span>";

    observeTip = "<span style = \"font-size: ttFontpx;\">"
                 "Make an observation, revealing the particle's position."
                 " This will 'collapse' the wavefunction into a small area.<br><br>"
                 "<b>Note:</b> &nbsp;&nbsp; As in practice, there is still a degree of uncertainty even after measurement"
                 "</span>";

    classicalTip = "<span style = \"font-size: ttFontpx;\">"
                   "Show the classical particle under the same initial conditions as the wavepacket. <br><br>"
                   " As a separate simulation, uses newtonian mechanics (i.e F = ma). <br><br>"
                   "<b>Note:</b> &nbsp;&nbsp; This is an imperfect analogy to the quantum case: a better representation"
                   " would be a cluster of point particles with varying initial positions and velocities. "
                   "</span>";

    restartTip = "<span style = \"font-size: ttFontpx;\">"
                 "Reset the wavefunction. Potential is not cleared. <br><br>"
                 "<b>Hint</b>: &nbsp;&nbsp; to reset potentials, simply set the equation as '0'"
                 "</span>";

    fillerTip = "<span style = \"font-size: ttFontpx;\">"
                "Available after simulation has started"
                "</span>";

    continuousTip = "<span style = \"font-size: ttFontpx;\">"
                    "Switch to continuous potential placement mode.<br><br>"
                    "<b>Tip:</b> &nbsp;&nbsp; Spin the camera while painting to get radial potentials"
                    "</span>";

    discreteTip = "<span style = \"font-size: ttFontpx;\">"
                  "Switch to discrete potential placement mode. <br><br>"
                  "</span>";


    credits = "<FONT COLOR='#485e70'>"
              "<br><br><span style = \"font-size:lFontpx; color:#4b6276;\"><b> CREDITS </b></span><br><br><br>"
              "<span style = \"font-size:msFontpx;\">"
              " The algorithm used was inspired by this 1991 &nbsp;"
              "<a href=\"http://www.hunter.cuny.edu/physics/courses/physics485/repository/files/reference-papers/Visscher%20NumSol%20Schrodinger%20Eqt.pdf\">"
              "<span style=\" text-decoration: underline; color:#3a60ed;\">paper</a><br><br>"
              " Equation parsing was facilitated by the lua interpreter and ae library<br><br>"
              " User interface was done in QT,  graphics in OpenGL<br><br>"
              " Color maps were taken from scipy's matplotlib"
              "</span>"
              "<span style = \"font-size:mFontpx;\">"
              "<br><br><br> <a href=\"back\"><span style=\" text-decoration:none; color:#696B9E;\">Back</a>"
              "</span>"
              "";

    accuracy = "<FONT COLOR ='#485e70'>"
               "<br><br><span style = \"font-size:lFontpx; color:#4b6276;\"><b> ACCURACY </b></span><br><br><br>"
               "<span style = \"font-size:msFontpx;\">"
               "<i> Correctness is not guaranteed, and results should be taken with a grain of salt. However, "
               "simulation accuracy is still a top priority. The following steps were taken to minimize errors: </i>"
               "<br><br>"
               "Enforced time-limit, to prevent a noticeable buildup of errors <br><br>"
               "Use of a 2nd order symplectic algorithm (velocity verlet) <br><br>"
               "Use of 6th order approximations to the laplacian operator <br><br>"
               "<b>Parameters:</b><br> dt = 0.001, 199x199 grid <br><br>"
               "<b>Note:</b><br> upsampling (bicubic) to 397x397 was applied <br><br> "
               "</span>"
               "<span style = \"font-size:mFontpx;\">"
               "<br><br><br> <a href=\"back\"><span style=\" text-decoration:none; color:#696B9E;\">Back</a>"
               "</span>"
               "";

    instructions = "<FONT COLOR ='#485e70'>"
                  "<br><br><span style = \"font-size:lFontpx; color:#4b6276; \"><b> INSTRUCTIONS </b></span><br><br><br>"
                  "<span style = \"font-size:msFontpx;\">"
                  "<i>A typical session : </i><br><br>"
                  "1) Click 'Potential' tab, set barriers <br>"
                  "2) Click 'Initial' tab, set properties <br>"
                  "3) Press play to start <br>"
                  "4) Press observe to reveal location <br>"
                  "5) Drag green net to find probability <br>"
                  "6) Press reset button <br><br><br>"
                  "<b>Note:</b><br><br> For detailed info, read tooltips."
                  " Most values can be set via mouse clicks on the simulation plane."
                  "</span>"
                  "<span style = \"font-size:mFontpx;\">"
                  "<br><br><br> <a href=\"back\"><span style=\" text-decoration:none; color:#696B9E;\">Back</a>"
                  "</span>"
                  "";

    initFonts();
    tabOneText = projectOverview + tabOneHelp + generalControls;
    tabTwoText = projectOverview + tabTwoHelp + generalControls;
    simulationTabText = projectOverview + simulationHelp + generalControls;
}


