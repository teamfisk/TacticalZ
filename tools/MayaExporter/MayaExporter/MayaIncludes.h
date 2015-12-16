#ifndef MAYAINCLUDES_H
#define MAYAINCLUDES_H

#define NT_PLUGIN
#define REQUIRE_IOSTREAM
#define EXPORT __declspec(dllexport)


#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MPoint.h>
#include <maya/MMatrix.h>
#include <maya/MEulerRotation.h>
#include <maya/MVector.h>
#include <maya/MItDag.h>
#include <maya/MFnCamera.h>
#include <maya/M3dView.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MImage.h>
#include <maya/MFnPointLight.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MCommandMessage.h>
#include <maya/MPlug.h>
#include <maya/MAnimControl.h>
#include <maya/MTime.h>


// Wrappers
#include <maya/MGlobal.h>
#include <maya/MCallbackIdArray.h>

#include <maya/MQtUtil.h>


// Messages
#include <maya/MMessage.h>
#include <maya/MTimerMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MEventMessage.h>
#include <maya/MPolyMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDagPath.h>
#include <maya/MDagMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MModelMessage.h>

// Commands
#include <maya/MPxCommand.h>

// Libraries to link from Maya
#pragma comment(lib,"Foundation.lib")
#pragma comment(lib,"OpenMaya.lib")
#pragma comment(lib,"OpenMayaUI.lib")
#pragma comment (lib, "OpenMayaAnim.lib")

#endif