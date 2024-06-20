cmake_minimum_required(VERSION 3.5)    
set(CMAKE_CXX_STANDARD 11)

project(Test_ArchiveHTTP)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${GTESTINCLUDE} ${QTINCLUDE} ${QTINCLUDE}/QtCore ../../../caQtDM_Lib/caQtDM_Plugins/archive/archiveHTTP/)

add_executable(Test_ArchiveHTTP
	src/tst_urlhandlerhttp.cpp
	src/tst_httpperformancedata.cpp)
target_link_directories(Test_ArchiveHTTP PRIVATE ../../../${CAQTDMBINARIESNAME} ../../../${CAQTDMBINARIESNAME}/controlsystems ${GTESTLIB})
target_link_libraries(Test_ArchiveHTTP gtest gtest_main archiveHTTP_plugin Qt::Core)
