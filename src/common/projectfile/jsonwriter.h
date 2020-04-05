#ifndef JSONWRITER_H
#define JSONWRITER_H

class MainHost;

class JsonWriter
{
public:
    JsonWriter(const MainHost *host);
    bool writeProjectFile(QIODevice *device, bool saveProject, bool saveSetup, bool binary);

private:
    const MainHost *myHost;
};

#endif // JSONWRITER_H
