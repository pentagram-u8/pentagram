@ECHO OFF
setlocal
set Conf=x%*
set Conf2=%Conf:*AMD64=AMD64%
if x%Conf2:~0,5%x == xAMD64x ( 
	set ConfCPU=AMD64
) ELSE ( 
	set ConfCPU=i386
)

if x%CPU%x == xx (
	echo TestARCH : warning: CPU environment variable not set. Assuming i386 environment.
	set CPU=i386
)

if x%ConfCPU%x == x%CPU%x (
	echo TestARCH : Buidling for "%CPU%"
	exit /b 0
) ELSE (
	echo TestARCH : error: Mismatched CPU Settings. Environment is "%CPU%" while Configuration is "%ConfCPU%"
	exit /b -1
)
