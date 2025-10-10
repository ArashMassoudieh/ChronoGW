TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt


INCLUDEPATH += InverseModeling/include/GA
INCLUDEPATH += InverseModeling/include/MCMC
INCLUDEPATH += Utilities/

SOURCES += \
    GWA.cpp \
    InverseModeling/src/GA/Binary.cpp \
    InverseModeling/src/GA/Distribution.cpp \
    InverseModeling/src/GA/DistributionNUnif.cpp \
    InverseModeling/src/GA/Individual.cpp \
    Tracer.cpp \
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
    MCMC.h \
    Tracer.h \
    Well.h
