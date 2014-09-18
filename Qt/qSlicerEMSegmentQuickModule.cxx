/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QScopedPointer>
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerScriptedLoadableModuleWidget.h>
#include <qSlicerUtils.h>
#include <vtkSlicerConfigure.h> // For Slicer_QTLOADABLEMODULES_LIB_DIR

// EMSegment Logic includes
#include <vtkEMSegmentLogic.h>

// EMSegment QTModule includes
#include "qSlicerEMSegmentQuickModule.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerEMSegmentQuickModule, qSlicerEMSegmentQuickModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegmentQuick
class qSlicerEMSegmentQuickModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerEMSegmentQuickModule::qSlicerEMSegmentQuickModule(QObject* _parent)
  :Superclass(_parent)
  , d_ptr(new qSlicerEMSegmentQuickModulePrivate)
{

}

//-----------------------------------------------------------------------------
qSlicerEMSegmentQuickModule::~qSlicerEMSegmentQuickModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentQuickModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::helpText()const
{
  QString help =
      "<b>EMSegment Easy Module:</b>  This module provides EM segmentation without an atlas.<br>"
      "<br>"
      "It is possible to segment different structures by manual sampling.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::acknowledgementText()const
{
  return QLatin1String(
        "<img src=':/Icons/UPenn_logo.png'><br>"
        "<br>"
        "This module is currently maintained by Daniel Haehn"
        " and Kilian Pohl (SBIA,UPenn). The work is currently supported by an ARRA "
        "supplement to NAC and the Slicer Community (see also <a>http://www.slicer.org</a>). <br>"
        "<br>"
        "The work was reported in  <br>"
        "K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. "
        "IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007.");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerEMSegmentQuickModule::createWidgetRepresentation()
{
  // HACK - Since dependencies between modules are not yet managed at runtime, let's
  //      set the share directory associated with EMSegmentEasy logic when 'createWidgetRepresentation'
  //      is invoked.
  //      - Since 'createWidgetRepresentation' is called on-demand and since the logic have been
  //      instantiated when Slice initialized, it's okay.
  qSlicerModuleManager * moduleManager = qSlicerCoreApplication::application()->moduleManager();
  Q_ASSERT(moduleManager);
  qSlicerAbstractCoreModule * emsegmentModule = moduleManager->module("EMSegment");\
  if (!emsegmentModule)
    {
    qCritical() << "Failed to load EMSegmentEasy widget ! "
                   "EMSegment module is expected to be loaded.";
    return 0;
    }
  vtkSlicerModuleLogic * emsegmentLogic =
      vtkSlicerModuleLogic::SafeDownCast(emsegmentModule->logic());
  if (!emsegmentLogic)
    {
    qCritical() << "Failed to load EMSegmentEasy widget ! "
                   "EMSegment module couldn't be retrieved.";
    return 0;
    }

  vtkSlicerModuleLogic * logic = vtkSlicerModuleLogic::SafeDownCast(this->logic());
  logic->SetModuleShareDirectory(emsegmentLogic->GetModuleShareDirectory());

  QString pythonPath = qSlicerUtils::pathWithoutIntDir(
              QFileInfo(this->path()).path(), Slicer_QTLOADABLEMODULES_LIB_DIR);

  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  QString classNameToLoad = "qSlicerEMSegmentQuickModuleWidget";
  bool ret = widget->setPythonSource(
        pythonPath + "/Python/" + classNameToLoad + ".py", classNameToLoad);
  if (!ret)
    {
    return 0;
    }
  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerEMSegmentQuickModule::createLogic()
{
  return vtkEMSegmentLogic::New();
}

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentQuickModule::icon() const
{
  return QIcon(":/Icons/EMSegmentQuick.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerEMSegmentQuickModule::categories()const
{
  return QStringList() << QLatin1String("Segmentation");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentQuickModule::contributor()const
{
  return QLatin1String("Daniel Haehn, Kilian Pohl");
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentQuickModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  this->Superclass::setMRMLScene(_mrmlScene);
  vtkEMSegmentLogic * emsegmentLogic =
      vtkEMSegmentLogic::SafeDownCast(this->logic());
  Q_ASSERT(emsegmentLogic);
  if (_mrmlScene)
    {
    emsegmentLogic->InitializeEventListeners();
    }
}
