/**************************************************************************
#    Copyright 2010-2025 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
#include "jsonreader.h"
#include "mainhost.h"
#include "mainwindow.h"

bool JsonReader::readProjectFile(QIODevice *device, MainHost *host, MainWindow * window, bool /*binary*/)
{
	QByteArray saveData = device->readAll();

 //    QJsonDocument loadDoc(binary
 //        ? QJsonDocument::fromBinaryData(qUncompress(saveData))
 //        : QJsonDocument::fromJson(saveData)
    // );
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

	QJsonObject json = loadDoc.object();
	
    if (json.contains("proc") && host) {
		readProjectProcess(json["proc"].toObject(), host);
	}
    if (json.contains("view") && window) {
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
