/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMLocalClass.cxx,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#include "vtkImageEMLocalClass.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//------------------------------------------------------------------------
vtkImageEMLocalClass* vtkImageEMLocalClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMLocalClass");
  if(ret)
  {
    return (vtkImageEMLocalClass*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMLocalClass;

}


//----------------------------------------------------------------------------
vtkImageEMLocalClass::vtkImageEMLocalClass() { 
  this->LogMu                   = NULL;
  this->LogCovariance           = NULL;

  this->ProbDataWeight          = 0.0;
  
  this->PCAEigenVectorImageData = NULL;
  this->PCAMeanShapeImageData   = NULL;

  this->PCAShapeParameters  = NULL;
  this->PCANumberOfEigenModes = 0; 
  this->PCAEigenValues      = NULL;

  this->ReferenceStandardPtr= NULL;

  // Printing functions 
  this->PrintQuality        = 0;
  this->PrintPCA            = 0;

  this->PCALogisticSlope    = 1.0;
  this->PCALogisticBoundary = 9.5;
  this->PCALogisticMin      = 0.0;
  this->PCALogisticMax      = 20.0;

#if VTK_MAJOR_VERSION <= 5
  // Important to set it - otherwise set to 1 by default  
  // I disabled it bc if no image data is attached to this class 
  // => NumberOfInputs = 0 => Update is disabled.
  // this->vtkProcessObject::SetNumberOfInputs(0); 
#endif
}


//------------------------------------------------------------------------------
void vtkImageEMLocalClass::DeleteClassVariables(){
  if (this->LogCovariance) {
    for (int y=0; y < this->NumInputImages; y++) {
      delete[] this->LogCovariance[y];
    }
    delete[] this->LogCovariance;
  }
  
  if (this->LogMu) delete[] this->LogMu;
  
  this->LogCovariance       = NULL;
  this->LogMu               = NULL;

  this->ProbImageData       = NULL;
  this->ReferenceStandardPtr= NULL;
  this->DeletePCAParameters();
}

//------------------------------------------------------------------------------
void vtkImageEMLocalClass::DeletePCAParameters()  {
      if (this->PCAShapeParameters) delete[] this->PCAShapeParameters;
      this->PCAShapeParameters  = NULL;

      if (this->PCAEigenVectorImageData) delete[] this->PCAEigenVectorImageData;
      this->PCAEigenVectorImageData = NULL;

      if (this->PCAEigenValues) delete[] this->PCAEigenValues;
      this->PCAEigenValues = NULL;

      this->PCANumberOfEigenModes  = 0;
}

//----------------------------------------------------------------------------
void vtkImageEMLocalClass::PrintSelf(ostream& os,vtkIndent indent) { 
  int x,y;
  os << indent << "------------------------------------------ CLASS ----------------------------------------------" << endl;
  this->vtkImageEMLocalGenericClass::PrintSelf(os,indent);  


  os << indent << "LogMu:                   ";
  for (x= 0 ; x < this->NumInputImages; x ++) os << this->LogMu[x] << " ";
  os<< endl;

  os << indent << "LogCovariance:           ";
  for (y= 0 ; y < this->NumInputImages; y ++) {
     for (x= 0; x < this->NumInputImages; x++)  os << this->LogCovariance[y][x] << " " ;
     if ( y < (this->NumInputImages-1)) os<< "| ";
  }
  os<< endl;

  os << indent << "ReferenceStandardPtr:    ";
  if (this->ReferenceStandardPtr) {
     os << this->ReferenceStandardPtr << endl;
  } else {os << "(None)" << endl;}

  os << indent << "PrintQuality:            " << this->PrintQuality << endl;  
  os << indent << "PrintPCA:                " << this->PrintPCA     << endl;

  this->PrintPCAParameters(os, indent);
}


//------------------------------------------------------------------------------
void vtkImageEMLocalClass::PrintPCAParameters(ostream& os,vtkIndent indent) {

  os << indent << "PCANumberOfEigenModes:  " << this->PCANumberOfEigenModes << endl;
  os << indent << "PCALogisticSlope:        " << this->PCALogisticSlope << endl;
  os << indent << "PCALogisticBoundary:     " << this->PCALogisticBoundary << endl;
  os << indent << "PCALogisticMin:          " << this->PCALogisticMin << endl;
  os << indent << "PCALogisticMax:          " << this->PCALogisticMax << endl;
  
  if (this->PCANumberOfEigenModes) {
    os << indent << "PCAEigenValues     : ";
    for (int x = 0; x <  this->PCANumberOfEigenModes; x++ ) os << this->PCAEigenValues[x] << " " ;
    os << endl;

    os << indent << "PCAMeanShape :   " << endl; 
    if (this->PCAMeanShapeImageData) this->PCAMeanShapeImageData->PrintSelf(os,indent.GetNextIndent());
    for (int x = 0; x <  this->PCANumberOfEigenModes; x++ ) {
      os << indent << "PCAEigenVector" << x << ":   " << endl; 
      if (this->PCAEigenVectorImageData[x]) this->PCAEigenVectorImageData[x]->PrintSelf(os,indent.GetNextIndent());
    }
  }
}


//------------------------------------------------------------------------------
void vtkImageEMLocalClass::SetNumInputImages(int number) {
  int OldNumber = this->NumInputImages;
#if VTK_MAJOR_VERSION <= 5
  this->vtkImageEMLocalGenericClass::SetNumInputImages(number);
#else
  this->Superclass::SetNumInputImages(number);
#endif
  if (OldNumber == number) return;
  this->DeleteClassVariables();

  if (number > 0 ) {
    // Create new space for variables
    this->LogMu               = new double[number];
    this->LogCovariance       = new double*[number];
    for (int z=0; z < number; z++) {
      this->LogCovariance[z] = new double[number];
    }
    for (int z= 0; z < number; z++) {
    this->LogMu[z] = -1;
    memset(this->LogCovariance[z], 0,number*sizeof(double));
    }
  } 
}

//------------------------------------------------------------------------------
void vtkImageEMLocalClass::SetLogMu(double mu, int x){
  if ((x<0) || (x >= this->NumInputImages) || (mu < 0)) {
    vtkEMAddErrorMessage("Error:vtkImageEMLocalClass::SetLogMu: Incorrect input");
    return;
  }
  this->LogMu[x] = mu;
}

//------------------------------------------------------------------------------
void vtkImageEMLocalClass::SetLogCovariance(double value, int y, int x){
  if ((y<0) || (y >= this->NumInputImages) || (x<0) || (x >= this->NumInputImages)) {
    vtkEMAddErrorMessage("Error:vtkImageEMLocalClass::SetLogCovariance: Incorrect input");
    return;
  }
  this->LogCovariance[y][x] = value;
}

//----------------------------------------------------------------------------
// PCA Stuff 
//----------------------------------------------------------------------------
void vtkImageEMLocalClass::SetPCANumberOfEigenModes(int init)
{
  // std::cerr << "EMLocalClass::SetPCANumberOfEigenModes : Currently : " << this->PCANumberOfEigenModes << " New One: " << init << endl; 
  if (this->PCANumberOfEigenModes != init)
    {
    this->DeletePCAParameters();
    this->PCANumberOfEigenModes =  init;
    this->PCAShapeParameters = new float[this->PCANumberOfEigenModes];
    memset(this->PCAShapeParameters,0,sizeof(float)*this->PCANumberOfEigenModes);
    this->PCAEigenVectorImageData  = new vtkImageData*[this->PCANumberOfEigenModes];
    this->PCAEigenValues     = new double[this->PCANumberOfEigenModes];
    }
}

//----------------------------------------------------------------------------
void vtkImageEMLocalClass::SetPCAEigenVector(vtkImageData *image, int index)
{
  // std::cerr << "vtkImageEMLocalClass::SetPCAEigenVector" << endl;
  // \fixme: index should be [0, PCANumberOfEigenModes[
  if (index < 1 || index > this->PCANumberOfEigenModes )
    {
    vtkEMAddErrorMessage("Error:SetPCAEigenVector: index has to be greater 0"
                         " and not greater than NumberOfEigenModes("
                         << this->PCANumberOfEigenModes << ")");
    return;
    }
  int port = PCAMeanShapeInputPort+index;
#if VTK_MAJOR_VERSION <= 5
  this->SetInput(port,image);
#else
  this->SetNumberOfInputPorts(
    std::max(this->GetNumberOfInputPorts(), port+1));
  this->SetInputData(port,image);
#endif
}

//----------------------------------------------------------------------------
void vtkImageEMLocalClass::SetPrintQuality(int init) { 
    if ( (init < 0)  || (init > EMSEGMENT_NUM_OF_QUALITY_MEASURE)) {
      vtkEMAddErrorMessage("The parameter for PrintQuality has be between 0 and "<< EMSEGMENT_NUM_OF_QUALITY_MEASURE - 1 << "!");
      return;
    }
    this->PrintQuality = init;
}

//----------------------------------------------------------------------------
// Definition of  DataTypeIndex 
// 1   = probability map
// 2   = Mean Shape 
// > 2 = PCA Eigenvecotrs

int vtkImageEMLocalClass::CheckAndAssignPCAImageData(vtkImageData *inData, int DataTypeIndex) {
   // For Mean Shape and EigenVectors we currently only accepts float
  if (this->CheckInputImage(inData, VTK_FLOAT, DataTypeIndex)) return 0;
  if (DataTypeIndex >= PCAEigenVectorFirstInputPort)
    this->PCAEigenVectorImageData[DataTypeIndex - PCAEigenVectorFirstInputPort] = inData;
  else
    this->PCAMeanShapeImageData = inData;
  return 1;
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
#if VTK_MAJOR_VERSION <= 5
void vtkImageEMLocalClass::ExecuteData(vtkDataObject *)
#else
int vtkImageEMLocalClass::RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector)
#endif
{
  // std::cerr << "Start vtkImageEMLocalClass::ExecuteData " << endl;
  // std::cerr << "PCANumberOfEigenModes " << this->PCANumberOfEigenModes << endl;

   vtkDebugMacro(<<"ExecuteData()");

   // ==================================================
   // Initial Check Parameters
   this->ResetWarningMessage();

   if (this->GetErrorFlag()) 
     vtkEMAddWarningMessage("The error flag for this module was set with the following messages (the error messages will be reset now:\n"<<this->GetErrorMessages());

   this->ResetErrorMessage();
#if VTK_MAJOR_VERSION <= 5
   this->vtkImageEMLocalGenericClass::ExecuteData(NULL);
#else
   this->Superclass::RequestData(request, inputVector, outputVector);
#endif
   if (this->GetErrorFlag())
     {
#if VTK_MAJOR_VERSION <= 5
     return;
#else
     return 0;
#endif
     }

   if (this->Label < 0) {
     vtkEMAddErrorMessage("Not all labels are defined for the classes");
#if VTK_MAJOR_VERSION <= 5
     return ;
#else
     return 0;
#endif
   } 
   for (int j = 0; j < this->NumInputImages; j++) {
     // Check Mu
     if (this->LogMu[j] < 0) {
       vtkEMAddErrorMessage("Mu[" << j <<"] = " << this->LogMu[j] << " for class with label " << this->Label <<" must be greater than 0!");
#if VTK_MAJOR_VERSION <= 5
       return;
#else
       return 0;
#endif
     } 
     for (int k=j+1; k <   NumInputImages; k++) {
     if (fabs(this->LogCovariance[j][k] - this->LogCovariance[k][j]) > 0.000001) 
         {   
           vtkEMAddErrorMessage("Covariance must be symetric for class with label " << this->Label);
           vtkIndent indent;
           this->PrintSelf(cout,indent);
#if VTK_MAJOR_VERSION <= 5
           return;
#else
           return 0;
#endif
         }
     }
   }

   // ==================================================
   // Define Parameters

#if VTK_MAJOR_VERSION <= 5
   // First input (input[0]) is a fake
   int NumberOfRealInputData = this->GetNumberOfInputs() - FakeInputPorts;
#else
   int NumberOfRealInputData = this->GetNumberOfInputPorts() - FakeInputPorts;
#endif
   if (this->PCANumberOfEigenModes && (this->PCANumberOfEigenModes + PCAMeanShapeInputPort > NumberOfRealInputData ))
     {
     vtkEMAddErrorMessage("Number of Eingemodes is "<< this->PCANumberOfEigenModes
                          << " but only " << NumberOfRealInputData - PCAMeanShapeInputPort << " were defined!" );
#if VTK_MAJOR_VERSION <= 5
      return;
#else
      return 0;
#endif
   }


   // No inputs defined we do not need to do here anything
   if (NumberOfRealInputData == 0) {
     if (this->ProbDataWeight > 0.0)
       {
       vtkEMAddErrorMessage("ProbDataWeight > 0.0 and no Probability Map defined !" );
#if VTK_MAJOR_VERSION <= 5
       return;
#else
       return 0;
#endif
       }
#if VTK_MAJOR_VERSION <= 5
     return;
#else
     return 1;
#endif
   }

   // Redefine ImageRelatedClass Parameters   
#if VTK_MAJOR_VERSION <= 5
   vtkImageData **inData  = (vtkImageData **) this->GetInputs();
#endif

   // ================================================== 
   // Load the images
 
   // Check if everything is OK with proabability data
   if (NumberOfRealInputData <= PCAMeanShapeInputPort)
     {
#if VTK_MAJOR_VERSION <= 5
     if (inData[ProbInputPort] == NULL)
       {
#else
     if (this->GetInputDataObject(ProbInputPort,0) == NULL)
       {
#endif
       if (this->ProbDataWeight > 0.0)
         {
         vtkEMAddErrorMessage("ProbDataWeight > 0.0 but no Probability Map defined !" );
#if VTK_MAJOR_VERSION <= 5
         return;
#else
         return 0;
#endif
         }
       else
         {
         vtkEMAddWarningMessage("No probability map is defined for class with Label " << this->Label);
         }
       }
     // Job is done - No PCA data
#if VTK_MAJOR_VERSION <= 5
     return;
#else
     return 1;
#endif
     }
   
   // Check and set PCA Mean Shape    
#if VTK_MAJOR_VERSION <= 5
   if (inData[PCAMeanShapeInputPort])
     {
     if (!this->CheckAndAssignPCAImageData(
           inData[PCAMeanShapeInputPort], PCAMeanShapeInputPort))
#else
   if (this->GetImageDataInput(PCAMeanShapeInputPort))
     {
     if (!this->CheckAndAssignPCAImageData(
           this->GetImageDataInput(PCAMeanShapeInputPort), PCAMeanShapeInputPort))
#endif
       {
#if VTK_MAJOR_VERSION <= 5
       return;
#else
       return 1;
#endif
       }
     }
   else
     {
     vtkEMAddErrorMessage("PCA Eigen Vectors defined but PCA Mean Shape is missing!");
#if VTK_MAJOR_VERSION <= 5
     return;
#else
     return 0;
#endif
     }

   // Check and Read in Eigenvectors
   for (int j = 0 ; j < this->PCANumberOfEigenModes; j++)
     {
#if VTK_MAJOR_VERSION <= 5
     if (inData[j+PCAEigenVectorFirstInputPort])
       {
       if (!this->CheckAndAssignPCAImageData(inData[j+PCAEigenVectorFirstInputPort],
                                             j+PCAEigenVectorFirstInputPort))
#else
     if (this->GetImageDataInput(j+PCAEigenVectorFirstInputPort))
       {
       if (!this->CheckAndAssignPCAImageData(this->GetImageDataInput(j+PCAEigenVectorFirstInputPort),
                                             j+PCAEigenVectorFirstInputPort))
#endif
         {
#if VTK_MAJOR_VERSION <= 5
         return;
#else
         return 1;
#endif
         }
       }
     else
       {
       vtkEMAddErrorMessage(j << ". PCA Eigen Vector is not defined !");
#if VTK_MAJOR_VERSION <= 5
       return;
#else
       return 0;
#endif
       }
     }
#if VTK_MAJOR_VERSION <= 5
  return;
#else
  return 1;
#endif
}

//----------------------------------------------------------------------------
void* vtkImageEMLocalClass::GetPCAEigenVectorPtr(int index, int type) { 
   assert(this->PCAEigenVectorImageData); 
   return this->GetDataPtr(this->PCAEigenVectorImageData[index],type); 
}

//----------------------------------------------------------------------------
void vtkImageEMLocalClass::SetPCAMeanShape(vtkImageData *image)
{
#if VTK_MAJOR_VERSION <= 5
  this->SetInput(PCAMeanShapeInputPort,image);
#else
  this->SetNumberOfInputPorts(
    std::max(this->GetNumberOfInputPorts(), PCAMeanShapeInputPort + 1));
  this->SetInputData(PCAMeanShapeInputPort,image);
#endif
}
