#include "Menu.h"
#include <iostream>

using namespace std;

Menu::Menu()
{
}

Menu::Menu(QDialog* dialog)
{
	// Save the dialog pointer. Needed when the application gets destroyed
	m_DialogPointer = dialog;

	// Create QpushButtons & give them names
	m_ExportSelectedButton = new QPushButton("&Export Selected", this);
	m_BrowseButton = new QPushButton("&...", this);
	m_ExportAllButton = new QPushButton("&Export All", this);
	m_CancelButton = new QPushButton("&Cancel", this);
	m_AddClipsButton = new QPushButton("&Add Clips", this);
	m_RemoveClipsButton = new QPushButton("&Remove Latest Clip", this);

	// Option box and checkboxes
	QGroupBox *optionsBox = new QGroupBox(tr("Options"));

	m_ExportAnimationsButton = new QCheckBox(tr("&Export Animations"));
	m_CopyTexturesButton = new QCheckBox(tr("&Copy Textures"));
	m_Button3 = new QCheckBox(tr("Test Materials"));

	m_ExportAnimationsButton->setChecked(true);
	m_CopyTexturesButton->setChecked(true);
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(m_ExportAnimationsButton);
	vbox->addWidget(m_CopyTexturesButton);
	vbox->addWidget(m_Button3);
	vbox->addStretch(1);
	optionsBox->setLayout(vbox);

	// Connect the buttons with signals & functions
	connect(m_ExportSelectedButton, SIGNAL(clicked(bool)), this, SLOT(ExportSelected(bool)));
	connect(m_BrowseButton, SIGNAL(clicked(bool)), this, SLOT(ExportPathClicked(bool)));
	connect(m_ExportAllButton, SIGNAL(clicked(bool)), this, SLOT(ExportAll(bool)));
	connect(m_CancelButton, SIGNAL(clicked(bool)), this, SLOT(CancelClicked(bool)));
	connect(m_AddClipsButton, SIGNAL(clicked(bool)), this, SLOT(AddClipClicked(bool)));
	connect(m_RemoveClipsButton, SIGNAL(clicked(bool)), this, SLOT(RemoveClipClicked(bool)));

	connect(m_ExportAnimationsButton, SIGNAL(clicked(bool)), this, SLOT(Button1Clicked(bool)));
	connect(m_CopyTexturesButton, SIGNAL(clicked(bool)), this, SLOT(Button2Clicked(bool)));
	connect(m_Button3, SIGNAL(clicked(bool)), this, SLOT(Button3Clicked(bool)));

	// Creating several layouts, adding widgets & adding them to one layout in the end
	QHBoxLayout* topLayout = new QHBoxLayout;
	QVBoxLayout* midLayout = new QVBoxLayout;
	QHBoxLayout* botLayout = new QHBoxLayout;
	QVBoxLayout* baseLayout = new QVBoxLayout;
	QHBoxLayout* clipButtonLayout = new QHBoxLayout;
	QHBoxLayout* startEndLabelLayout = new QHBoxLayout;
	m_ClipLayout = new QVBoxLayout;



	m_ExportPath = new QLineEdit;
	m_FileDialog = new QFileDialog;

	QLabel* exportLabel = new QLabel;
	exportLabel->setText("Export Path:");
	QLabel* nameLabel = new QLabel;
	nameLabel->setText("Name:");
	QLabel* startLabel = new QLabel;
	startLabel->setText("Start:");
	QLabel* endLabel = new QLabel;
	endLabel->setText("End:");

	//exportLabel->setText("Export Path:");

	midLayout->addWidget(optionsBox);

	topLayout->addWidget(exportLabel);
	topLayout->addWidget(m_ExportPath);
	topLayout->addWidget(m_BrowseButton);

	botLayout->addWidget(m_ExportSelectedButton);
	botLayout->addWidget(m_ExportAllButton);
	botLayout->addWidget(m_CancelButton);

	startEndLabelLayout->addWidget(nameLabel);
	startEndLabelLayout->addWidget(startLabel);
	startEndLabelLayout->addWidget(endLabel);

	clipButtonLayout->addWidget(m_AddClipsButton);
	clipButtonLayout->addWidget(m_RemoveClipsButton);

	baseLayout->addLayout(topLayout);
	baseLayout->addLayout(midLayout);

	baseLayout->addSpacing(10);
	baseLayout->addLayout(botLayout);

	baseLayout->addSpacing(10);
	baseLayout->addLayout(clipButtonLayout);
	baseLayout->addLayout(startEndLabelLayout);
	baseLayout->addLayout(m_ClipLayout);
	baseLayout->addStretch();

	// Set the layout for our window
	dialog->setLayout(baseLayout);

	for (unsigned int i = 0; i < 3; i++) {
		this->AddClipClicked(true);
	}

}


