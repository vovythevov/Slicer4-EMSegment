// .NAME vtkImageClipAutomatic - Reduces the image extent of the input.
// .SECTION Description
// vtkImageClipAutomaticBound  will make an image smaller by finding a ox around the iamges and than cliping it off 
// you can delete the input afterards if you want to 
// 2: If ClipDataOn is set, then you will get no more that the clipped
// extent.
#ifndef __vtkImageClipAutomatic_h
#define __vtkImageClipAutomatic_h
#include "vtkEMSegment.h"

// I did not make this a subclass of in place filter because
// the references on the data do not matter. I make no modifications
// to the data.
// VTK includes
#if VTK_MAJOR_VERSION <= 5
#include "vtkImageToImageFilter.h"
#else
#include <vtkImageAlgorithm.h>
#endif

#include "vtkIntArray.h"

class VTK_EMSEGMENT_EXPORT  vtkImageClipAutomatic
#if VTK_MAJOR_VERSION <= 5
  : public vtkImageToImageFilter
#else
  : public vtkImageAlgorithm
#endif
{
public:
  static vtkImageClipAutomatic *New();
#if VTK_MAJOR_VERSION <= 5
  vtkTypeMacro(vtkImageClipAutomatic,vtkImageToImageFilter);
#else
  vtkTypeMacro(vtkImageClipAutomatic,vtkImageAlgorithm);
#endif

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Call this function before updating the filter;
#if VTK_MAJOR_VERSION <= 5
  void DetermineOutputWholeExtent();
#endif
  // Desciption:
  // This function can be called just to find out the maximum 
  void DetermineOutputWholeExtent(vtkImageData *inData, int &MinBoundX,  int &MaxBoundX, int &MinBoundY, int &MaxBoundY, int &MinBoundZ, int &MaxBoundZ); 
  void DetermineOutputWholeExtent(vtkImageData *inData, vtkIntArray *Min, vtkIntArray *Max); 
  

  // Description:
  // The whole extent of the output has to be set explicitly.
  vtkGetVector6Macro(OutputWholeExtent, int); 

protected:
  vtkImageClipAutomatic();
  ~vtkImageClipAutomatic() {};

  // Time when OutputImageExtent was computed.
  vtkTimeStamp CTime;
  int Initialized; // Set the OutputImageExtent for the first time.
  int OutputWholeExtent[6];
  int ClipData;
  
  void CopyData(vtkImageData *inData, vtkImageData *outData, int *ext);

  int SplitExtentTmp(int piece, int numPieces, int *ext);

#if VTK_MAJOR_VERSION <= 5
  virtual void ExecuteData(vtkDataObject *out);
  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
#else
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
#endif


  void DetermineOutputWholeExtent(vtkImageData *inData, int *MinBound,int *MaxBound);

private:
  vtkImageClipAutomatic(const vtkImageClipAutomatic&);  // Not implemented.
  void operator=(const vtkImageClipAutomatic&);  // Not implemented.

  void SetOutputWholeExtent(int extent[6]);
  void SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, int minZ, int maxZ);
#if VTK_MAJOR_VERSION <= 5
  void ResetOutputWholeExtent();
#endif  

};

#endif



