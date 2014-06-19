
#include <vtkSlicerConfigure.h>

#ifdef Slicer3_USE_KWWIDGETS

// Slicer3
#include "vtkKWTkUtilities.h"
#include "../../Applications/GUI/Slicer3Helper.cxx"
#include "vtkEMSegmentKWDynamicFrame.h"
#include "vtkEMSegmentPreProcessingStep.h"
#include "vtkEMSegmentInputChannelsStep.h"
#include "vtkEMSegmentGUI.h"

#else

#include <vtksys/SystemTools.hxx>

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkErrorLogModel.h>
#include <ctkScopedCurrentDir.h>

// PythonQT includes
#include <PythonQt.h>

#endif

// EMSegment includes
#include "vtkSlicerCommonInterface.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkSlicerCommonInterface* vtkSlicerCommonInterface::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerCommonInterface");
  if (ret)
    {
    return (vtkSlicerCommonInterface*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerCommonInterface;
}

//----------------------------------------------------------------------------
vtkSlicerCommonInterface::vtkSlicerCommonInterface()
{

  // initialize random seed
  srand(time(NULL));

  this->StringHolder = std::string("");

  this->simpleMode = false;

#ifndef Slicer3_USE_KWWIDGETS

  // Slicer4
  this->remoteIOLogic = 0;
  this->returnChar = NULL;

#endif

}

//----------------------------------------------------------------------------
vtkSlicerCommonInterface::~vtkSlicerCommonInterface()
{

#ifndef Slicer3_USE_KWWIDGETS
  if (this->remoteIOLogic)
    {
    this->remoteIOLogic->Delete();
    this->remoteIOLogic = 0;
    }

  if (this->returnChar  )
    {
      delete[] this->returnChar;
      this->returnChar = NULL; 
    }
#endif

}

//----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  Tcl_Interp* vtkSlicerCommonInterface::Startup(int& argc, char *argv[], ostream* err)
#else 
  Tcl_Interp* vtkSlicerCommonInterface::Startup(int& argc, char *argv[], ostream* vtkNotUsed(err))
#endif 
{
#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::InitializeTcl(argc, argv, err);

#else

  // Slicer4
  qSlicerApplication* app = new qSlicerApplication(argc, argv);
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);
  if(!disablePython)
    {
    // Load EMSegment Python module(s)
    QString emsegmentModulePath = QString::fromStdString(
        vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory(argv[0]));
    emsegmentModulePath.append("/" Slicer_QTLOADABLEMODULES_SUBDIR);
    app->pythonManager()->appendPythonPath(emsegmentModulePath);
    // Update current application directory, so that *PythonD modules can be loaded
    ctkScopedCurrentDir scopedCurrentDir(emsegmentModulePath);

    QString emsegmentModulePythonPath = emsegmentModulePath + "/Python";
    std::cout << "emsegmentModulePythonPath:" << qPrintable(emsegmentModulePythonPath) << std::endl;
    app->pythonManager()->executeString(QString(
      "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg(emsegmentModulePythonPath).arg("EMSegment"));

    }
#endif
  // we don't need this here!
  app->errorLogModel()->setAllMsgHandlerEnabled(false);
#endif

  return 0;
}

//----------------------------------------------------------------------------

int vtkSlicerCommonInterface::SourceTclFile(const char *tclFile)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3

  // Load Tcl File defining the setting
  if (!vtkSlicerApplication::GetInstance()->LoadScript(tclFile))
    {
    return 1;
    }

#endif

#ifdef Slicer_USE_PYTHONQT_WITH_TCL

  // Slicer4 and Tcl through Python activated
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);

  if (!disablePython)
    {
    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();

    QString import = QString("from __main__ import tcl\n");
    QString tclCall = import + QString("tcl('source \"%1\"')\n");

    py->executeString(tclCall.arg(tclFile));

    }
  else
    {
    return 1;
    }

#endif

  return 0;

}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::EvaluateTcl(const char* command)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->Script(command);
  //#else

#endif