void Menu::ExportSelected(bool checked)
{
	// Retrieving the objects we currently have selected
	MSelectionList selected;
	MGlobal::getActiveSelectionList(selected);

	// Loop through or list of selection(s)
	for (unsigned int i = 0; i < selected.length(); i++) {
		MObject object;
		selected.getDependNode(i, object);
		MFnDependencyNode thisNode(object);

		cout << thisNode.name().asChar() << endl;
		GetMeshData(object);
	}
	if (m_ExportPath->text().isEmpty()) {
		cout << "Please select a folder." << endl;
	}
	else {
		cout << m_ExportPath->text().toLocal8Bit().constData() << endl;
	}

	if (m_ExportAnimationsButton->isChecked()) {
		GetSkeletonData();
	}

}

void Menu::ExportPathClicked(bool)
{
	// Opens up a file dialog. Save/Changes the name in the exportPath
	m_FileDialog->setFileMode(QFileDialog::Directory);
	m_FileDialog->setOption(QFileDialog::ShowDirsOnly);
	QString fileName = m_FileDialog->getExistingDirectory(this, "Select", "/home", QFileDialog::ShowDirsOnly);
	m_ExportPath->setText(fileName);
}

void Menu::AddClipClicked(bool)
{
	QHBoxLayout* tempLayout = new QHBoxLayout;

	QLineEdit* nameLineEdit = new QLineEdit;
	QLineEdit* startLineEdit = new QLineEdit;
	QLineEdit* endLineEdit = new QLineEdit;

	m_AnimationClipName.push_back(nameLineEdit);
	m_StartFrameLines.push_back(startLineEdit);
	m_EndFrameLines.push_back(endLineEdit);

	tempLayout->addWidget(nameLineEdit);
	tempLayout->addWidget(startLineEdit);
	tempLayout->addWidget(endLineEdit);

	m_ClipLayout->addLayout(tempLayout);
	//m_ClipLayout->update();
	layouts.push_back(tempLayout);
}

void Menu::RemoveClipClicked(bool)
{
	if (m_StartFrameLines.size() > 0) {
		QLayoutItem* tempWidget;// = m_ClipLayout->itemAt(0);

		for (unsigned int i = 0; i < layouts.size(); i++) {
			while ((tempWidget = layouts[layouts.size() - 1]->takeAt(0)) != 0) {
				delete tempWidget->widget();
				delete tempWidget;
			}
		}

		m_ClipLayout->removeItem(tempWidget);
		m_ClipLayout->update();

		layouts.pop_back();
		m_StartFrameLines.pop_back();
		m_EndFrameLines.pop_back();
	}
}

void Menu::ExportAll(bool)
{
	MDagPath path;
    m_File.ASCIIFilePath("C:/Users/Nickelodion/Desktop/coolASCII.txt");
    m_File.binaryFilePath("C:/Users/Nickelodion/Desktop/coolSoptunz.bin");
    m_File.OpenFiles();

	// Loop through all nodes in the scene
	MItDependencyNodes it(MFn::kMesh);
	for (; !it.isDone(); it.next()) {
		MObject node = it.thisNode();
		if (node.hasFn(MFn::kMesh)) {
			MFnDependencyNode thisNode(node);
            MPlugArray connections;

            thisNode.findPlug("inMesh").connectedTo(connections, true, true);
            bool next = false;
            for (unsigned int i = 0; i < connections.length(); i++) {
                if(connections[i].node().apiType() == MFn::kSkinClusterFilter){ 
                    next = true;
                    break;
                }
            }
            if (next)
                continue;

            cout << thisNode.name().asChar() << endl;
            MGlobal::displayInfo("EXPORT ALL FUNCTION: " + thisNode.name() + " " + thisNode.findPlug("inMesh").asMObject().apiTypeStr());
            GetMeshData(node);
		}
	}
	if (m_ExportPath->text().isEmpty()) {
		cout << "Please select a folder." << endl;
	}
	else {
		cout << m_ExportPath->text().toLocal8Bit().constData() << endl;
	}

	if (m_ExportAnimationsButton->isChecked())
		GetSkeletonData();
    m_File.CloseFiles();
}

