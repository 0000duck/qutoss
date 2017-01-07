#! /bin/bash
## setting up default ids for the libs

install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Frameworks/QtCore.framework/Versions/5/QtCore

install_name_tool -id @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets QM_visual.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets

install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui QM_visual.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui

install_name_tool -id @executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent QM_visual.app/Contents/Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent

install_name_tool -id @executable_path/../Frameworks/QtDBus.framework/Versions/5/QtDBus QM_visual.app/Contents/Frameworks/QtDBus.framework/Versions/5/QtDBus

install_name_tool -id @executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport QM_visual.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport

## getting the executable to find dynamic libs in the right place 

install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui QM_visual.app/Contents/MacOS/QM_visual

install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/MacOS/QM_visual

install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets QM_visual.app/Contents/MacOS/QM_visual

install_name_tool -change @rpath/QtConcurrent.framework/Versions/5/QtConcurrent @executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent QM_visual.app/Contents/MacOS/QM_visual

## getting the libraries to find other libraries in the right places
## redirections for QtGui
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui

## redirections for QtWidgets
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets

install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui QM_visual.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets

## redirections for QtConcurrent
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent

## redirections for libqcocoa

install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui QM_visual.app/Contents/Plugins/platforms/libqcocoa.dylib

install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Plugins/platforms/libqcocoa.dylib

install_name_tool -change @rpath/QtDBus.framework/Versions/5/QtDBus @executable_path/../Frameworks/QtDBus.framework/Versions/5/QtDBus QM_visual.app/Contents/Plugins/platforms/libqcocoa.dylib

install_name_tool -change @rpath/QtPrintSupport.framework/Versions/5/QtPrintSupport @executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport QM_visual.app/Contents/Plugins/platforms/libqcocoa.dylib

install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets QM_visual.app/Contents/Plugins/platforms/libqcocoa.dylib


## redirections for QtDBus
install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Frameworks/QtDBus.framework/Versions/5/QtDBus

## redirections for QtPrintSupport
install_name_tool -change @rpath/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets QM_visual.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport

install_name_tool -change @rpath/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui QM_visual.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport

install_name_tool -change @rpath/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore QM_visual.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport




