#ifndef Menu_Menu_h__
#define Menu_Menu_h__

#include <map>
#include <vector>

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

#include "MayaIncludes.h"
#include "Material.h"
#include "Mesh.h"
#include "Skeleton.h"

class Menu : public QWidget
{
	Q_OBJECT
public:
	Menu(QDialog* dialog);
	~Menu();

	void GetMaterialData();
	void GetSkeletonData();

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
	QPushButton* m_ExportSelectedButton = nullptr;
	QPushButton* m_BrowseButton = nullptr;
	QPushButton* m_ExportAllButton = nullptr;
	QPushButton* m_CancelButton = nullptr;

	QCheckBox* m_ExportAnimationsButton = nullptr;
	QCheckBox* m_CopyTexturesButton = nullptr;
	QCheckBox* m_Button3 = nullptr;

	QLineEdit* m_ExportPath = nullptr;
	QFileDialog* m_FileDialog = nullptr;
	QDialog* m_DialogPointer = nullptr;

	Material* m_MaterialHandler = nullptr;
	Skeleton* m_SkeletonHandler = nullptr;
};

#endif