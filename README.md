# Crossplattform-Debug-Logger

A LINUX/OSX Crossplattform Debug Logger with Webserver to get realtime Logs of your program.
Create log-entries with timestamp and functionname/line and debuglevels.
Create linecharts (using the `canvasjs.min.js` lib) and callstacks.
Export logged data to html,xml,csv (charts only).

Samples of the exported data can be sound at `/samples/`.

To use this lib on Windows, please use the CMake-File to build a windows compatible project.

# FEATURES
* HTML, CSV, XML Data export for further processing
* Debuglevels eg. ERROR, INFO, WARNING
* Seperate Webserverthread, can be start/stop at runtime
* Create multible linecharts (required canvasjs.min.js lib)
* Create multible callstacks at runtime
* Group log entries

### LINECHARTS
* If you want to use linecharts please download the `canvasjs.min.js` lib and put them to the export dir.
* The webserver uses the direct online version of the lib : http://canvasjs.com/assets/script/canvasjs.min.js

# SAMPLE CODE
A test project to show the basic usage of the lib is located at : `/src/FRM_DEBUG_LOGGER_TESTER/FRM_DEBUG_LOGGER_TESTER/main.cpp`

# TODO 
* Format the callstack output to a good readable format
* add timer for func durations

# IMAGES
### LOG ENTRIES
![alt tag](https://raw.githubusercontent.com/RBEGamer/Debug-Logger/master/images/startup_section.png)
### CHARTS
![alt tag](https://raw.githubusercontent.com/RBEGamer/Debug-Logger/master/images/line_chart.png)
### CALLSTACK
![alt tag](https://raw.githubusercontent.com/RBEGamer/Debug-Logger/master/images/callstack.png)
