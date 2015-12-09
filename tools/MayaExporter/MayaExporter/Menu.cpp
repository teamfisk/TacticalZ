#include "Menu.h"
#include <iostream>

using namespace std;

Menu::Menu()
{
}

Menu::Menu(QDialog* dialog)
{
	// Save the dialog pointer. Needed when the application gets destroyed
	dialogPointer = dialog;

	// Create QpushButtons & give them names
	exportSelectedButton = new QPushButton("&Export Selected", this);
	browseButton = new QPushButton("&...", this);
	exportAllButton = new QPushButton("&Export All", this);
	cancelButton = new QPushButton("&Cancel", this);

	// Option box and checkboxes
	QGroupBox *optionsBox = new QGroupBox(tr("Options"));

	exportAnimationsButton = new QCheckBox(tr("&Export Animations"));
	copyTexturesButton = new QCheckBox(tr("&Copy Textures"));
	button3 = new QCheckBox(tr("option3"));

	exportAnimationsButton->setChecked(true);
	copyTexturesButton->setChecked(true);
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(exportAnimationsButton);
	vbox->addWidget(copyTexturesButton);
	vbox->addWidget(button3);
	vbox->addStretch(1);
	optionsBox->setLayout(vbox);

	// Connect the buttons with signals & functions
	connect(exportSelectedButton, SIGNAL(clicked(bool)), this, SLOT(ExportSelected(bool)));
	connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(ExportPathClicked(bool)));
	connect(exportAllButton, SIGNAL(clicked(bool)), this, SLOT(ExportAll(bool)));
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(CancelClicked(bool)));

	connect(exportAnimationsButton, SIGNAL(clicked(bool)), this, SLOT(Button1Clicked(bool)));
	connect(copyTexturesButton, SIGNAL(clicked(bool)), this, SLOT(Button2Clicked(bool)));
	connect(button3, SIGNAL(clicked(bool)), this, SLOT(Button3Clicked(bool)));

	// Creating several layouts, adding widgets & adding them to one layout in the end
	QHBoxLayout* topLayout = new QHBoxLayout;
	QVBoxLayout* midLayout = new QVBoxLayout;
	QHBoxLayout* botLayout = new QHBoxLayout;
	QVBoxLayout* baseLayout = new QVBoxLayout;

	exportPath = new QLineEdit;
	fileDialog = new QFileDialog;

	QLabel* exportLabel = new QLabel;
	exportLabel->setText("Export Path:");
	
	midLayout->addWidget(optionsBox);

	topLayout->addWidget(exportLabel);
	topLayout->addWidget(exportPath);
	topLayout->addWidget(browseButton);

	botLayout->addWidget(exportSelectedButton);
	botLayout->addWidget(exportAllButton);
	botLayout->addWidget(cancelButton);

	baseLayout->addLayout(topLayout);
	baseLayout->addLayout(midLayout);

	baseLayout->addSpacing(10);
	
	baseLayout->addLayout(botLayout);
	baseLayout->addStretch();

	// Set the layout for our window
	dialog->setLayout(baseLayout);

}

void Menu::ExportSelected(bool checked)
{
	// Retrieving the objects we currently have selected
	MSelectionList selected;
	MGlobal::getActiveSelectionList(selected);

	// Loop through or list of selection(s)
	for (unsigned int i = 0; i < selected.length();i++)
	{
		MObject object;
		selected.getDependNode(i, object);
		MFnDependencyNode thisNode(object);

		cout << thisNode.name().asChar() << endl;
		GetMeshData(object);
	}
	if (exportPath->text().isEmpty())
		cout << "Please select a folder." << endl;
	else
		cout << exportPath->text().toLocal8Bit().constData() << endl;
}

void Menu::ExportPathClicked(bool)
{
	// Opens up a file dialog. Save/Changes the name in the exportPath
	fileDialog->setFileMode(QFileDialog::Directory);
	fileDialog->setOption(QFileDialog::ShowDirsOnly);
	QString fileName = fileDialog->getExistingDirectory(this, "Select", "/home", QFileDialog::ShowDirsOnly);
	exportPath->setText(fileName);
}

