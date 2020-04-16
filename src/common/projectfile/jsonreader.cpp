#include "jsonreader.h"
#include "mainhost.h"
#include "mainwindow.h"

bool JsonReader::readProjectFile(QIODevice *device, MainHost *host, MainWindow * window, bool binary)
{
	QByteArray saveData = device->readAll();

	QJsonDocument loadDoc(binary
		? QJsonDocument::fromBinaryData(qUncompress(saveData))
		: QJsonDocument::fromJson(saveData)
	);

	QJsonObject json = loadDoc.object();
	
	if (json.contains("proc")) {
		readProjectProcess(json["proc"].toObject(), host);
	}
	if (json.contains("view")) {
		readProjectView(json["view"].toObject(), window, host);
	}

	return true;
}

void JsonReader::readProjectProcess(const QJsonObject &json, MainHost *host)
{
	if (json.contains("containers") && json["containers"].isArray()) {
		QJsonArray contArray = json["containers"].toArray();
		for (int i = 0; i < contArray.size(); ++i) {
			QJsonObject contObj = contArray[i].toObject();

			QString cntName;
			if (contObj.contains("objInfo")) {
				QJsonObject jInfo = contObj["objInfo"].toObject();
				cntName = jInfo["name"].toString();
			}

			QSharedPointer<Connectables::Container> cnt = 0;

			if (cntName == "hostContainer") {
				host->SetupHostContainer();
				cnt = host->hostContainer;
			}
			if (cntName == "projectContainer") {
				host->SetupProjectContainer();
				cnt = host->projectContainer;
			}
			if (cntName == "programContainer") {
				host->SetupProgramContainer();
				cnt = host->programContainer;
			}
			if (cntName == "groupContainer") {
				host->SetupGroupContainer();
				cnt = host->groupContainer;
			}

			if (cnt) {
				cnt->fromJson(contObj);
			}
		}
	}
	if (json.contains("programs")) {
		QJsonObject jProgs = json["programs"].toObject();
		host->programManager->fromJson(jProgs);
	}

}

void JsonReader::readProjectView(const QJsonObject &json, MainWindow *window, MainHost *host)
{
	if (json.contains("contViews") && json["contViews"].isArray()) {
		QJsonArray viewArray = json["contViews"].toArray();
		for (int i = 0; i < viewArray.size(); ++i) {
			QJsonObject contView = viewArray[i].toObject();

			if (contView["name"] == "viewHost") {
				window->mySceneView->viewHost->fromJson(contView);
				if(host)
					window->mySceneView->viewHost->SetViewProgram(host->hostContainer->GetProgramToSet());
			}
			if (contView["name"] == "viewProject") {
				window->mySceneView->viewProject->fromJson(contView);
				if(host)
					window->mySceneView->viewProject->SetViewProgram(host->projectContainer->GetProgramToSet());
			}
			if (contView["name"] == "viewProgram") {
				window->mySceneView->viewProgram->fromJson(contView);
				if (host)
					window->mySceneView->viewProgram->SetViewProgram(host->programContainer->GetProgramToSet());
			}
			if (contView["name"] == "viewGroup") {
				window->mySceneView->viewGroup->fromJson(contView);
				if (host)
					window->mySceneView->viewGroup->SetViewProgram(host->groupContainer->GetProgramToSet());
			}
		}
	}

	if (json.contains("viewCfg")) {
		QJsonObject viewCfg = json["viewCfg"].toObject();
		window->viewConfig->fromJson(viewCfg);
	}
}
