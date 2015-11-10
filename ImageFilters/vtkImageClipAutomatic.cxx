#include "vtkImageClipAutomatic.h"

#include "vtkCellData.h"
#include "vtkExtentTranslator.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

#if VTK_MAJOR_VERSION > 5
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#endif


//----------------------------------------------------------------------------
vtkImageClipAutomatic::vtkImageClipAutomatic()
{
  this->Initialized = 0;

  this->OutputWholeExtent[0] =
  this->OutputWholeExtent[2] =
  this->OutputWholeExtent[4] = -100000;

  this->OutputWholeExtent[1] =
  this->OutputWholeExtent[3] =
  this->OutputWholeExtent[5] = 100000;
}

//------------------------------------------------------------------------------
vtkImageClipAutomatic* vtkImageClipAutomatic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageClipAutomatic");
  if(ret) return (vtkImageClipAutomatic*)ret;
  // If the factory was unable to create the object, then create it here.
  return new vtkImageClipAutomatic;
}

//----------------------------------------------------------------------------
void vtkImageClipAutomatic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  int idx;
  
  os << indent << "OutputWholeExtent: (" << this->OutputWholeExtent[0]
     << "," << this->OutputWholeExtent[1];
  for (idx = 1; idx < 3; ++idx)
    {
    os << indent << ", " << this->OutputWholeExtent[idx * 2]
       << "," << this->OutputWholeExtent[idx*2 + 1];
    }
  os << ")\n";
}
  
//----------------------------------------------------------------------------
void vtkImageClipAutomatic::SetOutputWholeExtent(int extent[6])
{
  int idx;
  int modified = 0;
  
  for (idx = 0; idx < 6; ++idx)
    {
    if (this->OutputWholeExtent[idx] != extent[idx])
      {
      this->OutputWholeExtent[idx] = extent[idx];
      modified = 1;
      }
    }
  this->Initialized = 1;
  if (modified)
    {
    this->Modified();
    vtkImageData *output = this->GetOutput();
    if (output)
      {
#if VTK_MAJOR_VERSION <= 5
       output->SetUpdateExtent(extent);
#else
       output->SetExtent(extent);
#endif
      }
    }
}

//----------------------------------------------------------------------------
void vtkImageClipAutomatic::SetOutputWholeExtent(int minX, int maxX, 
                                             int minY, int maxY,
                                             int minZ, int maxZ)
{
  int extent[6];
  
  extent[0] = minX;  extent[1] = maxX;
  extent[2] = minY;  extent[3] = maxY;
  extent[4] = minZ;  extent[5] = maxZ;
  this->SetOutputWholeExtent(extent);
}


//----------------------------------------------------------------------------
// void vtkImageClipAutomatic::GetOutputWholeExtent(int extent[6])
// {
//   int idx;
//   
//   for (idx = 0; idx < 6; ++idx)
//     {
//     extent[idx] = this->OutputWholeExtent[idx];
//     }
// }


//----------------------------------------------------------------------------
// Change the WholeExtent
#if VTK_MAJOR_VERSION <= 5
void vtkImageClipAutomatic::ExecuteInformation(vtkImageData *inData, 
                                      vtkImageData *outData)
{
  int idx, extent[6];
  
  inData->GetWholeExtent(extent);
  if ( ! this->Initialized)
    {
    this->SetOutputWholeExtent(extent);
    }

  // Clip the OutputWholeExtent with the input WholeExtent
  for (idx = 0; idx < 3; ++idx)
    {
    if (this->OutputWholeExtent[idx*2] >= extent[idx*2] && 
        this->OutputWholeExtent[idx*2] <= extent[idx*2+1])
      {
      extent[idx*2] = this->OutputWholeExtent[idx*2];
      }
    if (this->OutputWholeExtent[idx*2+1] >= extent[idx*2] && 
        this->OutputWholeExtent[idx*2+1] <= extent[idx*2+1])
      {
      extent[idx*2+1] = this->OutputWholeExtent[idx*2+1];
      }
    // make usre the order is correct
    if (extent[idx*2] > extent[idx*2+1])
      {
      extent[idx*2] = extent[idx*2+1];
      }
      }
  
  outData->SetWholeExtent(extent);
}
#endif

//----------------------------------------------------------------------------
// Sets the output whole extent to be the input whole extent.
#if VTK_MAJOR_VERSION <= 5
void vtkImageClipAutomatic::ResetOutputWholeExtent()
{
  if ( ! this->GetInput())
    {
    vtkWarningMacro("ResetOutputWholeExtent: No input");
    return;
    }

  this->GetInput()->UpdateInformation();
  this->SetOutputWholeExtent(this->GetInput()->GetWholeExtent());
}
#endif 


