# LIII
multi-platform bittorrent client

## Windows development/build environment:
* CMake
* Microsoft Visual Studio Community 2017 / Microsoft Visual C++ 2017
* Qt 5.12.10:
	- http://www.nic.funet.fi/pub/mirrors/download.qt-project.org/archive/qt/5.12/5.12.10/qt-opensource-windows-x86-5.12.10.exe - msvc2017 and msvc2017_64
	- http://www.nic.funet.fi/pub/mirrors/download.qt-project.org/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.debug_info.win64_msvc2017_64/5.12.10-0-202011040843qtbase-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64-debug-symbols.7z - x64 Qt Core debug  symbols can be found here.
* Boost:
	- https://deac-fra.dl.sourceforge.net/project/boost/boost-binaries/1.76.0/boost_1_76_0-msvc-14.1-64.exe
	- https://deac-ams.dl.sourceforge.net/project/boost/boost-binaries/1.76.0/boost_1_76_0-msvc-14.1-32.exe
	
BOOST_ROOT environment variable set accordingly.

Both Qt Creator and Visual Studio can be used.
