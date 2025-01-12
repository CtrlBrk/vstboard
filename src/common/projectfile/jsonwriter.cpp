#include "jsonwriter.h"
#include "mainhost.h"
#include "mainwindow.h"

bool JsonWriter::writeProjectFile(QIODevice *device, MainHost *host, MainWindow *window, bool saveProject, bool saveSetup, bool binary)
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
