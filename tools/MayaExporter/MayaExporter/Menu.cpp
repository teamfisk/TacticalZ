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
	m_BrowseButton = new QPushButton("&...", this);
	m_ExportAllButton = new QPushButton("&Export", this);
	m_CancelButton = new QPushButton("&Cancel", this);
	m_AddClipsButton = new QPushButton("&Add Clips", this);
	m_RemoveClipsButton = new QPushButton("&Remove Latest Clip", this);

	// Option box and checkboxes
	QGroupBox *optionsBox = new QGroupBox(tr("Options"));

    m_ExportSelectedButton = new QCheckBox(tr("&Export Selected"));
	m_ExportAnimationsButton = new QCheckBox(tr("&Export Animations"));
	m_ExportMaterialButton = new QCheckBox(tr("&Export Material"));;

	m_ExportAnimationsButton->setChecked(true);
    m_ExportMaterialButton->setChecked(true);
	QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(m_ExportSelectedButton);
	vbox->addWidget(m_ExportAnimationsButton);
	vbox->addWidget(m_ExportMaterialButton);

	vbox->addStretch(1);
	optionsBox->setLayout(vbox);

	// Connect the buttons with signals & functions
	connect(m_BrowseButton, SIGNAL(clicked(bool)), this, SLOT(ExportPathClicked(bool)));
	connect(m_ExportAllButton, SIGNAL(clicked(bool)), this, SLOT(ExportAll(bool)));
	connect(m_CancelButton, SIGNAL(clicked(bool)), this, SLOT(CancelClicked(bool)));
	connect(m_AddClipsButton, SIGNAL(clicked(bool)), this, SLOT(AddClipClicked(bool)));
	connect(m_RemoveClipsButton, SIGNAL(clicked(bool)), this, SLOT(RemoveClipClicked(bool)));

    connect(m_ExportSelectedButton, SIGNAL(clicked(bool)), this, SLOT(NULL));
	connect(m_ExportAnimationsButton, SIGNAL(clicked(bool)), this, SLOT(NULL));
	connect(m_ExportMaterialButton, SIGNAL(clicked(bool)), this, SLOT(NULL));

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

    this->AddClipClicked(true);
	//for (unsigned int i = 0; i < 3; i++) {
	//	this->AddClipClicked(true);
	//}

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
        m_AnimationClipName.pop_back();
	}
}

void Menu::ExportAll(bool)
{
    if (m_ExportPath->text().isEmpty()) {
        MGlobal::displayError(MString() + "Please select a folder.");
        return;
    }

    //Export meshes
    if (!m_Export.Meshes(m_ExportPath->text().toLocal8Bit().constData(), m_ExportSelectedButton->isChecked())) {
        MGlobal::displayError(MString() + "Could not export mesh");
        return;
    }

    if (m_ExportMaterialButton->isChecked()) {
        if (!m_Export.Materials(m_ExportPath->text().toLocal8Bit().constData())) {
            MGlobal::displayError(MString() + "Could not export materials");
            return;
        }
    }

    std::vector<Export::AnimationInfo> animations;
    for (unsigned int i = 0; i < m_AnimationClipName.size(); i++) {
        Export::AnimationInfo thisClip;
        thisClip.Name = std::string(m_AnimationClipName[i]->text().toLocal8Bit().constData());
        thisClip.Start = m_StartFrameLines[i]->text().toInt();
        thisClip.End = m_EndFrameLines[i]->text().toInt();

        animations.push_back(thisClip);
    }
    if (m_ExportAnimationsButton->isChecked()) {
        //Export Animations
        if (!m_Export.Animations(m_ExportPath->text().toLocal8Bit().constData(), animations)) {
            MGlobal::displayError(MString() + "Could not export animations");
            return;
        }
    }
}

void Menu::CancelClicked(bool)
{
	m_DialogPointer->close();
}

Menu::~Menu()
{
	//delete exportSelectedButton;
	//delete browseButton;
	//delete exportPath;
	//delete fileDialog;
	m_FileDialog->~QFileDialog();

    //delete m_Export;
	//delete MaterialHandler;
}