void Menu::ExportAll(bool)
{
	MDagPath path;
	
	// Loop through all nodes in the scene
	MItDependencyNodes it(MFn::kInvalid);
	for (;!it.isDone();it.next())
	{
		MObject node = it.thisNode();
		if (node.hasFn(MFn::kMesh))
		{
			MFnDependencyNode thisNode(node);

			cout << thisNode.name().asChar() << endl;
			GetMeshData(node);
		}
	}
	if (exportPath->text().isEmpty())
		cout << "Please select a folder." << endl;
	else
		cout << exportPath->text().toLocal8Bit().constData() << endl;
}

void Menu::CancelClicked(bool)
{
	dialogPointer->close();
}

void Menu::Button1Clicked(bool)
{
	if(exportAnimationsButton->isChecked())
		MGlobal::displayInfo("1 checked!");
	else
		MGlobal::displayInfo("1 unchecked!");
}

void Menu::Button2Clicked(bool)
{
	if (copyTexturesButton->isChecked())
		cout << "2 checked!" << endl;
	else
		cout << "2 unchecked!" << endl;
}

void Menu::Button3Clicked(bool)
{
	if (button3->isChecked())
		cout << "3 checked!" << endl;
	else
		cout << "3 unchecked!" << endl;
}

void Menu::GetMeshData(MObject object)
{
	// In here, we retrieve triangulated polygons from the mesh
	MFnMesh mesh(object);

	map<UINT, vector<UINT>> vertexToIndex;

	vector<VertexLayout> verticesData;
	vector<UINT>indexArray;

	MIntArray intdexOffsetVertexCount, vertices, triangleList;
	MPointArray dummy;

	UINT vertexIndex;
	MVector normal;
	MPoint pos;
	float2 UV;
	VertexLayout thisVertex;

	for (MItMeshPolygon meshPolyIter(object); !meshPolyIter.isDone(); meshPolyIter.next())
	{
		vector<UINT> localVertexToGlobalIndex;
		meshPolyIter.getVertices(vertices);

		meshPolyIter.getTriangles(dummy, triangleList);
		UINT indexOffset = verticesData.size();

		for (UINT i = 0; i < vertices.length(); i++)
		{
			vertexIndex = meshPolyIter.vertexIndex(i);
			pos = meshPolyIter.point(i);
			pos.get(thisVertex.pos);

			meshPolyIter.getNormal(i, normal);
			thisVertex.normal[0] = normal[0];
			thisVertex.normal[1] = normal[1];
			thisVertex.normal[2] = normal[2];

			meshPolyIter.getUV(i, UV);
			thisVertex.uv[0] = UV[0];
			thisVertex.uv[1] = UV[1];

			verticesData.push_back(thisVertex);
			localVertexToGlobalIndex.push_back(vertexIndex);

			cout << "Pos: " << thisVertex.pos[0] << "/" << thisVertex.pos[1] << "/" << thisVertex.pos[2] << endl;
			cout << "Normals: " << thisVertex.normal[0] << "/" << thisVertex.normal[1] << "/" << thisVertex.normal[2] << endl;
			cout << "UV: " << thisVertex.uv[0] << "/" << thisVertex.uv[1] << endl;
		}
		for (UINT i = 0; i < triangleList.length(); i++)
		{
			UINT k = 0;
			while (localVertexToGlobalIndex[k] != triangleList[i])
				k++;
			indexArray.push_back(indexOffset + k);
		}
	}
	
}

void Menu::GetMaterialData()
{
	this->MaterialHandler = new Material();

	// Traverse scene and return vector with all materials
	std::vector<MaterialNode>* AllMaterials = MaterialHandler->DoIt();

	// Access the colorR component of one material (example)
	cout << AllMaterials->at(0).Color[0] << endl;
	MGlobal::displayInfo(MString() +  AllMaterials->at(0).Color[0]);
}

Menu::~Menu()
{
	//delete exportSelectedButton;
	//delete browseButton;
	//delete exportPath;
	//delete fileDialog;
	fileDialog->~QFileDialog();
	delete MaterialHandler;
}