template <class T>
void DetermineBoundingBox(T *source, vtkIdType* Inc, int *Ext, int* MinBound,int* MaxBound) { 
  MinBound[2] = Ext[5];  MinBound[1] = Ext[3];  MinBound[0] = Ext[1];
  MaxBound[2] = Ext[4];  MaxBound[1] = Ext[2];  MaxBound[0] = Ext[0];
  int FlagZ, FlagY; 
  T value =  *source;
  for (int z = Ext[4]; z <= Ext[5] ; z ++) {
    FlagZ = 0;
    for (int y = Ext[2]; y <= Ext[3] ; y ++) {
      FlagY = 0;
      for (int x = Ext[0]; x <= Ext[1] ; x ++) {
    if (*source != value) {
      FlagY = FlagZ = 1;
      if (MinBound[0] > x) MinBound[0] = x;
      if (MaxBound[0] < x) MaxBound[0] = x;
    }
    source ++;
      }
      if (FlagY) {
    if (MinBound[1] > y) MinBound[1] = y;
    if (MaxBound[1] < y) MaxBound[1] = y;
      }
      source += Inc[1];
    }
    if (FlagZ) {
      if (MinBound[2] > z) MinBound[2] = z;
      MaxBound[2] = z;
    }
    source += Inc[2];
  }
  // cout << "BoundingBox: "<< MinBound[0] << " " << MaxBound[0] << " " << MinBound[1] << " " << MaxBound[1] << " " << MinBound[2] << " " << MaxBound[2] << endl;
}

void vtkImageClipAutomatic::DetermineOutputWholeExtent(vtkImageData *inData, int *MinBound,int *MaxBound) {
  if (!inData) {
      vtkWarningMacro("ResetOutputWholeExtent: No input");
      return;
  }

  if (inData->GetNumberOfScalarComponents() != 1) {
     vtkErrorMacro("Number Of Scalar Componentsfor Input has to be 1.");
     return;
  }
#if VTK_MAJOR_VERSION <= 5
  inData->Update();
#endif

  int inExt[6];
  vtkIdType inInc[3];
#if VTK_MAJOR_VERSION <= 5
  inData->GetWholeExtent(inExt);
#else
 inData->GetExtent(inExt);
#endif 
  inData->GetContinuousIncrements(inExt, inInc[0], inInc[1], inInc[2]);
  void* inPtr = inData->GetScalarPointerForExtent(inExt);

  switch (inData->GetScalarType()) {
    vtkTemplateMacro(DetermineBoundingBox((VTK_TT*) inPtr, inInc, inExt, MinBound, MaxBound));
  default: 
      vtkErrorMacro("Execute: Unknown ScalarType");
      return;
  }
}

void vtkImageClipAutomatic::DetermineOutputWholeExtent(vtkImageData *inData, int &MinBoundX,  int &MaxBoundX, int &MinBoundY, int &MaxBoundY, int &MinBoundZ, int &MaxBoundZ) {
  int MinBound[3];
  int MaxBound[3];
  this->DetermineOutputWholeExtent(inData,MinBound,MaxBound);
  MaxBoundX =  MaxBound[0];
  MaxBoundY =  MaxBound[1];
  MaxBoundZ =  MaxBound[2];
  MinBoundX =  MinBound[0];
  MinBoundY =  MinBound[1];
  MinBoundZ =  MinBound[2];
}

void vtkImageClipAutomatic::DetermineOutputWholeExtent(vtkImageData *inData, vtkIntArray *Min, vtkIntArray *Max) {
  this->DetermineOutputWholeExtent(inData,Min->GetPointer(0),Max->GetPointer(0));
}


#if VTK_MAJOR_VERSION <= 5
void vtkImageClipAutomatic::DetermineOutputWholeExtent() {
  int MinBound[3];
  int MaxBound[3];
  this->ResetOutputWholeExtent();
  this->DetermineOutputWholeExtent(this->GetInput(),MinBound,MaxBound);
  this->SetOutputWholeExtent(MinBound[0],MaxBound[0],MinBound[1],MaxBound[1],MinBound[2],MaxBound[2]);
}
#endif


//----------------------------------------------------------------------------
// This method simply copies by reference the input data to the output.
#if VTK_MAJOR_VERSION <= 5
void vtkImageClipAutomatic::ExecuteData(vtkDataObject *)
{
  vtkImageData *outData = this->GetOutput();
  vtkImageData *inData = this->GetInput();
   vtkDebugMacro(<<"Executing image clip");

  int *inExt  = inData->GetExtent(); 
  //outData->SetExtent(inExt);
  //outData->GetPointData()->PassData(inData->GetPointData());
  //outData->GetCellData()->PassData(inData->GetCellData());

#else
int vtkImageClipAutomatic::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the input
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *inData = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  // get the output
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *outData = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  this->AllocateOutputData(outData, outInfo);

  int inExt[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), inExt);
  int outExt[6];
  outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), outExt);
#endif

  outData->SetExtent(inExt);
  outData->GetPointData()->PassData(inData->GetPointData());
  outData->GetCellData()->PassData(inData->GetCellData());
  outData->Crop(inExt); 
#if VTK_MAJOR_VERSION <= 5
     return;
#else
     return 0;
#endif

}

