#include "vtkSlicerCommonInterface.h"
#include "vtkEMSegmentLogic.h"
#include <vtksys/SystemTools.hxx>
// EMSegment/MRML includes
#include "vtkEMSegmentMRMLManager.h"
#include "vtkMRMLEMSGlobalParametersNode.h"

// EMSegment/CLI includes
#include "EMSegmentAPIHelper.h"
#include "EMSegmentCommandLineFct.h"

// Colors/Logic includes
// #include <vtkSlicerColorLogic.h>

// test the tcl adapter provided by the Slicer Common Interface
int main(int argc, char** argv)
{
  //
  // ==== Parse CommandLine ================
  //
  if (argc < 4)
  {
    std::cerr 
      << "Usage: vtkSlicerCommonInterfaceTest1"      << std::endl
      <<         "<EMSegment Source Dir>"          << std::endl
      <<         "<Tcl File>"          << std::endl
      <<         "<Volume File>"         << std::endl
      << std::endl;
    return EXIT_FAILURE;
  }

  std::string emSourceDir                = argv[1];
  std::string tclFile                  = argv[2];
  std::string input          = argv[3];

  //
  // ==== Initialize Tcl Interface ================
  //
  vtkSlicerCommonInterface* slicerCommon = vtkSlicerCommonInterface::New();
  slicerCommon->Startup(argc,argv,&cout);
  std::string appTcl = std::string(slicerCommon->GetApplicationTclName());
  if (appTcl.size() != 8)
    {
    std::cerr << "The Tcl Name for the Application has an invalid size!";
    return EXIT_FAILURE;
    }

  std::ostringstream os;
  os << "namespace eval slicer3 set Application ";
  os << appTcl;
  slicerCommon->EvaluateTcl(os.str().c_str());

  //
  // ==== Create MRML Scene  ================
  //
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  slicerCommon->RegisterObjectWithTcl(mrmlScene,"MRMLScene");
  
  //
  // ==== Create EMSegmenter  ===============
  // 
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->SetMRMLScene(mrmlScene);
  emLogic->InitializeEventListeners();
  emLogic->RegisterNodes();

  // get the tcl name of EMLogic
  std::string EMSLogicTcl = vtksys::SystemTools::DuplicateString(slicerCommon->GetTclNameFromPointer(emLogic));

  if (EMSLogicTcl.size() != 8)
    {
    std::cerr << "The Tcl Name for EMSLogic has an invalid size!";
    return EXIT_FAILURE;
    }

  std::string tclCommand = "set emLogic " + std::string(EMSLogicTcl);
  slicerCommon->EvaluateTcl(tclCommand.c_str());

  tclCommand = std::string("set emSourceDir ") + emSourceDir ;
  slicerCommon->EvaluateTcl(tclCommand.c_str());

  tclCommand = std::string("set input ") + input ;
  slicerCommon->EvaluateTcl(tclCommand.c_str());

  //
  // ==== Run Tcl Script  ===============
  // 
  cout << "==================== Start =================" << endl;
  cout << "Sourcing " << tclFile.c_str() << endl;
  tclCommand =  std::string("source [file join ") + emSourceDir + std::string(" " ) + tclFile + std::string("]");
  slicerCommon->EvaluateTcl(tclCommand.c_str());
  cout << "==================== END =================" << endl;

  //emSourceDir
  // ==== Clean Up  ===============
  // 
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();

  mrmlScene->Clear(true);
  mrmlScene->Delete();

  slicerCommon->DestroySlicerApplication();
  slicerCommon->Delete();

  return EXIT_SUCCESS;

}
