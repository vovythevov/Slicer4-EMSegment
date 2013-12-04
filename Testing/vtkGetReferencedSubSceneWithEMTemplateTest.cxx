#include "vtkEMSegmentLogic.h"
#include "vtkMRMLEMSTemplateNode.h"
#include "vtkMRMLEMSAtlasNode.h"
#include <vtkMRMLScene.h>

// STD includes
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

// Tests the vtkMRMLScene GetReferencedSubScene function with the vtkMRMLEMSTemplateNode by first loading in a mrml scene with a template node, 
// calling the function, and then checking if the template node was correctly copied. I created the test as it is used by vtkEMSegmentMRMLManager::CopyEMRelatedNodesToMRMLScene
// which is used by vtkEMSegmentLogic::PackageAndWriteData , which is called  vtkEMSegmentLogic::SaveIntermediateResults, which is test by EMSegCL_IntermediateResults

void TestMRMLScene(vtkMRMLScene* scene )
{
  vtkMRMLEMSTemplateNode* templateNode = vtkMRMLEMSTemplateNode::SafeDownCast(
    scene->GetNthNodeByClass(0, "vtkMRMLEMSTemplateNode"));
  if (templateNode == NULL)
    {
    std::cerr << "Error: No Template node in new scene" << std::endl;
    throw std::runtime_error("Parameters Not Found!");
    }

  // check atlas node - atlas is part of the template -if it is not it - then template was not correctly created 
  vtkMRMLEMSAtlasNode *atlasNode = templateNode->GetSpatialAtlasNode();
  if (atlasNode == NULL)
    {
    std::cerr << "Error: Template node does not have correct structure" << std::endl;
    throw std::runtime_error("Template structure not complete!");
    }
}

//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  if (argc < 2)
    {
    std::cout << "Usage: "
              << "vtkGetReferenceSubSceneWithEMTemplateTest path/to/scene.mrml"
              << std::endl;
    return EXIT_FAILURE;
    }
  std::string mrmlSceneFilename           = argv[1];

  bool passFlag = true;

  // create a mrml scene that will hold the data parameters

  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  mrmlScene->SetURL(mrmlSceneFilename.c_str());
  vtkMRMLScene* newScene =  vtkMRMLScene::New();

  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetModuleName("EMSegment");
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->SetMRMLScene(mrmlScene);
  emLogic->InitializeEventListeners();
  emLogic->RegisterMRMLNodesWithScene();

  try
    {
    cout << "====================================" << endl; 
    cout << "Load and Check input " << endl; 

    try
      {
      mrmlScene->Import();
      std::cout << "Imported: " << mrmlScene->GetNumberOfNodes() << " nodes." << std::endl;
      }
    catch (...)
      {
      std::cerr << "Error reading/setting mrml scene: " << std::endl;
      throw;
      }

    TestMRMLScene(mrmlScene);
    std::cout << "... OK" << endl;

    cout << "====================================" << endl;
    cout << "Checking GetReferencedSubScene ... " << endl;

    vtkMRMLEMSTemplateNode* templateNode =
      vtkMRMLEMSTemplateNode::SafeDownCast(
        mrmlScene->GetNthNodeByClass(0, "vtkMRMLEMSTemplateNode"));
    // Copy Reference Scene - same as in CopyEMRelatedNodesToMRMLScene
    mrmlScene->GetReferencedSubScene(templateNode, newScene);
    TestMRMLScene(newScene);
    cout << "... Copy function works " << endl;

    }
  catch(...)
    {
    passFlag = false;
    }

   // clean up
   emLogic->SetAndObserveMRMLScene(NULL);
   emLogic->Delete();

   newScene->Clear(true);
   newScene->Delete();

   mrmlScene->Clear(true);
   mrmlScene->Delete();

  return (passFlag ?  EXIT_SUCCESS : EXIT_FAILURE);
}

