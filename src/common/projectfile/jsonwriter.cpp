#include "jsonwriter.h"
#include "mainhost.h"
#include "mainwindow.h"

JsonWriter::JsonWriter(const MainHost *host) :
    myHost(host)
{

}

bool JsonWriter::writeProjectFile(QIODevice *device)
{
    QJsonObject jsonObj;

    QJsonArray contArray;
    QJsonObject contObj;

    //only in setup files
//    myHost->hostContainer->SaveProgram();
//    myHost->hostContainer->toJson(contObj);
//    contArray.append(contObj);

    myHost->projectContainer->SaveProgram();
    myHost->projectContainer->toJson(contObj);
    contArray.append(contObj);

    myHost->programContainer->SaveProgram();
    myHost->programContainer->toJson(contObj);
    contArray.append(contObj);

    myHost->groupContainer->SaveProgram();
    myHost->groupContainer->toJson(contObj);
    contArray.append(contObj);

    jsonObj["containers"] = contArray;

    QJsonObject jProgs;
    myHost->programManager->toJson(jProgs);
    jsonObj["programs"] = jProgs;

    if(myHost->mainWindow) {
        myHost->mainWindow->mySceneView->viewProject->SaveProgram();
        myHost->mainWindow->mySceneView->viewProgram->SaveProgram();
        myHost->mainWindow->mySceneView->viewGroup->SaveProgram();

        QJsonArray viewArray;
        QJsonObject jView;

        //only in setup files
//        jView["name"] = "viewHost";
//        myHost->mainWindow->mySceneView->viewHost->toJson(jView);
//        viewArray.append(jView);

        jView["name"] = "viewProject";
        myHost->mainWindow->mySceneView->viewProject->toJson(jView);
        viewArray.append(jView);

        jView["name"] = "viewProgram";
        myHost->mainWindow->mySceneView->viewProgram->toJson(jView);
        viewArray.append(jView);

        jView["name"] = "viewGroup";
        myHost->mainWindow->mySceneView->viewGroup->toJson(jView);
        viewArray.append(jView);

        jsonObj["contViews"] = viewArray;
    }

    //only in setup files
//    QJsonObject viewCfg;
//    myHost->mainWindow->viewConfig->toJson(viewCfg);
//    jsonObj["viewCfg"] = viewCfg;

    QJsonDocument saveDoc(jsonObj);

//    device->write(saveDoc.toJson(QJsonDocument::Compact));
//    device->write(saveDoc.toJson(QJsonDocument::Indented));
    device->write( qCompress(saveDoc.toBinaryData()) );
    return true;
}
