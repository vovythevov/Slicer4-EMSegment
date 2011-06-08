from __main__ import qt, ctk

from EMSegmentPyStep import *
from Helper import *

class EMSegmentPyStepOne(EMSegmentPyStep) :
  
  def __init__(self, stepid):  
    self.initialize(stepid)
    self.setName('1. Define Task')
    self.setDescription('Select a (new) task.')
    
    self.__tasksList = dict() 
    
    
  def createUserInterface(self):
    '''
    '''
    self.buttonBoxHints = self.ButtonBoxHidden
    self.__parent = super(EMSegmentPyStepOne, self)
    self.__layout = self.__parent.createUserInterface()
    
    # let's load all tasks
    self.loadTasks()
    
    selectTaskLabel = qt.QLabel('Select Task')
    selectTaskLabel.setFont(self.__parent.getBoldFont())
    self.__layout.addRow(selectTaskLabel)
    
    self.__taskComboBox = qt.QComboBox()
    self.__taskComboBox.toolTip = "Choose a task."
    
    # fill the comboBox with the taskNames
    self.__taskComboBox.addItems(self.getTaskNames())
    self.__layout.addRow(Helper.CreateSpace(20), self.__taskComboBox)
    
    # add empty row
    self.__layout.addRow("", qt.QWidget())    
    
    chooseModeLabel = qt.QLabel('Choose Mode')
    chooseModeLabel.setFont(self.__parent.getBoldFont())
    self.__layout.addRow(chooseModeLabel)
    
    buttonBox = qt.QDialogButtonBox()
    simpleButton = buttonBox.addButton(buttonBox.Discard)
    simpleButton.setIcon(qt.QIcon())
    simpleButton.text = "Simple"
    simpleButton.toolTip = "Click to use the simple mode."
    advancedButton = buttonBox.addButton(buttonBox.Apply)
    advancedButton.setIcon(qt.QIcon())
    advancedButton.text = "Advanced"
    advancedButton.toolTip = "Click to use the advanced mode."
    self.__layout.addWidget(buttonBox)
    
    # connect the simple and advanced buttons
    simpleButton.connect('clicked()', self.goSimple)
    advancedButton.connect('clicked()', self.goAdvanced)
    
    
  def loadTasks(self):
    '''
    Load all available Tasks and save them to self.__tasksList as key,value pairs of taskName and fileName
    '''
    if not self.logic():
      Helper.Info("No logic class!")
      return False
      
    # we query the logic for a comma-separated string with the following format of each item:
    # tasksName:tasksFile
    tasksList = self.logic().GetTasks().split(',')
    
    self.__tasksList.clear()
    
    for t in tasksList:
      task = t.split(':')
      taskName = task[0]
      taskFile = task[1]
      
      # add this entry to out tasksList
      self.__tasksList[taskName] = taskFile
    
    return True
  
  def loadTask(self):
    '''
    '''
    index = self.__taskComboBox.currentIndex
    
    taskName = self.__taskComboBox.currentText
    taskFile = self.__tasksList[taskName]
    
    if not taskName or not taskFile:
      # error!
      Helper.Info("Error: either taskName or taskFile was empty!")
      return False
    
    Helper.Debug("Attempting to load task '" + taskName + "' from file '" + taskFile + "'")

    self.mrmlManager().ImportMRMLFile(taskFile)
    
    # now get the loaded EMSTemplateNode
    # TODO should not be only the first one
    templateNode = slicer.mrmlScene.GetNthNodeByClass(0,'vtkMRMLEMSTemplateNode')
    
    if not templateNode:
      # error!
      Helper.Info("Error: could not find template node!")
      return False
    
    loadResult = self.mrmlManager().SetLoadedParameterSetIndex(templateNode)
    if not loadResult:
      Helper.Info("EMS node is corrupted - the manager could not be updated with new task: " + taskName)
      return False

    self.logic().DefineTclTaskFullPathName(self.mrmlManager().GetTclTaskFilename())
    
    
  def getTaskNames(self):
    '''
    Get the taskNames of our tasksList
    '''
    return self.__tasksList.keys()
    
  def goSimple(self):
    '''
    '''
    self.loadTask()
    
    workflow = self.workflow()
    if not workflow:
      Helper.Info("No valid workflow found!")
      return False
    
    # we go forward in the simpleMode branch
    workflow.goForward('SimpleMode')
    
    
  def goAdvanced(self):
    '''
    '''
    self.loadTask()
    
    workflow = self.workflow()
    if not workflow:
      Helper.Info("No valid workflow found!")
      return False

    # we go forward in the advancedMode branch
    workflow.goForward('AdvancedMode')
    
