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
$emLogic PrintText "Test 1 : DeepCopy Ellipsoid" 
set ellips [vtkImageEllipsoidSource New]
$outputVolume DeepCopy [$ellips GetOutput] 

# ------------------------------------------------------
$emLogic PrintText "Test 2 : Threshold Input" 
set thresh [vtkImageThreshold New]
$thresh SetInput [$inputNode GetImageData] 
$thresh Update

# ------------------------------------------------------
$emLogic PrintText "Test 3 : DeepCopy Input" 
$outputVolume DeepCopy [$inputNode GetImageData] 


# ------------------------------------------------------
$emLogic PrintText "Test 4 : DeepCopy Threshold" 
$outputVolume DeepCopy [$thresh GetOutput] 

# ------------------------------------------------------
$emLogic PrintText "Test 5 : First DeepCopy Ellipse and then Threshold" 
$outputVolume DeepCopy [$ellips GetOutput] 
$emLogic PrintText "Ellipse done"
$outputVolume DeepCopy [$thresh GetOutput] 
$emLogic PrintText "Thresh done"

# Did not catch the error message from vtk
# catch { $outputVolume DeepCopy [$thresh GetOutput] } errMSG
# $emLogic PrintText "--- $errMSG -- "


$thresh Delete
$ellips Delete

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
