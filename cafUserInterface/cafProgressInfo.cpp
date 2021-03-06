//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#include "cafProgressInfo.h"
#include <QPointer>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QThread>
#include <assert.h>

namespace caf {

//==================================================================================================
///
/// \class caf::ProgressInfo
/// This class provides a simple frontend to the Qt progressbar, allowing distributed 
/// progress calculation. 
/// 
/// Create an instance of this object in the method/function that needs progress information
/// Then call incrementProgress() or setProgress() at proper times in your method.
/// When the method returns, the ProgressInfo destructor will clean up and finish.
/// The real beauty is that this class will magically interact with possible ProgressInfo instances
/// instantiated in function your method calls, providing a complete, consistent and detailed progress bar 
/// 
/// caf::ProgressInfo progInfo(3, "Open File");
/// progInfo.setProgressDescription("Reading");
/// ...readFile()
/// progInfo.incrementProgress();
/// progInfo.setProgressDescription("Validating");
/// ... validateData();
/// progInfo.incrementProgress();
/// progInfo.setProgressDescription("Building geometry");
///  ... buildGeometry();
/// progInfo.incrementProgress(); // not needed really, caus destructor will send progress to top.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgressInfo::ProgressInfo(int maxProgressValue, const QString& title)
{
    ProgressInfoStatic::start(maxProgressValue, title);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ProgressInfo::~ProgressInfo()
{
    ProgressInfoStatic::finished();
}

//--------------------------------------------------------------------------------------------------
/// Sets a description of the step currently being executed. 
/// Used to create a nice text in the progressDialog
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setProgressDescription(const QString& description)
{
    ProgressInfoStatic::setProgressDescription(description);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setProgress(int progressValue)
{
    ProgressInfoStatic::setProgress(progressValue);
}

//--------------------------------------------------------------------------------------------------
/// Increments progress by 1, or by the amount set by setNextProgressStepSize
//--------------------------------------------------------------------------------------------------
void ProgressInfo::incrementProgress()
{
    ProgressInfoStatic::incrementProgress();
}

//--------------------------------------------------------------------------------------------------
/// To make a certain operation span more of the progress bar than one tick, 
/// set the number of progress ticks that you want it to use before calling it.
/// Eg.
/// caf::ProgressInfo progInfo(5, "Doing things");
/// // ... Do one small thing
/// progInfo.incrementProgress();
/// progInfo.setNextProgressStepSize(3);
/// // ... Some heavy function call with its own progress handling
/// progInfo.incrementProgress();
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfo::setNextProgressIncrement(int nextStepSize)
{
    ProgressInfoStatic::setNextProgressIncrement(nextStepSize);
}








//==================================================================================================
///
/// Internal file-scope private functions to implement the progress dialog
/// 
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static QProgressDialog* progressDialog()
{
     static QPointer<QProgressDialog> progDialog;
     if (progDialog.isNull())
     {
         progDialog = new QProgressDialog();

         progDialog->hide();
     }
     return progDialog;
}

//--------------------------------------------------------------------------------------------------
/// A static vector containing the maximum values for the progress on each sublevel (call stack level)
//--------------------------------------------------------------------------------------------------
static std::vector<int>& maxProgressStack()
{
     static std::vector<int> m_maxProgressStack;
  
     return m_maxProgressStack;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static std::vector<QString>& titleStack()
{
    static std::vector<QString> m_titleStack;

    return m_titleStack;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static std::vector<QString>& descriptionStack()
{
    static std::vector<QString> m_descriptionStack;

    return m_descriptionStack;
}

//--------------------------------------------------------------------------------------------------
/// The actual progress value on each level (call stack level) 0 corresponds to the outermost function
//--------------------------------------------------------------------------------------------------
static std::vector<int>& progressStack()
{
    static std::vector<int> m_progressStack;

    return m_progressStack;
}

//--------------------------------------------------------------------------------------------------
/// The number of progress ticks (span) on each callstack level that the level deeper (in callstack) shall fill
/// used to balance the progress, making some (heavy) operations span more of the progress bar
//--------------------------------------------------------------------------------------------------
static std::vector<int>& progressSpanStack()
{
    static std::vector<int> m_progressSpanStack;

    return m_progressSpanStack;
}


//--------------------------------------------------------------------------------------------------
/// Calculate the total number of progress values we would need if we only look at the levels from 
/// \a levelDepth and below (increasing subdivision)
//--------------------------------------------------------------------------------------------------
static int subLevelsMaxProgressValue(size_t levelDepth)
{
    int levCount = 1;
    for (; levelDepth < maxProgressStack().size(); ++levelDepth)
    {
        levCount *=  maxProgressStack()[levelDepth];
    }
    return levCount;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the total progress value based on the current level subdivision and progress
//--------------------------------------------------------------------------------------------------
static int currentTotalProgress()
{
    int progress = 0;
    for (size_t i = 0; i < progressStack().size(); ++i)
    {
        int span = (i < 1) ? 1 : progressSpanStack()[i-1];
        progress = progress + span*progressStack()[i]* subLevelsMaxProgressValue(i+1);
    }
    return progress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static int currentTotalMaxProgressValue()
{
    return subLevelsMaxProgressValue(0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
static QString currentComposedLabel()
{
    QString labelText;

    for (size_t i = 0; i < titleStack().size(); ++i)
    {
        if (!titleStack()[i].isEmpty()) labelText += titleStack()[i];
        if (!titleStack()[i].isEmpty() && !descriptionStack()[i].isEmpty()) labelText +=": ";
        if (!descriptionStack()[i].isEmpty()) labelText += descriptionStack()[i] ;
        if (!(titleStack()[i].isEmpty() && descriptionStack()[i].isEmpty())) labelText += "\n";
    }
    labelText += "\n                                                                                                                      ";
    return labelText;

}

static bool isWrongThread()
{
   return  !(progressDialog()->thread() == QThread::currentThread());
}
//==================================================================================================
///
/// \class caf::ProgressInfoStatic
/// 
/// 
/// 
//==================================================================================================


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::start(int maxProgressValue, const QString& title)
{
    if (!qApp) return;

    if (isWrongThread()) return;

    if (!maxProgressStack().size())
    {
        progressDialog()->setWindowModality(Qt::WindowModal);
        progressDialog()->setMinimum(0);
        progressDialog()->setWindowTitle(title);
        progressDialog()->setCancelButton(NULL);
        progressDialog()->show();
    }

    maxProgressStack().push_back(maxProgressValue);
    progressStack().push_back(0);
    progressSpanStack().push_back(1);
    titleStack().push_back(title);
    descriptionStack().push_back("");

    progressDialog()->setMaximum(currentTotalMaxProgressValue());
    progressDialog()->setValue(currentTotalProgress());
    progressDialog()->setLabelText(currentComposedLabel());

    QCoreApplication::processEvents();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setProgressDescription(const QString& description)
{
    if (isWrongThread()) return;

    descriptionStack().back() = description;

    progressDialog()->setLabelText(currentComposedLabel());
    QCoreApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setProgress(int progressValue)
{
    if (isWrongThread()) return;
    if (progressValue == progressStack().back()) return; // Do nothing if no progress.

    // Guard against the max value set for theis level
    if (progressValue < 0 ) progressValue = 0;
    if (progressValue > maxProgressStack().back() ) progressValue = maxProgressStack().back();

    progressStack().back() = progressValue;
    progressSpanStack().back() = 1;

    assert(currentTotalProgress() <= progressDialog()->maximum());
    int totProg = currentTotalProgress();

    progressDialog()->setMaximum(currentTotalMaxProgressValue());
    progressDialog()->setValue(currentTotalProgress());
    QCoreApplication::processEvents();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::incrementProgress()
{
    assert(progressStack().size());
    ProgressInfoStatic::setProgress(progressStack().back() += progressSpanStack().back());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::setNextProgressIncrement(int nextStepSize)
{
    assert(progressSpanStack().size());

    progressSpanStack().back() = nextStepSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ProgressInfoStatic::finished()
{
    if (isWrongThread()) return;

    assert(maxProgressStack().size() && progressStack().size() && progressSpanStack().size() && titleStack().size() && descriptionStack().size());

    // Set progress to max value, and leave it there until somebody touches the progress again

    ProgressInfoStatic::setProgress(maxProgressStack().back());

    // Pop all the stacks
    maxProgressStack().pop_back();
    progressStack().pop_back();
    progressSpanStack().pop_back();
    titleStack().pop_back();
    descriptionStack().pop_back();

    // Update the text to reflect the "previous level"
    progressDialog()->setLabelText(currentComposedLabel());

    // If we are finishing the last level, clean up
    if (!maxProgressStack().size())
    {
        if (progressDialog() != NULL)
        {
            progressDialog()->hide();
            delete progressDialog();
        }
    }

    // Make sure the Gui is repainted
    QCoreApplication::processEvents();
}


} // namespace caf 