#ifdef Slicer_USE_PYTHONQT_WITH_TCL

  // Slicer4 and Tcl through Python activated
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);

  if (!disablePython)
    {
    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();

    QString import = QString("from __main__ import tcl");

    py->executeString(import);

    QString tclCall = QString("__rv = tcl('%1')");

    py->executeString(tclCall.arg(command));

    QVariant returnValue = py->getVariable("__rv");

    // convert returnValue to const char*
    QByteArray byteArray = returnValue.toString().toUtf8();
    this->resetReturnChar() ;
    this->returnChar = qstrdup( byteArray );
    // const char* cString = byteArray.constData();
    return this->returnChar;
    }

#endif

  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetExtensionsDirectory()
{

#ifdef Slicer3_USE_KWWIDGETS

  return vtkSlicerApplication::GetInstance()->GetExtensionsInstallPath();

#else

  // Slicer4
  QString extensionsDir = qSlicerApplication::application()->extensionsInstallPath();
  this->resetReturnChar() ;
  this->returnChar = qstrdup( extensionsDir.toLatin1() );
  return this->returnChar;

#endif

}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetTclNameFromPointer(vtkObject *obj)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkKWTkUtilities::GetTclNameFromPointer(
      vtkSlicerApplication::GetInstance()->GetMainInterp(), obj);

#endif

#if defined(Slicer_USE_PYTHONQT)

  // Slicer4

  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);

  if (!disablePython)
    {
    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();

    // here we can choose our own tcl name
    this->StringHolder = this->randomStrGen(8);

    // Note: we can not register python variables on toplevel, so we use the namespace slicer
    std::string pythonName = std::string("slicer." + this->StringHolder);

    py->addVTKObjectToPythonMain(pythonName.c_str(), obj);

    // register the python variable in Tcl
    std::string registerCmd = "proc ::" + this->StringHolder + " {args} {::tpycl::methodCaller " + this->StringHolder + " " + pythonName + " $args}";
    this->EvaluateTcl(registerCmd.c_str());

    return this->StringHolder.c_str();
    }

#endif

  return 0;
}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::RegisterObjectWithTcl(vtkObject* obj, const char* name)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  const char* tclName = this->GetTclNameFromPointer(obj);

  std::stringstream cmd;
  cmd << "namespace eval slicer3 set ";
  cmd << std::string(name) + " ";
  cmd << std::string(tclName);

  this->EvaluateTcl(cmd.str().c_str());

#endif

#if defined(Slicer_USE_PYTHONQT)

  // Slicer4
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);

  if (!disablePython)
    {
    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();
    py->addVTKObjectToPythonMain(name, obj);
    }

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
   void vtkSlicerCommonInterface::InitializePythonViaTcl(Tcl_Interp* interp, int argc, char **argv)
#else 
     void vtkSlicerCommonInterface::InitializePythonViaTcl(Tcl_Interp* vtkNotUsed(interp), int vtkNotUsed(argc), char** vtkNotUsed(argv))
