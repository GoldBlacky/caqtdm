BUILDING:
First, the following environment variables need to be defined with the respective paths. They aready have a default for WIN32 and else, as shown by ENV = default:

WIN32:
	GTESTHOME = "D:/Google_tests"
	QTHOME = "D:/qt/build/Qt-6.5.1_VS22_64bit"
	GTESTINCLUDE = "D:/Google_tests/googletest/include"
	QWTINCLUDE = "D:/qt/qwt-6.2.0/src"
	QWTLIB = "D:/qt/qwt-6.2.0/lib"
	QWTLIBNAME = "qwt"
else:
	GTESTINCLUDE = "/usr/include/gtest"
	QTHOME = "/usr" (used to build string /usr/include/qt5)
	QWTINCLUDE = "/usr/include/qt5/qwt"
	QWTLIB = "/usr/lib64"
	QWTLIBNAME = "qwt-qt5"

Then, building the unit test project should work without much further action needed, simply by opening the unit test project in QtCreator and the clicking the build icon.

RUNNING:
To run the unit test project, the paths of "Com.dll" and "qwt.dll" need to be on PATH.
In QtCreator, this can be easily done under Projects > Build & Run > Run > Environment and then add something along the lines of "Path+=;D:\qt\qwt-6.2.0\lib;D:\epics\Package\base\bin\windows-x64" in batch edit mode.
Afterwards, it should also be possible to run the tests under the "Test Results" tab.
Because there are different test directories for different tested directories (e.g. Test_ArchiveHTTP and Test_caStripPlot), there are also different binaries.
Simply by clicking run, you cannot execute all tests from all binaries at once. However, you can use the "Test Results" tab, as QtCreator can identify all binaries and run them all together there.

IMPORTANT: The unit test project does NOT build caQtDM. So if you make changes to caQtDM, manually build it first, before running the tests.

When including headers that contain the Q_DECL_EXPORT macro in their class definition, it most likely fails, as it did for me. To workaround this, I extended the tested class headers by the following:
#ifdef CAQTDM_UNITTEST_INCLUDE
class UrlHandlerHttp : public QObject
#else
class Q_DECL_EXPORT UrlHandlerHttp : public QObject
#endif
{
... etc

Then, when including the header in the test, this can be used like this:
#define CAQTDM_UNITTEST_INCLUDE
#include "urlhandlerhttp.h"

Like this, it will still build with Q_DECL_EXPORT (=neccessary), but the includes will also work.
This is certainly not the best or nicests solution, feel free to improve it and update this documentation.