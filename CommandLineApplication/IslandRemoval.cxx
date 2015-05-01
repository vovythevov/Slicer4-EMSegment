/*=auto=========================================================================

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date: 2014-05-02 14:49:48 -0700 (Fri, 02 May 2014) $
Version:   $Revision: 23121 $

=========================================================================auto=*/

#include "IslandRemovalCLP.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkITKImageWriter.h"
#include <vtksys/SystemTools.hxx>
#include "vtkImageIslandFilter.h" 
#include "vtkDebugLeaks.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"

#define VTK_CREATE(type, var) \
  vtkSmartPointer<type> var = vtkSmartPointer<type>::New();

int main(int argc, char * argv[])
{
  PARSE_ARGS;
  vtkDebugLeaks::SetExitError(true);

  // Read the file
  if (!vtksys::SystemTools::FileExists(InputVolume.c_str()))
        {
          std::cerr << "Error: " << InputVolume.c_str() << " does not exist."
                    << std::endl;
          return EXIT_FAILURE;
        }

  VTK_CREATE(vtkITKArchetypeImageSeriesScalarReader,reader)
  reader->SetArchetype(InputVolume.c_str());
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  reader->SetUseNativeOriginOn();
  reader->Update();

#if (VTK_MAJOR_VERSION <= 5)
  if (! reader->GetOutput())
#else
  if (! reader->GetOutputDataObject(0))
#endif
    {
          std::cerr << "Error: Could not read " << InputVolume.c_str() << " !"
                    << std::endl;
          return EXIT_FAILURE;
    }

  std::cout << "Done reading the file " << InputVolume << endl;

  if (MinIslandSize < 2)
    {
       std::cerr << "Error: Nothing to do as MinIslandSize < 2 !" 
                 << std::endl;
       return EXIT_FAILURE;
    }

   VTK_CREATE(vtkImageIslandFilter, islandFilter);
#if VTK_MAJOR_VERSION <= 5
   islandFilter->SetInput(reader->GetOutput());
#else
   // islandFilter->SetInputData(reader->GetOutputDataObject(0));
  islandFilter->SetInputConnection(reader->GetOutputPort());
#endif
   islandFilter->SetIslandMinSize(MinIslandSize);
   if (Neighborhood2D)
      {
        islandFilter->SetNeighborhoodDim2D();
        std::cout << "2D Neighborhood Island activated" << std::endl;
      }
   else
      {
         islandFilter->SetNeighborhoodDim3D();
      }
   islandFilter->SetPrintInformation(1);
   islandFilter->Update();
  
   VTK_CREATE(vtkITKImageWriter, export_iwriter);
#if VTK_MAJOR_VERSION <= 5
   export_iwriter->SetInput(islandFilter->GetOutput());
#else
  export_iwriter->SetInputConnection(islandFilter->GetOutputPort());
#endif
  export_iwriter->SetFileName(OutputVolume.c_str());
  VTK_CREATE(vtkMatrix4x4, mat);
  export_iwriter->SetRasToIJKMatrix(mat);
  export_iwriter->SetUseCompression(1);
  export_iwriter->Write();
  cout << "Warning : Header information is not preserved" << endl;

  return EXIT_SUCCESS;
}
