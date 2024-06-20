cmake_minimum_required(VERSION 3.5)    
set(CMAKE_CXX_STANDARD 11)

project(Test_caStripPlot)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${GTESTINCLUDE} ${QTINCLUDE} ${QTINCLUDE}/QtCore ${QTINCLUDE}/QtGui ${QTINCLUDE}/QtWidgets ${QWTINCLUDE} ../../../caQtDM_QtControls/src)

add_executable(Test_caStripPlot src/tst_castripplot.cpp)
target_link_directories(Test_caStripPlot PRIVATE ../../../${CAQTDMBINARIESNAME} ${QWTLIB} ${GTESTLIB})
target_link_libraries(Test_caStripPlot gtest gtest_main qtcontrols Qt::Core Qt::Widgets ${QWTLIBNAME})
