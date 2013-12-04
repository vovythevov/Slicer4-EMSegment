#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"
#include "vtkEMSegmentMRMLManager.h"

int main(int argc, char** argv)
{
  std::cerr << "Starting EM read/write MRML test..." << std::endl;
  std::string mrmlSceneFilenameInput;
  std::string mrmlSceneFilenameOutput;
  std::string removeFlag;
  bool success = true;

  //
  // parse command line
  if (argc < 3)
  {
    std::cerr 
      << "Usage: vtkEMSegmentReadWriteMRMLTest"      << std::endl
      <<         "<mrmlSceneFilenameInput>"          << std::endl
      <<         "<mrmlSceneFilenameOutput>"         << std::endl
      <<         "<0/1 = Remove EM Related Nodes"    << std::endl
      << std::endl;
    return EXIT_FAILURE;
  }

  mrmlSceneFilenameInput           = argv[1];
  mrmlSceneFilenameOutput          = argv[2];
  removeFlag                       = argv[3];

  //
  // create a mrml scene that will hold the data parameters
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  mrmlScene->SetURL(mrmlSceneFilenameInput.c_str());

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetModuleName("EMSegment");
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->SetMRMLScene(mrmlScene);
  emLogic->InitializeEventListeners();

  emLogic->RegisterMRMLNodesWithScene();

  try 
  {
    mrmlScene->Import();
    if (!removeFlag.compare("1"))
      {    
        emLogic->GetMRMLManager()->SetLoadedParameterSetIndex(0);
        emLogic->RemoveTaskAndTempFiles();
      }
    mrmlScene->Commit(mrmlSceneFilenameOutput.c_str());
  }
  catch (...)
  {
    std::cerr << "Error: " << std::endl;
    success = false;
  }

  if (success)
    {
      std::cerr << "Read/Wrote: " << mrmlScene->GetNumberOfNodes()
                << " nodes." << std::endl;
    }

  // clean up
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  
  return success ? EXIT_SUCCESS : EXIT_FAILURE;    
}

