TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt


INCLUDEPATH += InverseModeling
INCLUDEPATH += InverseModeling/include/GA
INCLUDEPATH += InverseModeling/include/MCMC
INCLUDEPATH += Utilities/

DEFINES += GSL
DEFINES += _arma

SOURCES += \
    GWA.cpp \
    InverseModeling/observation.cpp \
    InverseModeling/parameter.cpp \
    InverseModeling/parameter_set.cpp \
    InverseModeling/src/GA/DistributionNUnif.cpp \
    InverseModeling/src/GA/GADistribution.cpp \
    InverseModeling/src/GA/Individual.cpp \
    LIDconfig.cpp \
    Tracer.cpp \
    Utilities/Distribution.cpp \
    Utilities/Matrix.cpp \
    Utilities/Matrix_arma.cpp \
    Utilities/QuickSort.cpp \
    Utilities/Utilities.cpp \
    Utilities/Vector.cpp \
    Utilities/Vector_arma.cpp \
    Well.cpp \
    main.cpp

HEADERS += \
    GA.h \
    GWA.h \
    InverseModeling/include/GA/Binary.h \
    InverseModeling/include/GA/Distribution.h \
    InverseModeling/include/GA/DistributionNUnif.h \
    InverseModeling/include/GA/GA.h \
    InverseModeling/include/GA/GA.hpp \
    InverseModeling/include/GA/Individual.h \
    InverseModeling/observation.h \
    InverseModeling/parameter.h \
    InverseModeling/parameter_set.h \
    Tracer.h \
    Utilities/Distribution.h \
    Utilities/Matrix.h \
    Utilities/Matrix_arma.h \
    Utilities/QuickSort.h \
    Utilities/TimeSeries.h \
    Utilities/TimeSeries.hpp \
    Utilities/TimeSeriesSet.h \
    Utilities/TimeSeriesSet.hpp \
    Utilities/Vector.h \
    Utilities/Vector_arma.h \
    Well.h

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -fopenmp

linux {
    #sudo apt-get install libblas-dev liblapack-dev
     DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS
     LIBS += -larmadillo -llapack -lblas -lgsl -lopenblas

}
