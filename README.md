# OVERVIEW

qRestAPI is a cross-platform Qt-based library allowing to easily query a [Midas](https://github.com/midasplatform/midas) or [XNAT](http://xnat.org) server.

---

**Update**: Girder https://github.com/girder/girder is a more modern, better supported application than Midas Server. Look at Girder before using Midas Server.

---

## Prerequisites

 * Qt 4
 * CMake

## How to build

    git clone git://github.com/commontk/qRestAPI.git
    mkdir qRestAPI-build
    cd qRestAPI-build
    cmake -DQT_QMAKE_EXECUTABLE:FILEPATH=/path/to/cmake ../qRestAPI
    make -j4
    ctest

## Contribute
Fork + pull.
