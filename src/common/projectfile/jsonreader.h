#ifndef JSONREADER_H
#define JSONREADER_H

class MainHost;
class MainWindow;

class JsonReader
{
public:
	JsonReader() {}
    static bool readProjectFile(QIODevice *device, MainHost *host, MainWindow * window, bool binary);
	static void readProjectProcess(const QJsonObject &json, MainHost *host);
	static void readProjectView(const QJsonObject &json, MainWindow *window, MainHost *host);

private:
    MainHost *myHost;
};

#endif // JSONREADER_H
