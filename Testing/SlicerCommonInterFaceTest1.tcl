# ------------------------
# General tcl/python functions
proc em_logic  {args} {
      ::tpycl::methodCaller em_logic slicer.modules.emsegment.logic() $args
}

proc slicer_app {args} {
       ::tpycl::methodCaller slicer_app slicer.app $args
} 

# -----------------------------------
# Main 
# -----------------------------------

set em_manager [$emLogic GetMRMLManager] 
set mrmlScene  [$em_manager GetMRMLScene]
set appLogic   [ slicer_app applicationLogic.__call__ ]

$emLogic PrintText "Loading Input: $inputVolumeFileName"
set inputNode [$emLogic AddArchetypeScalarVolume "$inputVolumeFileName" "input" $appLogic $mrmlScene  0]

set outputNode  [$em_manager CreateVolumeScalarNode $inputNode "blubber"]
set outputVolume [$outputNode GetImageData]

# ------------------------------------------------------
$emLogic PrintText "Test 1 : DeepCopy Input" 
$outputVolume DeepCopy [$inputNode GetImageData] 

# ------------------------------------------------------
$emLogic PrintText "Test 2 : Threshold Input and DeepCopy" 
set thresh [vtkImageThreshold New]
if {[$emLogic GetVTKVersion] <= 5  } {
    $thresh SetInput [$inputNode GetImageData]  
} else {
    $thresh SetInputData [$inputNode GetImageData]
}

$thresh Update
$outputVolume DeepCopy [$thresh GetOutput] 

# ------------------------------------------------------
$emLogic PrintText "Test 3 : DeepCopy Ellipsoid" 
set ellips [vtkImageEllipsoidSource New]
$outputVolume DeepCopy [$ellips GetOutput] 

# ------------------------------------------------------
$emLogic PrintText "Test 4 : DeepCopy Threshold again" 
$outputVolume DeepCopy [$thresh GetOutput] 

# Did not catch the error message from vtk
# catch { $outputVolume DeepCopy [$thresh GetOutput] } errMSG
# $emLogic PrintText "--- $errMSG -- "

$thresh Delete
$ellips Delete

# ------------------------------------------------------
$emLogic PrintText "Test 5 : LabelPropagation" 

set voronoi [vtkImageLabelPropagation New]
if {[$emLogic GetVTKVersion] <= 5  } {
  $voronoi SetInput [$inputNode GetImageData] 
} else {
  $voronoi SetInputData [$inputNode GetImageData] 
}
$voronoi Update
$voronoi Delete

# ------------------------------------------------------
$emLogic PrintText "Test 6 : Island Filter" 

set islandFilter [vtkImageIslandFilter New] 
if {[$emLogic GetVTKVersion] <= 5  } {
    $islandFilter SetInput [$inputNode GetImageData] 
} else { 
    $islandFilter SetInputData [$inputNode GetImageData] 
}
$islandFilter SetIslandMinSize 2
$islandFilter SetNeighborhoodDim2D
$islandFilter SetPrintInformation 1
$islandFilter Update

$islandFilter Delete
# ------------------------------------------------------
$emLogic PrintText "Test 7 : PreprocessingBiasFieldCorrection" 
$em_manager CreateAndObserveNewParameterSet 
set outputNode [$emLogic PreprocessingBiasFieldCorrection $inputNode 1]
$emLogic RemoveTempFilesAndDirs

set inputScalarRange [[$inputNode GetImageData] GetScalarRange]
set outputScalarRange [[$outputNode GetImageData] GetScalarRange]

if { ( [lindex $inputScalarRange 0] != [lindex $outputScalarRange 0] ) || ( [lindex $inputScalarRange 1] != [lindex $outputScalarRange 1] ) } {
  $emLogic PrintText "TCL: ERROR: Scalar range of input and output do not match" 
  $emLogic PrintText "TCL: ERROR: Input: $inputScalarRange;  Output: $outputScalarRange" 
}


# -----------------------------------
# Notes 
# -----------------------------------

# ------------------------------------------------
# To execute this script in Slicer 
# Step 1 : Go in EMSEgmenter Module 
# Step 2 : run in the python console
#    emlogic=slicer.modules.emsegment.logic()  
#    em_manager=em_logic.GetMRMLManager()
#    tcl('set inputVolumeFileName blub') 
#    tcl('source /software/Slicer4Sandbox/test-thresh.tcl')
# which executes this script 


#--------------------------------------------------------------------
#import vtkITK
#rd=vtkITK.vtkITKArchetypeImageSeriesScalarReader()
#rd.SetArchetype('/software/Slicer4-EMSegment/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd')
# rd.Update()

# set e [vtkImageEllipsoidSource New]
# works - next use emsegmenters way of reading in 

#Replace 
#set rd [vtkITKArchetypeImageSeriesScalarReader New ]
#$rd SetArchetype "/software/Slicer4-EMSegment/Testing/TestData/TutorialTest/VolumeData/targetT1Normed_small.mhd"
# $rd Update
# with 
# set EML [vtkEMSegmentLogic New]
