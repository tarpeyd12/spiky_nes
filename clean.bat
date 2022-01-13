@echo off
rem this cleans up all the saves, crash saves, graphing csv's and log files in ./exec/
pause
pushd exec
del *.xml
del *.csv
del *.log
popd
echo All done.
pause
