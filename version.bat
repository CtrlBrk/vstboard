set APP_VERSION_MAJOR=0
set APP_VERSION_MINOR=6
set APP_VERSION_BUILD=0
set APP_VERSION_PATCH=0

echo DEFINES += 'APP_VERSION_MAJOR=%APP_VERSION_MAJOR%' > src\version.pri
echo DEFINES += 'APP_VERSION_MINOR=%APP_VERSION_MINOR%' >> src\version.pri
echo DEFINES += 'APP_VERSION_BUILD=%APP_VERSION_BUILD%' >> src\version.pri
echo DEFINES += 'APP_VERSION_PATCH=%APP_VERSION_PATCH%' >> src\version.pri
