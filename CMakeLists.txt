

# This is a skeleton CMakeLists.txt file, auto-generated on
# Wed May 31 12:16:48 CDT 2023.  The developer(s) of this package should delete
# this comment as well as adding dependent targets, packages,
# etc. specific to the package. For details on how to write a package,
# please see
# https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/

cmake_minimum_required(VERSION 3.12)
project(fdreadoutmodules VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()


find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(opmonlib REQUIRED)
find_package(readoutmodules REQUIRED)
find_package(fdreadoutlibs REQUIRED)
find_package(fddetdataformats REQUIRED)
find_package(utilities REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen

daq_codegen(dummymodule.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2)
daq_codegen(dummymoduleinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2)

daq_codegen(dummyconsumerinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2)

##############################################################################
# Dependency sets
set(FDREADOUTMODULES_DEPENDENCIES
#tools
  appfwk::appfwk
  logging::logging
  readoutlibs::readoutlibs
  readoutmodules::readoutmodules
  fdreadoutlibs::fdreadoutlibs
  fddetdataformats::fddetdataformats
)

set(FDREADOUTLIBS_USE_INTRINSICS ON)

if(${FDREADOUTLIBS_USE_INTRINSICS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
endif()
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( LINK_LIBRARIES ${FDREADOUTMODULES_DEPENDENCIES} ) # Any source files and/or dependent libraries to link in not yet determined

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings


daq_add_python_bindings(*.cpp LINK_LIBRARIES ${PROJECT_NAME} ) # Any additional libraries to link in beyond the main library not yet determined


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

daq_add_plugin(DataRecorder duneDAQModule LINK_LIBRARIES appfwk::appfwk readoutmodules::readoutmodules fdreadoutlibs::fdreadoutlibs)
daq_add_plugin(ErroredFrameConsumer duneDAQModule LINK_LIBRARIES appfwk::appfwk readoutmodules::readoutmodules fddetdataformats::fddetdataformats)
daq_add_plugin(FragmentConsumer duneDAQModule LINK_LIBRARIES appfwk::appfwk readoutmodules::readoutmodules fddetdataformats::fddetdataformats)
daq_add_plugin(TimeSyncConsumer duneDAQModule LINK_LIBRARIES appfwk::appfwk readoutmodules::readoutmodules utilities::utilities)

daq_add_plugin(FDDataLinkHandler duneDAQModule LINK_LIBRARIES appfwk::appfwk readoutmodules::readoutmodules fdreadoutlibs::fdreadoutlibs)
daq_add_plugin(FDFakeCardReader duneDAQModule LINK_LIBRARIES appfwk::appfwk readoutmodules::readoutmodules fdreadoutlibs::fdreadoutlibs)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

#daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

