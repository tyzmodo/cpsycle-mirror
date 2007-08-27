@echo off

%~d0
cd %~p0

set pkgdir=..\..\..\..\..\external-packages\boost-1.33.1
set libdir=lib-mswindows-msvc-8.0-cxxabi-1400

if not exist ..\..\..\..\include\boost (
	pushd %pkgdir% || exit /b 1
	..\7za\7za x -y include.tar.bz2 || exit /b 1
	..\7za\7za x -y include.tar || exit /b 1
	del/q include.tar || exit /b 1
	popd || exit /b 1

	if not exist ..\..\..\..\include\ (
		mkdir ..\..\..\..\include\ || exit /b 1
	)
	rem cmd's move command bugs with long paths, so we try cygwin's mv command first
	mv %pkgdir%\include\boost ..\..\..\..\include\boost || (
		move %pkgdir%\include\boost ..\..\..\..\include\boost || exit /b 1
	)
	rmdir/s/q %pkgdir%\include || exit /b 1
)

if not exist ..\..\output\boost_stamp (
	pushd %pkgdir% || exit /b 1
	..\7za\7za x -y %libdir%.tar.bz2 || exit /b 1
	..\7za\7za x -y %libdir%.tar || exit /b 1
	del/q %libdir%.tar || exit /b 1
	popd || exit /b 1

	if not exist ..\..\output\debug\bin (
		mkdir ..\..\output\debug\bin || exit /b 1
	)
	move %pkgdir%\%libdir%\*-gd-*.dll ..\..\output\debug\bin || exit /b 1

	if not exist ..\..\output\debug\lib (
		mkdir ..\..\output\debug\lib || exit /b 1
	)
	move %pkgdir%\%libdir%\*-gd-*.lib ..\..\output\debug\lib || exit /b 1

	if not exist ..\..\output\release\bin (
		mkdir ..\..\output\release\bin || exit /b 1
	)
	move %pkgdir%\%libdir%\*.dll ..\..\output\release\bin || exit /b 1

	if not exist ..\..\output\release\lib (
		mkdir ..\..\output\release\lib || exit /b 1
	)
	move %pkgdir%\%libdir%\*.lib ..\..\output\release\lib || exit /b 1

	echo boost extracted > ..\..\output\boost_stamp || exit /b 1
	rmdir/s/q %pkgdir%\%libdir% || exit /b 1
)
