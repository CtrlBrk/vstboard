#include "jsonwriter.h"
#include "mainhost.h"
#include "mainwindow.h"

JsonWriter::JsonWriter(const MainHost *host) :
    myHost(host)
{

}

bool JsonWriter::writeProjectFile(QIODevice *device, bool saveProject, bool saveSetup, bool binary)
{
    QJsonObject jsonObj;

    QJsonArray contArray;
    QJsonObject contObj;

    if(saveSetup) {
        myHost->hostContainer->SaveProgram();
        myHost->hostContainer->toJson(contObj);
        contArray.append(contObj);
    }
    if(saveProject) {
        myHost->projectContainer->SaveProgram();
        myHost->projectContainer->toJson(contObj);
        contArray.append(contObj);

        myHost->programContainer->SaveProgram();
        myHost->programContainer->toJson(contObj);
        contArray.append(contObj);

        myHost->groupContainer->SaveProgram();
        myHost->groupContainer->toJson(contObj);
        contArray.append(contObj);
    }

    jsonObj["containers"] = contArray;

    if(saveProject) {
        QJsonObject jProgs;
        myHost->programManager->toJson(jProgs);
        jsonObj["programs"] = jProgs;
    }

    if(myHost->mainWindow) {
        myHost->mainWindow->mySceneView->viewProject->SaveProgram();
        myHost->mainWindow->mySceneView->viewProgram->SaveProgram();
        myHost->mainWindow->mySceneView->viewGroup->SaveProgram();

        QJsonArray viewArray;
        QJsonObject jView;

        if(saveSetup) {
            jView["name"] = "viewHost";
            myHost->mainWindow->mySceneView->viewHost->toJson(jView);
            viewArray.append(jView);
        }

        if(saveProject) {
            jView["name"] = "viewProject";
            myHost->mainWindow->mySceneView->viewProject->toJson(jView);
            viewArray.append(jView);

            jView["name"] = "viewProgram";
            myHost->mainWindow->mySceneView->viewProgram->toJson(jView);
            viewArray.append(jView);

            jView["name"] = "viewGroup";
            myHost->mainWindow->mySceneView->viewGroup->toJson(jView);
            viewArray.append(jView);
        }

        jsonObj["contViews"] = viewArray;
    }

    if(saveSetup) {
        QJsonObject viewCfg;
        myHost->mainWindow->viewConfig->toJson(viewCfg);
        jsonObj["viewCfg"] = viewCfg;
    }

    QJsonDocument saveDoc(jsonObj);

    device->write( binary
        ? qCompress(saveDoc.toBinaryData())
        : saveDoc.toJson(QJsonDocument::Indented)
    );

    return true;
}
