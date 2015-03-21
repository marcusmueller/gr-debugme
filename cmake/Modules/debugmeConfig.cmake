INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_DEBUGME debugme)

FIND_PATH(
    DEBUGME_INCLUDE_DIRS
    NAMES debugme/api.h
    HINTS $ENV{DEBUGME_DIR}/include
        ${PC_DEBUGME_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    DEBUGME_LIBRARIES
    NAMES gnuradio-debugme
    HINTS $ENV{DEBUGME_DIR}/lib
        ${PC_DEBUGME_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DEBUGME DEFAULT_MSG DEBUGME_LIBRARIES DEBUGME_INCLUDE_DIRS)
MARK_AS_ADVANCED(DEBUGME_LIBRARIES DEBUGME_INCLUDE_DIRS)

