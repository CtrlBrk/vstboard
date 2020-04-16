#ifndef JSONWRITER_H
#define JSONWRITER_H

class MainHost;
class MainWindow;

class JsonWriter
{
public:
	JsonWriter() {}
	static bool writeProjectFile(QIODevice *device, MainHost *host, MainWindow *win, bool saveProject, bool saveSetup, bool binary);
    static QJsonObject writeProjectProcess(MainHost *host, bool saveProject, bool saveSetup);
	static QJsonObject writeProjectView(MainWindow *window, bool saveProject, bool saveSetup);

};

#endif // JSONWRITER_H
