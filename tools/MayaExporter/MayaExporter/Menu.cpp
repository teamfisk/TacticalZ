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

	connect(m_ExportAnimationsButton, SIGNAL(clicked(bool)), this, SLOT(Button1Clicked(bool)));
	connect(m_CopyTexturesButton, SIGNAL(clicked(bool)), this, SLOT(Button2Clicked(bool)));
	connect(m_Button3, SIGNAL(clicked(bool)), this, SLOT(Button3Clicked(bool)));

	// Creating several layouts, adding widgets & adding them to one layout in the end
	QHBoxLayout* topLayout = new QHBoxLayout;
	QVBoxLayout* midLayout = new QVBoxLayout;
	QHBoxLayout* botLayout = new QHBoxLayout;
	QVBoxLayout* baseLayout = new QVBoxLayout;

	m_ExportPath = new QLineEdit;
	m_FileDialog = new QFileDialog;

	QLabel* exportLabel = new QLabel;
	exportLabel->setText("Export Path:");
	
	midLayout->addWidget(optionsBox);

	topLayout->addWidget(exportLabel);
	topLayout->addWidget(m_ExportPath);
	topLayout->addWidget(m_BrowseButton);

	botLayout->addWidget(m_ExportSelectedButton);
	botLayout->addWidget(m_ExportAllButton);
	botLayout->addWidget(m_CancelButton);

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
	for (unsigned int i = 0; i < selected.length();i++) {
		MObject object;
		selected.getDependNode(i, object);
		MFnDependencyNode thisNode(object);

		cout << thisNode.name().asChar() << endl;
		Mesh mesh;
		mesh.GetMeshData(object);
	}
	if (m_ExportPath->text().isEmpty()) {
		cout << "Please select a folder." << endl;
	}
	else {
		cout << m_ExportPath->text().toLocal8Bit().constData() << endl;
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

void Menu::ExportAll(bool)
{
	MDagPath path;
	
	// Loop through all nodes in the scene
	MItDependencyNodes it(MFn::kInvalid);
	for (;!it.isDone();it.next()) {
		MObject node = it.thisNode();
		if (node.hasFn(MFn::kMesh)) {
			MFnDependencyNode thisNode(node);

			cout << thisNode.name().asChar() << endl;
			Mesh mesh;
			mesh.GetMeshData(node);
		}
	}
	if (m_ExportPath->text().isEmpty()) {
		cout << "Please select a folder." << endl;
	}
	else {
		cout << m_ExportPath->text().toLocal8Bit().constData() << endl;
	}
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

void Menu::GetMaterialData()
{
	this->m_MaterialHandler = new Material();

	// Traverse scene and return vector with all materials
	std::vector<MaterialNode>* AllMaterials = m_MaterialHandler->DoIt();

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
	m_FileDialog->~QFileDialog();
	//delete MaterialHandler;
}