#endif 
{
#if defined(Slicer3_USE_KWWIDGETS) && defined(Slicer3_USE_PYTHON)
  // Initialize Python
#if defined(_WIN32) || defined(WIN32)
#define PathSep ";"
#else
#define PathSep ":"
#endif
  vtksys_stl::string slicerHome;
  if (!vtksys::SystemTools::GetEnv(Slicer_HOME_ENVVAR_NAME, slicerHome))
    {
    slicerHome = std::string(this->GetBinDirectory()) + "/..";
    slicerHome = vtksys::SystemTools::CollapseFullPath(slicerHome.c_str());
    }

  // Set up the search path
  std::string pythonEnv = "PYTHONPATH=";

  const char* existingPythonEnv = vtksys::SystemTools::GetEnv("PYTHONPATH");
  if (existingPythonEnv)
    {
    pythonEnv += std::string(existingPythonEnv) + PathSep;
    }

  pythonEnv += slicerHome + "/" + Slicer3_INSTALL_LIB_DIR
      + "/SlicerBaseGUI/Python" + PathSep;
  pythonEnv += slicerHome + "/" + Slicer3_INSTALL_PLUGINS_BIN_DIR + PathSep;
  vtkKWApplication::PutEnv(const_cast<char *> (pythonEnv.c_str()));

  Py_Initialize();
  PySys_SetArgv(argc, argv);
  PyObject* PythonModule = PyImport_AddModule("__main__");
  if (PythonModule == NULL)
    {
    std::cout << "Warning: Failed to initialize python" << std::endl;
    }
  PyObject* PythonDictionary = PyModule_GetDict(PythonModule);

  // Intercept _tkinter, and use ours...
  init_mytkinter(vtkSlicerApplication::GetInstance()->GetMainInterp());
  init_slicer();
  PyObject* v;

  std::vector < std::string > pythonInitStrings;

  pythonInitStrings.push_back(std::string("import _tkinter;"));
  pythonInitStrings.push_back(std::string("import Tkinter;"));
  pythonInitStrings.push_back(std::string("import sys;"));
  pythonInitStrings.push_back(std::string("from os.path import join as j;"));
  pythonInitStrings.push_back(std::string("tk = Tkinter.Tk();"));
  pythonInitStrings.push_back(std::string("sys.path.append ( j('" + slicerHome
      + "','" + Slicer3_INSTALL_LIB_DIR + "', 'SlicerBaseGUI', 'Python')"
      + " );"));
  pythonInitStrings.push_back(std::string("sys.path.append ( j('" + slicerHome
      + "','" + Slicer3_INSTALL_PLUGINS_BIN_DIR + "') );"));

  /*
   std::string TkinitString = "import Tkinter, sys;"
   "from os.path import join as j;"
   "tk = Tkinter.Tk();"
   "sys.path.append ( j('"
   + slicerHome + "','" + Slicer3_INSTALL_LIB_DIR + "', 'SlicerBaseGUI', 'Python')"
   + " );\n"
   "sys.path.append ( j('"
   + slicerHome + "','" + Slicer3_INSTALL_PLUGINS_BIN_DIR
   + "') );\n";
   */

  std::vector<std::string>::iterator strIt;
  strIt = pythonInitStrings.begin();
  for (; strIt != pythonInitStrings.end(); strIt++)
    {
    v = PyRun_String( (*strIt).c_str(),
        Py_file_input,
        PythonDictionary,
        PythonDictionary );
    if (v == NULL)
      {
      PyObject *exception, *v, *tb;
      PyObject *exception_s, *v_s, *tb_s;

      PyErr_Fetch(&exception, &v, &tb);
      if (exception != NULL)
        {
        PyErr_NormalizeException(&exception, &v, &tb);
        if (exception != NULL)
          {
          exception_s = PyObject_Str(exception);
          v_s = PyObject_Str(v);
          tb_s = PyObject_Str(tb);
          const char *e_string, *v_string, *tb_string;
          cout << "Running: " << (*strIt).c_str() << endl;
          e_string = PyString_AS_STRING(exception_s);
          cout << "Exception: " << e_string << endl;
          v_string = PyString_AS_STRING(v_s);
          cout << "V: " << v_string << endl;
          tb_string = PyString_AS_STRING(PyObject_Str(tb_s));
          cout << "TB: " << tb_string << endl;
          Py_DECREF ( exception_s )
            ;
          Py_DECREF ( v_s )
            ;
          Py_DECREF ( tb_s )
            ;
          Py_DECREF ( exception )
            ;
          Py_DECREF ( v )
            ;
          if (tb)
            {
            Py_DECREF ( tb )
              ;
            }
          }
        }

      PyErr_Print();
      }
    else
      {
      if (Py_FlushLine())
        {
        PyErr_Clear();
        }
      }
    }

  vtkSlicerApplication::GetInstance()->InitializePython((void*) PythonModule,
      (void*) PythonDictionary);

#endif

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::EvaluatePython(const char* command)
{

#if defined(Slicer3_USE_KWWIDGETS) && defined(Slicer3_USE_PYTHON)

  PyObject* PythonModule = PyImport_AddModule("__main__");
  if (PythonModule == NULL)
    {
    std::cout << "Warning: Failed to initialize python" << std::endl;
    }
  PyObject* PythonDictionary = PyModule_GetDict(PythonModule);

  //command = "from Slicer import slicer; print slicer.vtkSlicerVolumesLogic();";

  PyObject* v = PyRun_String( command,
      Py_file_input,
      PythonDictionary,PythonDictionary);
  if (v == NULL)
    {
    PyErr_Print();
    }

#endif

#if defined(Slicer_USE_PYTHONQT)

  // Slicer4 and Tcl through Python activated
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);

  if (!disablePython)
    {
    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();

    QString pCommand = QString(command);
    pCommand += "\n";

    py->executeString(pCommand);
    }

#endif

}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetApplicationTclName()
{
#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return this->GetTclNameFromPointer(vtkSlicerApplication::GetInstance());

#endif

#if defined(Slicer_USE_PYTHONQT)

  // Slicer4
  // Slicer4 and Tcl through Python activated
  bool disablePython = qSlicerApplication::testAttribute(qSlicerApplication::AA_DisablePython);

  if (!disablePython)
    {

    qSlicerPythonManager *py = qSlicerApplication::application()->pythonManager();

    // here we can choose our own tcl name
    this->StringHolder = this->randomStrGen(8);

    py->addObjectToPythonMain(this->StringHolder.c_str(), qSlicerApplication::application());

    return this->StringHolder.c_str();
    }

#endif

  return 0;
}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetTemporaryDirectory()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  this->StringHolder = std::string(
      vtkSlicerApplication::GetInstance()->GetTemporaryDirectory());
  return this->StringHolder.c_str();

