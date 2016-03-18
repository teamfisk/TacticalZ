// UD1414_Plugin.cpp : Defines the exported functions for the DLL application.
#include "zmq.hpp"
#include "maya_includes.h"
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <maya/MFnPlugin.h>

#pragma comment(lib, "libzmq-v120-mt-4_0_4.lib")

using namespace std;

MCallbackIdArray IDArray;
map <string, vector <MObject>> textureObject;
//map <MObject, string> compareObject;
vector<MObject> compareObject_key;
vector<string> compareObject_value;
struct message {
	int docNameLength;
	//int numPixels;
	//int startX;
	//int startY;
	//int canvasY;
	//int canvasX;
	//int rowLength;
	char* docName;
	//unsigned char* pixels;
};

typedef struct
{
	bool shutDown = false;
	bool hasShutdown = false;
} ThreadVars;

ThreadVars threadVars;
vector<MImage> images;

map <string, MObject> docName_map;
list<string> toUpdate;
MThreadRetVal ThreadFunction(void* data)
{
	zmq::context_t context(1);
	zmq::socket_t subscriber(context, ZMQ_SUB);

	std::cout << "Connecting to hello world server..." << std::endl;
	subscriber.connect("tcp://localhost:5555");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	zmq::message_t msg;
	while (!threadVars.shutDown) 
	{
		if (subscriber.recv(&msg, ZMQ_DONTWAIT))
		{
			//MGlobal::displayInfo(MString() + "test1: " + msg.size());

			MObject outColorObject;
			MPlug outColorPlug;

			size_t tet = msg.size();
			message PSData;
			PSData.docNameLength = ((message*)msg.data())->docNameLength;
			//PSData.numPixels = ((message*)msg.data())->numPixels;
			//PSData.startX = ((message*)msg.data())->startX;
			//PSData.startY = ((message*)msg.data())->startY;
			//PSData.canvasY = ((message*)msg.data())->canvasY;
			//PSData.canvasX = ((message*)msg.data())->canvasX;
			//PSData.rowLength = ((message*)msg.data())->rowLength;
			PSData.docName = (char*)msg.data() + sizeof(message) - sizeof(char*) - 4;
			//PSData.pixels = (unsigned char*)msg.data() + sizeof(message) - sizeof(char*) * 2 + 8 + PSData.docNameLength;

			//MGlobal::displayInfo(MString() + "docName: " + PSData.docName);
			//MGlobal::displayInfo(MString() + "docNameLength: " + PSData.docNameLength);

			if (textureObject.find(PSData.docName) != textureObject.end())
			{
				toUpdate.push_back(PSData.docName);
				//toUpdate.unique();
			/*	vector<MObject>& textureNode = textureObject[PSData.docName];
				vector<MObject>::iterator it;
				for (it = textureNode.begin(); it != textureNode.end(); it++)
				{
					MFnDependencyNode depNode(*it);
					MPlug plug = depNode.findPlug("outColor");
					MGlobal::displayInfo(MString() + "ANKA: " + plug.name());
					plug.setMObject(plug.asMObject());
				}*/
			}

			//std::map<string, MObject>::iterator it;
			//it = docName_map.find(PSData.docName);
			//if (! (it != docName_map.end()))
			//{
			//	MStatus res = MS::kSuccess;
			//	MString file;
			//	MString	place2dTexture;

			//	do
			//	{
			//		res = MGlobal::executeCommand(MString() + "shadingNode - asUtility place2dTexture;", place2dTexture);
			//	} while (res != MS::kSuccess);

			//	do
			//	{
			//		res = MGlobal::executeCommand(MString() + "shadingNode - asTexture - isColorManaged file;", file);
			//	} while (res != MS::kSuccess);

			//	MGlobal::displayInfo(MString() + "ASDASDASD: " + file);
			//	MGlobal::displayInfo(MString() + "ASDASDASD: " + place2dTexture);
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".coverage " + file + ".coverage;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".translateFrame " + file + ".translateFrame;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".rotateFrame " + file + ".rotateFrame;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".mirrorU " + file + ".mirrorU;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".mirrorV " + file + ".mirrorV;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".stagger " + file + ".stagger;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".wrapU " + file + ".wrapU;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".wrapV " + file + ".wrapV;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".repeatUV " + file + ".repeatUV;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".offset " + file + ".offset;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".rotateUV " + file + ".rotateUV;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".noiseUV " + file + ".noiseUV;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".vertexUvOne " + file + ".vertexUvOne;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".vertexUvTwo " + file + ".vertexUvTwo;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".vertexUvThree " + file + ".vertexUvThree;");
			//	MGlobal::executeCommand("connectAttr - f " + place2dTexture + ".vertexCameraOne " + file + ".vertexCameraOne;");
			//	MGlobal::executeCommand("connectAttr " + place2dTexture + ".outUvFilterSize " + file + ".uvFilterSize;");
			//	MGlobal::executeCommand("connectAttr " + place2dTexture + ".outUV " + file + ".uv;");
			//	MSelectionList thisFile;
			//	MObject thisFileObject;

			//	for (map <string, vector <MObject>>::iterator it = textureObject.begin(); it != textureObject.end(); ++it)
			//	{
			//		//MGlobal::displayInfo(MString() + "SECOND: " + it->second[0].apiTypeStr());
			//		//MFnDependencyNode tmp_DN(it->second[0]);
			//		//MPlug tmpPlug = tmp_DN.findPlug("outColor");
			//		//tmpPlug.setMObject(tmpPlug.asMObject());
			//		//MGlobal::displayInfo(MString() + "tmp_DN: " + tmp_DN.name());
			//	}



			//	MGlobal::getSelectionListByName(file, thisFile);
			//	thisFile.getDependNode(0, thisFileObject);

			//	docName_map[PSData.docName] = thisFileObject;
			//	MFnDependencyNode textureNode(thisFileObject);
			//	MImage image;
			//	image.readFromTextureNode(thisFileObject);
			//	
			//	unsigned int w, h;
			//	image.getSize(w, h);
			//	/*MGlobal::displayInfo(MString() + w + " " + h);
			//	for (unsigned int i = 0; i < w * h * 4; i++)
			//	{
			//		MGlobal::displayInfo(MString() + "Pix: " + imagePixels[i]);
			//	}*/
			//	//image.setPixels(PSData.pixels, PSData.canvasX, PSData.canvasY);
			//	image.setRGBA(true);
			//	image.convertPixelFormat(MImage::MPixelType::kByte);
			//	unsigned char* imagePixels = image.pixels();

			//	MPlug color = textureNode.findPlug("uvCoord");
			//	//color.setMObject(color.asMObject());
			//	
			//	//unsigned int w, h;
			//	image.getSize(w, h);
			//	MGlobal::displayInfo(MString() + w + " " + h);
			//	for (unsigned int i = 0; i < w * h * 4; i++)
			//	{
			//		MGlobal::displayInfo(MString() + "Pix: " + imagePixels[i]);
			//	}
			//	MGlobal::displayInfo("999999999999");

			//	//image.writeToFile("C:/Users/kamisama/Desktop/3.png", "png");

			//	color.setMObject(color.asMObject());
			//}
			//else
			//{
			//	MObject& thisFileObject = docName_map[PSData.docName];
			//	MFnDependencyNode textureNode(thisFileObject);
			//	
			//	MImage image;
			//	image.readFromTextureNode(thisFileObject);
			//	unsigned char* imagePixels = image.pixels();

			//	unsigned int w, h;
			//	image.getSize(w, h);
			//	MGlobal::displayInfo("Before");
			//	MGlobal::displayInfo(MString() + w + " " + h);
			//	for (unsigned int i = 0; i < w * h * 4; i++)
			//	{
			//		//MGlobal::displayInfo(MString() + "Pix: " + imagePixels[i]);
			//	}
			//	//image.setPixels(PSData.pixels, PSData.canvasX, PSData.canvasY);
			//	//memcpy(image.pixels(), PSData.pixels, w * h * 4);
			//	//MPlug color = textureNode.findPlug("outColor");

			//	image.getSize(w, h);
			//	MGlobal::displayInfo("After");
			//	MGlobal::displayInfo(MString() + w + " " + h);
			//	for (unsigned int i = 0; i < w * h * 4; i++)
			//	{
			//		//MGlobal::displayInfo(MString() + "Pix: " + imagePixels[i]);
			//	}
			//	
			//	MPlug color = textureNode.findPlug("uvCoord");
			//	color.setMObject(color.asMObject());
			//}
			//MItDependencyNodes it(MFn::kFileTexture);
			//for (; !it.isDone(); it.next())
			//{
			//	MFnDependencyNode texture_node(it.thisNode());
			//	globalShitPlug = texture_node.findPlug("outColor");
			//	globalShit = globalShitPlug.asMObject();
			//	MGlobal::displayInfo(MString() + "Plug name: " + globalShitPlug.name());
			//	MImage test_Mimage;
			//	MGlobal::displayInfo(MString() + "hej");
			//	test_Mimage.create(1, 1, 4, MImage::kByte);
			//	//test_Mimage.readFromTextureNode(globalShit);
			//	MGlobal::displayInfo(MString() + "hej igen");
			//	unsigned char* test_pixels;
			//	test_pixels = test_Mimage.pixels();

			//	unsigned int width, height;
			//	test_Mimage.getSize(width, height);
			//	MGlobal::displayInfo(MString() + "WIDTH: " + width);
			//	MGlobal::displayInfo(MString() + "HEIGHT: " + height);
			//	for (unsigned int i = 0; i < width * height * 4; i++)
			//	{
			//		test_pixels[i] = imageData->pixels[i];
			//	}

			//	globalShitPlug.setMObject(globalShit);

			//	break;
			//}
		}
	}

	threadVars.hasShutdown = true;
	return 0;
}

void textureChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData)
{
	//MGlobal::displayInfo(MString() + "Hello!!!!!!!!!!!! " + otherPlug.name());
	//MGlobal::displayInfo(MString() + "Hello! :DDDDDDD " + otherPlug.asString());
	string plugName(plug.name().asChar());
	if (plugName.find("fileTextureName") != string::npos)
	{
		if (msg & MNodeMessage::AttributeMessage::kAttributeSet)
		{
			//MGlobal::displayInfo("Hello from file name! :P");
			
			map <string, vector <MObject>>::iterator it;
			vector<MObject>::iterator its;
			vector<string>::iterator itVec_string;
			bool newName = false;
			bool nameIsBiggerThenZero = false;
			int index = 0;
			for (its = compareObject_key.begin(); its != compareObject_key.end(); its++)
			{
				//MGlobal::displayInfo("Cool");
				if (*its == plug.node())
				{
					if (compareObject_value[index].compare(plug.asString().asChar()) != 0)
					{
						//MGlobal::displayInfo("Cool 1");
						newName = true;
						if (compareObject_value[index].length() > 0)
							//MGlobal::displayInfo("Cool 2");
							nameIsBiggerThenZero = true;
						break;
					}
				}
				index++;
			}
			if (its == compareObject_key.end())
			{
				newName = true;
			}

			if (newName == true)
			{
				MGlobal::displayInfo("New Name");
				if (nameIsBiggerThenZero)
				{
					vector<MObject>::iterator iii;
					vector<MObject>& objectVector = textureObject[compareObject_value[index]];
					index = 0;
					for (iii = objectVector.begin(); iii != objectVector.end(); iii++)
					{
						//MGlobal::displayInfo(MString() + "INSIDE THE FOR LOOP!!!!!! III " + (*iii).apiTypeStr());
						if ((*iii) == plug.node())
						{
							MGlobal::displayInfo("iii == plug.node()");
							break;
						}
						index++;
					}
					//MGlobal::displayInfo("erase");

					objectVector.erase(objectVector.begin() + index);
					textureObject[plug.asString().asChar()].push_back(plug.node());
					//MGlobal::displayInfo(MString() + "index: " + index + " " + objectVector.size());
				}

				index = 0;
				for (its = compareObject_key.begin(); its != compareObject_key.end(); its++)
				{
					if (*its == plug.node())
					{
						MGlobal::displayInfo("its->first == plug.node()");
						compareObject_value[index] = plug.asString().asChar();
						break;
					}
					index++;
				}
				if (its == compareObject_key.end())
				{
					//MGlobal::displayInfo("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
					compareObject_key.push_back(plug.node());
					compareObject_value.push_back(plug.asString().asChar());
					textureObject[plug.asString().asChar()].push_back(plug.node());
				}
		//		//compareObject[plug.node()] = plug.asString().asChar();
					
			}
			
			//MGlobal::displayInfo("Here");
			for (its = compareObject_key.begin(); its != compareObject_key.end(); its++)
			{
				//MGlobal::displayInfo(MString() + "compareObject_key: " + its->apiTypeStr());
			}

			for (itVec_string = compareObject_value.begin(); itVec_string != compareObject_value.end(); itVec_string++)
			{
				//MGlobal::displayInfo(MString() + "compareObject_value: " + (*itVec_string).c_str());
			}

			MGlobal::displayInfo("--- textureObject ---");

			for (it = textureObject.begin(); it != textureObject.end(); it++)
			{
				MGlobal::displayInfo(MString() + "filename: " + it->first.c_str());
				MGlobal::displayInfo(MString() + "nr of nodes: " + it->second.size());
			}

		}
	}
}

