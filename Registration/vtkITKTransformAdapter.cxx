#include "vtkITKTransformAdapter.h"
#include "vtkObjectFactory.h"

#if VTK_MAJOR_VERSION <= 5
vtkCxxRevisionMacro(vtkITKTransformAdapter, "$Revision: 1.30 $");
#endif
vtkStandardNewMacro(vtkITKTransformAdapter);

void vtkITKTransformAdapter::PrintSelf(ostream& os, vtkIndent indent) 
{
  Superclass::PrintSelf(os,indent);
  os << "ITKTransform: (not implemented)" 
     << std::endl;
}
