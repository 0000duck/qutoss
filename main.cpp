
//#include <OpenCL/cl.h>
#include <QTextStream>
#include <QSurfaceFormat>
#include <QtGui>
#include <QLabel>
#include "window.h"


/*
For the future (possibly near future):
 - use OpenCL to accelerate computations
 - analytically compute some eigenfunctions (or test it from scipy)

TDSE eigenstates solver feature:
 - FE-DVR could be helpful ... seems that Lobatto functions may be very robust, worth testing after all else is done

 NOTE ON COMMENTED OUT CODE:
 - old stuff is marked with "OLD VERSION"
 - test stuff is marked with "TESTING"
 - let the command-F games begin...
*/


using namespace std;


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if defined(Q_OS_MAC)
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();

    if (!(dir.cd("PlugIns") && dir.cd("platforms")))
    {
        cout << "Error encountered while looking for platform plugins" << endl;
        return 1;
    }

    QCoreApplication::removeLibraryPath(QLibraryInfo::location(QLibraryInfo::PluginsPath));
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));

#else
    // windows stuff, to be implemented
    // actually, it might not be necessary

#endif

    QSurfaceFormat fmt;
    fmt.setVersion(3,3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    Window window;
    window.resize(window.sizeHint());
    window.show();

    return app.exec();
}
