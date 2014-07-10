source [file join $emSourceDir Tasks/GenericTask.tcl ] 
variable ::EMSegmenterPreProcessingTcl::LOGIC $emLogic
set result [::EMSegmenterPreProcessingTcl::Get_Installation_Path [file dirname $input] [file tail $input] ]

if { $result == "" } {
   $emLogic PrintText "TCL: ERROR: Could not find $input"
}
