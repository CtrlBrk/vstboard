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
#include "jsonwriter.h"
#include "mainhost.h"
#include "mainwindow.h"

bool JsonWriter::writeProjectFile(QIODevice *device, MainHost *host, MainWindow *window, bool saveProject, bool saveSetup, bool /*binary*/)
{
	bool no_error = true;

	QJsonObject jsonObj;
	jsonObj["proc"] = writeProjectProcess(host, saveProject, saveSetup);
	jsonObj["view"] = writeProjectView(window, saveProject, saveSetup);

	QJsonDocument saveDoc(jsonObj);

    // device->write(binary
    // 	? qCompress(saveDoc.toBinaryData())
    // 	: saveDoc.toJson(QJsonDocument::Indented)
    // );
    device->write(saveDoc.toJson(QJsonDocument::Indented));
	return no_error;
}

QJsonObject JsonWriter::writeProjectProcess(MainHost *host, bool saveProject, bool saveSetup)
{
	QJsonObject jsonObj;

    QJsonArray contArray;
    QJsonObject contObj;

    if(saveSetup) {
		host->hostContainer->SaveProgram();
		host->hostContainer->toJson(contObj);
        contArray.append(contObj);
    }
    if(saveProject) {
		host->projectContainer->SaveProgram();
		host->projectContainer->toJson(contObj);
        contArray.append(contObj);

		host->programContainer->SaveProgram();
		host->programContainer->toJson(contObj);
        contArray.append(contObj);

		host->groupContainer->SaveProgram();
		host->groupContainer->toJson(contObj);
        contArray.append(contObj);
    }

    jsonObj["containers"] = contArray;

    if(saveProject) {
        QJsonObject jProgs;
		host->programManager->toJson(jProgs);
        jsonObj["programs"] = jProgs;
    }

    return jsonObj;
}

QJsonObject JsonWriter::writeProjectView(MainWindow *window, bool saveProject, bool saveSetup)
{
	QJsonObject jsonObj;

	window->mySceneView->viewProject->SaveProgram();
	window->mySceneView->viewProgram->SaveProgram();
	window->mySceneView->viewGroup->SaveProgram();

	QJsonArray viewArray;
	QJsonObject jView;

	if (saveSetup) {
		jView["name"] = "viewHost";
		window->mySceneView->viewHost->toJson(jView);
		viewArray.append(jView);
	}

	if (saveProject) {
		jView["name"] = "viewProject";
		window->mySceneView->viewProject->toJson(jView);
		viewArray.append(jView);

		jView["name"] = "viewProgram";
		window->mySceneView->viewProgram->toJson(jView);
		viewArray.append(jView);

		jView["name"] = "viewGroup";
		window->mySceneView->viewGroup->toJson(jView);
		viewArray.append(jView);
	}

	jsonObj["contViews"] = viewArray;

	if (saveSetup) {
		QJsonObject viewCfg;
		window->viewConfig->toJson(viewCfg);
		jsonObj["viewCfg"] = viewCfg;
	}

	return jsonObj;
}
