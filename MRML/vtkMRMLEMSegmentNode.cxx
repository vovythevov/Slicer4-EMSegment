#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLEMSegmentNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLEMSegmentNode* vtkMRMLEMSegmentNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSegmentNode");
  if(ret)
    {
    return (vtkMRMLEMSegmentNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSegmentNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLEMSegmentNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSegmentNode");
  if(ret)
    {
    return (vtkMRMLEMSegmentNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSegmentNode;
}

//----------------------------------------------------------------------------
vtkMRMLEMSegmentNode::vtkMRMLEMSegmentNode()
{
  this->Value = NULL;
  this->ModuleName = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLEMSegmentNode::~vtkMRMLEMSegmentNode()
{
  if (this->Value)
    {
    delete [] this->Value;
    this->Value = NULL;
    }
  if (this->ModuleName)
    {
    delete [] this->ModuleName;
    this->ModuleName = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLEMSegmentNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream

  vtkIndent indent(nIndent);

  if (this->ModuleName != NULL)
    {
    of << " ModuleName = \"" << this->ModuleName << "\" ";
    }

  std::map<std::string, std::string>::iterator iter;

  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
      of << " " << iter->first << "= \"" << iter->second << "\" ";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLEMSegmentNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if ( !strcmp(attName, "ModuleName") )
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ModuleName;
      }
    else
      {
      std::string sname(attName);
      std::string svalue(attValue);
      this->SetParameter(sname, svalue);
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLEMSegmentNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLEMSegmentNode *node = 
    (vtkMRMLEMSegmentNode *) anode;

  this->Parameters = node->Parameters;

  this->SetModuleName( this->GetModuleName() );
}

//----------------------------------------------------------------------------
void vtkMRMLEMSegmentNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "ModuleName: " 
     << (this->GetModuleName() ? this->GetModuleName() : "(none)") << "\n";

  std::map<std::string, std::string>::iterator iter;

  for (iter=this->Parameters.begin(); iter != this->Parameters.end(); iter++)
    {
    os << indent << iter->first << ": " << iter->second << "\n";
    }
}

//----------------------------------------------------------------------------
void
vtkMRMLEMSegmentNode
::SetParameter(const std::string& name, const std::string& value)
{
  // Set the default value of the named parameter with the value
  // specified
  const std::string *currentValue = this->GetParameter(name);
  if (currentValue == NULL || (currentValue != NULL && value != *currentValue))
    {
    this->Parameters[name] = value;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
const std::string *
vtkMRMLEMSegmentNode
::GetParameter(const std::string& name) const
{
  if ( this->Parameters.find(name) == this->Parameters.end() )
    {
    return (NULL);
    }
  return &(this->Parameters.find(name)->second);
}

//----------------------------------------------------------------------------
const char *
vtkMRMLEMSegmentNode
::GetParameter(const char *name)
{
  this->RequestParameter(name);
  return (this->GetValue());
}

//----------------------------------------------------------------------------
void
vtkMRMLEMSegmentNode
::SetParameter(const char *name, const char *value)
{
  std::string sname(name);
  std::string svalue(value);
  this->SetParameter(sname, svalue);
}

//----------------------------------------------------------------------------
void
vtkMRMLEMSegmentNode
::RequestParameter(const char *name)
{
  std::string sname(name);
  const std::string *svaluep = this->GetParameter(sname);
  this->SetValue (svaluep->c_str());
}