#else

  // Slicer4
  this->StringHolder = std::string(qSlicerApplication::application()->temporaryPath().toLatin1());
  return this->StringHolder.c_str();

#endif

}

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetBinDirectory()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->GetBinDir();

#else

  // Slicer4 - do not put in constructor bc otherwise tests fail 
  // I also added class variable bc if you only give a pointer Slicer can crash 
  // when pointer is asssigned to something else 
   QString dir = QString(qSlicerApplication::application()->slicerHome()) + QString("/bin/");
   this->resetReturnChar(); 
   this->returnChar = qstrdup( dir.toLatin1() );
   
  return this->returnChar;

#endif

  return 0;

}
#ifdef Slicer3_USE_KWWIDGETS
//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetPluginsDirectory()
{
  // Slicer3
  return vtkSlicerApplication::GetInstance()->GetPluginsDir();
}

#else
const char* vtkSlicerCommonInterface::GetPluginWithFullPath(const char* pluginName)
{
  this->returnString = std::string(vtksys::SystemTools::FindProgram(pluginName));

  if (this->returnString.empty()) {
    vtkErrorMacro("Could not find path to " <<  pluginName); 
  }
  
  return this->returnString.c_str();
}
#endif

//-----------------------------------------------------------------------------
const char* vtkSlicerCommonInterface::GetRepositoryRevision()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkSlicerApplication::GetInstance()->GetSvnRevision();

#else

  // Slicer4
  this->resetReturnChar() ;
  this->returnChar = qstrdup( qSlicerApplication::application()->repositoryRevision().toLatin1() );
  return this->returnChar;

#endif

  return 0;

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::SetApplicationBinDir(const char* bindir)
#else
    void vtkSlicerCommonInterface::SetApplicationBinDir(const char* vtkNotUsed(bindir))
#endif 

{
#ifdef Slicer3_USE_KWWIDGETS

  vtkSlicerApplication::GetInstance()->SetBinDir(bindir);

#endif
}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  vtkHTTPHandler* vtkSlicerCommonInterface::GetHTTPHandler(vtkMRMLScene* scene)
#else
  vtkHTTPHandler* vtkSlicerCommonInterface::GetHTTPHandler(vtkMRMLScene* vtkNotUsed(scene))
#endif 