void nodeCreated(MObject& node, void *clientData)
{
	if (node.hasFn(MFn::kFileTexture))
	{
		MGlobal::displayInfo("FileTextureAdded");
		IDArray.append(MNodeMessage::addAttributeChangedCallback(node, textureChanged));
	}
}

void timer(float elapsedTime, float lastTime, void *clientData) 
{
	int size = toUpdate.size();
	while(size)
	{
		vector<MObject>& textureNode = textureObject[toUpdate.front()];
		vector<MObject>::iterator it;
		for (it = textureNode.begin(); it != textureNode.end(); it++)
		{
			MFnDependencyNode depNode(*it);
			MPlug plug = depNode.findPlug("outColor");
			//MGlobal::displayInfo(MString() + "ANKA: " + plug.name());
			plug.setMObject(plug.asMObject());
		}
		toUpdate.pop_front();
		size = toUpdate.size();
	}
}

EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus res = MS::kSuccess;
	
	threadVars.shutDown = false;
	threadVars.hasShutdown = false;
	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}
	IDArray.append(MDGMessage::addNodeAddedCallback(nodeCreated));
	IDArray.append(MTimerMessage::addTimerCallback(0.2, timer));

	res = MThreadAsync::init();
	if (res == MStatus::kSuccess)
	{
		res = MThreadAsync::createTask(ThreadFunction, nullptr, nullptr, NULL);

		if (res != MStatus::kSuccess)
		{
			threadVars.hasShutdown = true;
			return MStatus::kFailure;
		}
	}

	MGlobal::displayInfo("Maya plugin loaded!");
	return res;
}


EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);

	threadVars.shutDown = true;
	//Wait for thread to shutdown befor releasing it
	while (!threadVars.hasShutdown)
	{
		Sleep(1);
	}
	MThreadAsync::release();
	MMessage::removeCallbacks(IDArray);

	MGlobal::displayInfo("Maya plugin unloaded!");

	return MS::kSuccess;
}