void Menu::CancelClicked(bool)
{
	m_DialogPointer->close();
}

void Menu::Button1Clicked(bool)
{
	if (m_ExportAnimationsButton->isChecked()) {
		MGlobal::displayInfo("1 checked!");
	}
	else {
		MGlobal::displayInfo("1 unchecked!");
	}
}

void Menu::Button2Clicked(bool)
{
	if (m_CopyTexturesButton->isChecked()) {
		cout << "2 checked!" << endl;
	}
	else {
		cout << "2 unchecked!" << endl;
	}
}

void Menu::Button3Clicked(bool)
{
	if (m_Button3->isChecked()) {
		cout << "3 checked!" << endl;
	}
	else {
		cout << "3 unchecked!" << endl;
	}
}
void Menu::GetMeshData(MObject object)
{
    std::vector<VertexLayout> vertexList;
    std::vector<unsigned int> indexList;

    Mesh mesh;
    mesh.GetMeshData(object, vertexList, indexList);

    for (auto aVertex : vertexList) {
        m_File.writeToFiles(&aVertex);
    }
    m_File.writeToFiles(indexList.data(), indexList.size());
}
void Menu::GetMaterialData()
{
	this->m_MaterialHandler = new Material();

	// Traverse scene and return vector with all materials
	std::vector<MaterialNode>* AllMaterials = m_MaterialHandler->DoIt();

	// Access the colorR component of one material (example)
	cout << AllMaterials->at(0).Color[0] << endl;
	MGlobal::displayInfo(MString() + AllMaterials->at(0).Color[0]);
}

void Menu::GetSkeletonData()
{
	if (MAnimControl::currentTime().unit() != MTime::kNTSCField) {

		MGlobal::displayError(MString() + "Please change to 60 FPS under Preferences/Settings!");
		return;
	}

	std::vector<BindPoseSkeletonNode> allBindPoses;
	std::vector<Animation> allAnimations;

	allBindPoses = m_SkeletonHandler->GetBindPoses();

	for (unsigned int j = 0; j < m_StartFrameLines.size(); j++) {
		if (m_StartFrameLines[j]->text().isEmpty() == true || m_StartFrameLines[j]->text().isEmpty() == true) {
			MGlobal::displayError(MString() + "Empty Animation Clip(s)");
			return;
		}

		int startFrame = m_StartFrameLines[j]->text().toInt();
		int  endFrame = m_EndFrameLines[j]->text().toInt();
		std::string animationName = m_AnimationClipName[j]->text().toAscii().constData();

		allAnimations.push_back(m_SkeletonHandler->GetAnimData(animationName, startFrame, endFrame));
	}

	//print out all bind poses
	for (auto aBindPose : allBindPoses){
		m_File.writeToFiles(&aBindPose);
		MGlobal::displayInfo(MString() + "BindPose Skeleton name: " + aBindPose.Name.c_str());

		for (int i = 0; i < aBindPose.Joints.size(); i++){
			//MGlobal::displayInfo(MString() + aBindPose.JointNames[i].c_str());
			//MGlobal::displayInfo(MString() + aBindPose.ParentIDs[i]);
			//MGlobal::displayInfo(MString() + aBindPose.Joints[i].Translation[0] + " " + aBindPose.Joints[i].Translation[1] + " " + aBindPose.Joints[i].Translation[2]);
			//MGlobal::displayInfo(MString() + aBindPose.Joints[i].Rotation[0] + " " + aBindPose.Joints[i].Rotation[1] + " " + aBindPose.Joints[i].Rotation[2]);
			//MGlobal::displayInfo(MString() + aBindPose.Joints[i].Scale[0] + " " + aBindPose.Joints[i].Scale[1] + " " + aBindPose.Joints[i].Scale[2]);
		}
	}
	//print out all animations
	for (auto aAnimation : allAnimations) {
		m_File.writeToFiles(&aAnimation);
	}
}

Menu::~Menu()
{
	//delete exportSelectedButton;
	//delete browseButton;
	//delete exportPath;
	//delete fileDialog;
	m_FileDialog->~QFileDialog();

	//delete MaterialHandler;
}