{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  return vtkHTTPHandler::SafeDownCast(
      scene->FindURIHandlerByName("HTTPHandler"));

#else

  // Slicer4
  // TODO
  return 0;

#endif

  return 0;

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::PromptBeforeExitOff()
{

#ifdef Slicer3_USE_KWWIDGETS

  vtkSlicerApplication::GetInstance()->PromptBeforeExitOff();

#endif

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::DestroySlicerApplication()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  this->PromptBeforeExitOff();

  vtkSlicerApplication* app = vtkSlicerApplication::GetInstance();

  if (app)
    {
    app->Exit();
    app->Delete();
    app = NULL;
    }

#else

  // Slicer4
  qSlicerApplication* app = qSlicerApplication::application();

  if (app)
    {
    delete app;
    }

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic *appLogic, vtkDataIOManagerLogic *dataIOManagerLogic)
#else
  void vtkSlicerCommonInterface::AddDataIOToScene(vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic* vtkNotUsed(appLogic), vtkDataIOManagerLogic *dataIOManagerLogic)
#endif
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  Slicer3Helper::AddDataIOToScene(mrmlScene,
      vtkSlicerApplication::GetInstance(), appLogic, dataIOManagerLogic);

#else

  // Slicer4
  if (!this->remoteIOLogic)
    {
    vtkMRMLRemoteIOLogic *remoteIOLogic = vtkMRMLRemoteIOLogic::New();
    this->remoteIOLogic = remoteIOLogic;
    }

  dataIOManagerLogic->SetAndObserveDataIOManager(this->remoteIOLogic->GetDataIOManager());

  this->remoteIOLogic->SetMRMLScene(mrmlScene);
  this->remoteIOLogic->AddDataIOToScene();

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::RemoveDataIOFromScene(vtkMRMLScene* mrmlScene, vtkDataIOManagerLogic *dataIOManagerLogic)
#else
    void vtkSlicerCommonInterface::RemoveDataIOFromScene(vtkMRMLScene* vtkNotUsed(mrmlScene), vtkDataIOManagerLogic* vtkNotUsed(dataIOManagerLogic))
#endif 
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  Slicer3Helper::RemoveDataIOFromScene(mrmlScene, dataIOManagerLogic);

#else

  // Slicer4
  if (this->remoteIOLogic)
    {
    this->remoteIOLogic->RemoveDataIOFromScene();
    }

#endif

}

//-----------------------------------------------------------------------------
std::string vtkSlicerCommonInterface::randomStrGen(int length)
{
  static std::string charset =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::string result;
  result.resize(length);

  for (int i = 0; i < length; i++)
    result[i] = charset[rand() % charset.length()];

  return result;
}

//-----------------------------------------------------------------------------
//
// DYNAMIC FRAME CREATION
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
vtkSlicerCommonInterface* vtkSlicerCommonInterface::GetSimpleDynamicFrame()
{

  this->simpleMode = true;
  return this;
}

//-----------------------------------------------------------------------------
vtkSlicerCommonInterface* vtkSlicerCommonInterface::GetAdvancedDynamicFrame()
{

  this->simpleMode = false;
  return this;
}

//-----------------------------------------------------------------------------
vtkEMSegmentKWDynamicFrame* vtkSlicerCommonInterface::GetSlicer3DynamicFrame()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentGUI* mod = vtkEMSegmentGUI::SafeDownCast(
      vtkSlicerApplication::GetInstance()->GetModuleGUIByName("EMSegmenter"));
  if (!mod)
    {
    vtkErrorMacro("Could not get the EMSegmenter GUI!")
    return 0;
    }

  vtkEMSegmentKWDynamicFrame* dynFrame = 0;
  // EMS Advanced Mode
  if (this->simpleMode)
    {
    dynFrame = mod->GetInputChannelStep()->GetCheckListFrame();
    }
  else
    {
    dynFrame = mod->GetPreProcessingStep()->GetCheckListFrame();
    }

  return dynFrame;

#endif

  return 0;

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::DefineCheckButton(const char *label, int initState, vtkIdType ID)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->DefineCheckButton(label, initState, ID);

#endif

#ifdef Slicer_USE_PYTHONQT

  // Slicer4
  std::stringstream cmd;
  if (this->simpleMode)
    {
    cmd << "slicer.modules.emsegmentSimpleDynamicFrame.DefineCheckButton('";
    }
  else
    {
    cmd << "slicer.modules.emsegmentAdvancedDynamicFrame.DefineCheckButton('";
    }
  cmd << label << "',";
  cmd << initState;
  cmd << ",";
  cmd << ID;
  cmd << ")\n";

  this->EvaluatePython(cmd.str().c_str());

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  int vtkSlicerCommonInterface::GetCheckButtonValue(vtkIdType ID)
#else
    int vtkSlicerCommonInterface::GetCheckButtonValue(vtkIdType vtkNotUsed(ID))
#endif

{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return -1;
    }

  return dynFrame->GetCheckButtonValue(ID);

#endif

  return -1;

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::DefineTextLabel(const char* label, vtkIdType ID)
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->DefineTextLabel(label, ID);

