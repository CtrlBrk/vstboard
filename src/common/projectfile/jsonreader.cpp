#include "jsonreader.h"
#include "mainhost.h"
#include "mainwindow.h"

JsonReader::JsonReader(MainHost *host) :
    myHost(host)
{

}


bool JsonReader::readProjectFile(QIODevice *device)
{
    QByteArray saveData = device->readAll();

//    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData) );
    QJsonDocument loadDoc( QJsonDocument::fromBinaryData( qUncompress(saveData) ) );
    QJsonObject json = loadDoc.object();

    if (json.contains("containers") && json["containers"].isArray()) {
        QJsonArray contArray = json["containers"].toArray();
        for (int i = 0; i < contArray.size(); ++i) {
            QJsonObject contObj = contArray[i].toObject();

            QString cntName;
            if (contObj.contains("objInfo")) {
                QJsonObject jInfo = contObj["objInfo"].toObject();
                cntName = jInfo["name"].toString();
            }

            QSharedPointer<Connectables::Container> cnt=0;

            if(cntName == "hostContainer") {
                myHost->SetupHostContainer();
                cnt = myHost->hostContainer;
            }
            if(cntName == "projectContainer") {
                myHost->SetupProjectContainer();
                cnt = myHost->projectContainer;
            }
            if(cntName == "programContainer") {
                myHost->SetupProgramContainer();
                cnt = myHost->programContainer;
            }
            if(cntName == "groupContainer") {
                myHost->SetupGroupContainer();
                cnt = myHost->groupContainer;
            }

            if(cnt) {
                cnt->fromJson(contObj);
            }
        }
    }
    if(json.contains("programs")) {
            QJsonObject jProgs = json["programs"].toObject();
            myHost->programManager->fromJson(jProgs);
    }

    if (json.contains("contViews") && json["contViews"].isArray()) {
        if(myHost->mainWindow) {
            QJsonArray viewArray = json["contViews"].toArray();
            for (int i = 0; i < viewArray.size(); ++i) {
                QJsonObject contView = viewArray[i].toObject();

                if(contView["name"] == "viewHost") {
                    myHost->mainWindow->mySceneView->viewHost->fromJson(contView);
                    myHost->mainWindow->mySceneView->viewHost->SetViewProgram( myHost->hostContainer->GetProgramToSet() );
                }
                if(contView["name"] == "viewProject") {
                    myHost->mainWindow->mySceneView->viewProject->fromJson(contView);
                    myHost->mainWindow->mySceneView->viewProject->SetViewProgram( myHost->projectContainer->GetProgramToSet() );
                }
                if(contView["name"] == "viewProgram") {
                    myHost->mainWindow->mySceneView->viewProgram->fromJson(contView);
                    myHost->mainWindow->mySceneView->viewProgram->SetViewProgram( myHost->programContainer->GetProgramToSet() );
                }
                if(contView["name"] == "viewGroup") {
                    myHost->mainWindow->mySceneView->viewGroup->fromJson(contView);
                    myHost->mainWindow->mySceneView->viewGroup->SetViewProgram( myHost->groupContainer->GetProgramToSet() );
                }
            }
        }
    }

    if(json.contains("viewCfg")) {
        QJsonObject viewCfg = json["viewCfg"].toObject();
        myHost->mainWindow->viewConfig->fromJson(viewCfg);
    }

//   if(json.contains("colors")) {

//   }
    return true;
}
