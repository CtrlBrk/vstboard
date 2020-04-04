#ifndef JSONREADER_H
#define JSONREADER_H

class MainHost;

class JsonReader
{
public:
    JsonReader(MainHost *host);
    bool readProjectFile(QIODevice *device);

private:
    MainHost *myHost;
};

#endif // JSONREADER_H