#endif

#ifdef Slicer_USE_PYTHONQT

  // Slicer4
  std::string str( label );
  std::string searchString( "\n" );
  std::string replaceString( "<br>" );

  std::string::size_type pos = 0;
  while ( (pos = str.find(searchString, pos)) != std::string::npos )
    {
    str.replace( pos, searchString.size(), replaceString );
    pos++;
    }

  std::stringstream cmd;
  if (this->simpleMode)
    {
    cmd << "slicer.modules.emsegmentSimpleDynamicFrame.DefineTextLabel('";
    }
  else
    {
    cmd << "slicer.modules.emsegmentAdvancedDynamicFrame.DefineTextLabel('";
    }
  cmd << str << "',";
  cmd << ID;
  cmd << ")\n";

  this->EvaluatePython(cmd.str().c_str());

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::DefineVolumeMenuButton(const char *label, vtkIdType initVolID, vtkIdType buttonID)
#else
    void vtkSlicerCommonInterface::DefineVolumeMenuButton(const char * vtkNotUsed(label), vtkIdType vtkNotUsed(initVolID), vtkIdType vtkNotUsed(buttonID))
#endif 
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->DefineVolumeMenuButton(label, initVolID, buttonID);

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  vtkIdType vtkSlicerCommonInterface::GetVolumeMenuButtonValue(vtkIdType  ID)
#else
  vtkIdType vtkSlicerCommonInterface::GetVolumeMenuButtonValue(vtkIdType  vtkNotUsed(ID))
#endif
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return 0;
    }

  return dynFrame->GetVolumeMenuButtonValue(ID);

#endif

  return 0;

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::VolumeMenuButtonCallback(vtkIdType buttonID, vtkIdType volID)
#else
    void vtkSlicerCommonInterface::VolumeMenuButtonCallback(vtkIdType vtkNotUsed(buttonID), vtkIdType vtkNotUsed(volID))
#endif 
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->VolumeMenuButtonCallback(buttonID, volID);

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::DefineTextEntry(const char *label, const char *initText, vtkIdType entryID, int widgetWidth)
#else 
    void vtkSlicerCommonInterface::DefineTextEntry(const char* vtkNotUsed(label), const char* vtkNotUsed(initText), vtkIdType vtkNotUsed(entryID), int vtkNotUsed(widgetWidth))
#endif 
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->DefineTextEntry(label, initText, entryID, widgetWidth);

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::DefineTextEntry(const char *label, const char *initText, vtkIdType entryID)
#else
    void vtkSlicerCommonInterface::DefineTextEntry(const char* vtkNotUsed(label), const char* vtkNotUsed(initText), vtkIdType vtkNotUsed(entryID))
#endif 
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->DefineTextEntry(label, initText, entryID);

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  const char* vtkSlicerCommonInterface::GetTextEntryValue(vtkIdType ID)
#else
  const char* vtkSlicerCommonInterface::GetTextEntryValue(vtkIdType vtkNotUsed(ID))
#endif 
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return 0;
    }

  return dynFrame->GetTextEntryValue(ID);

#endif

  return 0;

}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::SetButtonsFromMRML()
{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->SetButtonsFromMRML();

#endif

}

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWIDGETS
  void vtkSlicerCommonInterface::PopUpWarningWindow(const char * msg)
#else
    void vtkSlicerCommonInterface::PopUpWarningWindow(const char* vtkNotUsed(msg))
#endif 

{

#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->PopUpWarningWindow(msg);

#endif
}

//-----------------------------------------------------------------------------
void vtkSlicerCommonInterface::SaveSettingToMRML()
{
#ifdef Slicer3_USE_KWWIDGETS

  // Slicer3
  vtkEMSegmentKWDynamicFrame* dynFrame = this->GetSlicer3DynamicFrame();

  if (!dynFrame)
    {
    vtkErrorMacro("Could not get the EMSegmenter Dynamic Frame!")
    return;
    }

  dynFrame->SaveSettingToMRML();

#endif
}

#ifndef Slicer3_USE_KWWIDGETS
void vtkSlicerCommonInterface::resetReturnChar() 
{
  if (this->returnChar) 
    {
      delete[] this->returnChar;
      this->returnChar = NULL;
    }
} 
#endif
