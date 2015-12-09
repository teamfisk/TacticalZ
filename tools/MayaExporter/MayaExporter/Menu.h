#ifndef BUTTONS_H
#define BUTTONS_H

#include <map>
#include <vector>

#include "MayaIncludes.h"
#include "Material.h"
// Qt
#pragma comment(lib, "QtCore4")
#pragma comment(lib, "QtGui4")

#include <QtCore/qcoreapplication.h>

#include <maya/MQtUtil.h>
#include <QtGui/qwidget.h>
#include <QtGui/qapplication.h>
#include <QtGui/qdialog.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qformlayout.h>
#include <QtCore/qthread.h>
#include <QtCore/qpointer.h>
#include <QtCore/qlist.h>
#include <QtGui/qlistwidget.h>
#include <QtUiTools/quiloader.h>
#include <QtCore/qfile.h>
#include <QtGui/qlabel.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qradiobutton.h>
#include <QtGui/qfiledialog.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qgroupbox.h>

struct VertexLayout
{
	float pos[3];
	float normal[3];
	float uv[2];
};

class Menu : public QWidget
{
	Q_OBJECT
public:
	Menu(QDialog* dialog);
	~Menu();

	void GetMeshData(MObject object);
	void GetMaterialData();

private slots:
	void ExportSelected(bool checked);
	void ExportPathClicked(bool);
	void ExportAll(bool);
	void CancelClicked(bool);

	void Button1Clicked(bool);
	void Button2Clicked(bool);
	void Button3Clicked(bool);


private:
	Menu();
	QPushButton* exportSelectedButton;
	QPushButton* browseButton;
	QPushButton* exportAllButton;
	QPushButton* cancelButton;

	QCheckBox* exportAnimationsButton;
	QCheckBox* copyTexturesButton;
	QCheckBox* button3;

	QLineEdit* exportPath;
	QFileDialog* fileDialog;
	QDialog* dialogPointer;

	Material* MaterialHandler;
};

#endif