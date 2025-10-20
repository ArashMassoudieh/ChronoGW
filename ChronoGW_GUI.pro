#-------------------------------------------------
# GWA - Groundwater Age Modeling Application
# Qt Project File
#-------------------------------------------------

QT += core gui widgets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GWA
TEMPLATE = app

# C++ Standard
CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += Q_GUI_SUPPORT
DEFINES += Q_JSON_SUPPORT

DEFINES += GSL
DEFINES += _arma

# Optimization flags
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += -O3
}

# Platform-specific configurations
win32 {
    DEFINES += _WINDOWS
    # Add Windows-specific libraries if needed
}

unix:!macx {
    # Linux-specific configuration
    LIBS += -lopenblas -lgomp
    QMAKE_CXXFLAGS += -fopenmp
}

macx {
    DEFINES += _MacOS
    DEFINES += mac_version
    # macOS-specific configuration
    # Note: OpenMP may not be available on macOS by default
}

# OpenMP support (if available)
!macx {
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
}

#-------------------------------------------------
# Source Files
#-------------------------------------------------

SOURCES += \
    GWA.cpp \
    IconListWidget.cpp \
    InverseModeling/observation.cpp \
    InverseModeling/parameter.cpp \
    InverseModeling/parameter_set.cpp \
    InverseModeling/src/GA/Binary.cpp \
    InverseModeling/src/GA/DistributionNUnif.cpp \
    InverseModeling/src/GA/GADistribution.cpp \
    InverseModeling/src/GA/Individual.cpp \
    LIDconfig.cpp \
    Tracer.cpp \
    Utilities/Distribution.cpp \
    Utilities/Matrix.cpp \
    Utilities/Matrix_arma.cpp \
    Utilities/NormalDist.cpp \
    Utilities/QuickSort.cpp \
    Utilities/Utilities.cpp \
    Utilities/Vector.cpp \
    Utilities/Vector_arma.cpp \
    Well.cpp \
    chartviewer.cpp \
    chartwindow.cpp \
    constantortimeserieswidget.cpp \
    main_gui.cpp \
    parameterdialog.cpp \
    parametervaluewidget.cpp \
    tracerdialog.cpp \
    valueorparameterwidget.cpp \
    welldialog.cpp

#-------------------------------------------------
# Source Files
#-------------------------------------------------

HEADERS += \
    GA.h \
    GWA.h \
    IconListWidget.h \
    InverseModeling/include/GA/Binary.h \
    InverseModeling/include/GA/Distribution.h \
    InverseModeling/include/GA/DistributionNUnif.h \
    InverseModeling/include/GA/GA.h \
    InverseModeling/include/GA/GA.hpp \
    InverseModeling/include/GA/Individual.h \
    InverseModeling/include/MCMC/MCMC.h \
    InverseModeling/include/MCMC/MCMC.hpp \
    InverseModeling/levenbergmarquardt.h \
    InverseModeling/levenbergmarquardt.hpp \
    InverseModeling/observation.h \
    InverseModeling/parameter.h \
    InverseModeling/parameter_set.h \
    Tracer.h \
    Utilities/Distribution.h \
    Utilities/Matrix.h \
    Utilities/Matrix_arma.h \
    Utilities/NormalDist.h \
    Utilities/QuickSort.h \
    Utilities/TimeSeries.h \
    Utilities/TimeSeries.hpp \
    Utilities/TimeSeriesSet.h \
    Utilities/TimeSeriesSet.hpp \
    Utilities/Vector.h \
    Utilities/Vector_arma.h \
    Well.h \
    chartviewer.h \
    chartwindow.h \
    constantortimeserieswidget.h \
    parameterdialog.h \
    parametervaluewidget.h \
    tracerdialog.h \
    valueorparameterwidget.h \
    welldialog.h

#-------------------------------------------------
# GUI Components (add as you create them)
#-------------------------------------------------

 SOURCES += \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

ORMS += \
    gui/mainwindow.ui

RESOURCES += \
    resources/resources.qrc

#-------------------------------------------------
# Include Paths
#-------------------------------------------------

INCLUDEPATH += InverseModeling
INCLUDEPATH += InverseModeling/include/GA
INCLUDEPATH += InverseModeling/include/MCMC
INCLUDEPATH += Utilities/

#-------------------------------------------------
# Default rules for deployment
#-------------------------------------------------

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp

linux {
    #sudo apt-get install libblas-dev liblapack-dev
     DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS
     LIBS += -larmadillo -llapack -lblas -lgsl -lopenblas

}

#-------------------------------------------------
# Additional Libraries
#-------------------------------------------------

# Add any additional libraries your project needs
# LIBS += -L/path/to/libs -llibname

#-------------------------------------------------
# Build Configuration Messages
#-------------------------------------------------

message(Building GWA Groundwater Age Modeling Application)
message(Qt Version: $$[QT_VERSION])
message(Build Configuration: $$CONFIG)

FORMS += \
    mainwindow.ui
