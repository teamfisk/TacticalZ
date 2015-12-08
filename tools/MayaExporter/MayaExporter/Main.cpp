#include "MayaIncludes.h"
#include "Menu.h"
#include <iostream>
#include <maya/MFnPlugin.h>
using namespace std;
QDialog* dialog;
Menu* menu;

// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res))
	{
		CHECK_MSTATUS(res);
	}
	
	MGlobal::displayInfo("Maya plugin loaded!");

	dialog = new QDialog();
	dialog->setWindowTitle("Custom Exporter");

	menu = new Menu(dialog);

	dialog->resize(300, 200);
	dialog->show();

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	MGlobal::displayInfo("Maya plugin unloaded!");
	
	delete dialog;
	delete menu;
	return MS::kSuccess;
}
