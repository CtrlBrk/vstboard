#ifndef JSONREADER_H
#define JSONREADER_H

class MainHost;

class JsonReader
{
public:
    JsonReader(MainHost *host);
    bool readProjectFile(QIODevice *device, bool binary);

private:
    MainHost *myHost;
};

#endif // JSONREADER_H
