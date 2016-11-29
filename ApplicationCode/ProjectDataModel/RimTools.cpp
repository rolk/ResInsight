/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimTools.h"

#include "RiaApplication.h"
#include "RimProject.h"

#include <QFileInfo>
#include <QDir>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimTools::getCacheRootDirectoryPathFromProject()
{
    if (!RiaApplication::instance()->project())
    {
        return QString();
    }

    QString projectFileName = RiaApplication::instance()->project()->fileName();

    QString cacheRootFolderPath;
    QFileInfo fileInfo(projectFileName);
    cacheRootFolderPath = fileInfo.canonicalPath();
    cacheRootFolderPath += "/" + fileInfo.completeBaseName();

    return cacheRootFolderPath;
}

//--------------------------------------------------------------------------------------------------
///  Relocate the supplied file name, based on the search path as follows:
///  fileName, newProjectPath/fileNameWoPath, relocatedPath/fileNameWoPath
///  If the file is not found in any of the positions, the fileName is returned but converted to Qt Style path separators: "/"
///
///  The relocatedPath is found in this way:
///  use the start of newProjectPath
///  plus the end of the path to m_gridFileName
///  such that the common start of oldProjectPath and m_gridFileName is removed from m_gridFileName
///  and replaced with the start of newProjectPath up to where newProjectPath starts to be equal to oldProjectPath
//--------------------------------------------------------------------------------------------------
QString RimTools::relocateFile(const QString& orgFileName, const QString& orgNewProjectPath, const QString& orgOldProjectPath,
    bool* foundFile, std::vector<QString>* searchedPaths)
{
    if (foundFile) *foundFile = true;

    // Make sure we have a Qt formatted path ( using "/" not "\")
    QString fileName = QDir::fromNativeSeparators(orgFileName);
    QString newProjectPath = QDir::fromNativeSeparators(orgNewProjectPath);
    QString oldProjectPath = QDir::fromNativeSeparators(orgOldProjectPath);

    // If we from a file or whatever gets a real windows path on linux, we need to manually convert it
    // because Qt will not. QDir::fromNativeSeparators does nothing on linux.

    bool isWindowsPath = false;
    if (orgFileName.count("/")) isWindowsPath = false; // "/" are not allowed in a windows path
    else if (orgFileName.count("\\")
        && !QFile::exists(orgFileName)) // To make sure we do not convert single linux files containing "\"
    {
        isWindowsPath = true;
    }

    if (isWindowsPath)
    {
        // Windows absolute path detected. transform.
        fileName.replace(QString("\\"), QString("/"));
    }

    if (searchedPaths) searchedPaths->push_back(fileName);
    if (QFile::exists(fileName))
    {
        return fileName;
    }

    // First check in the new project file directory
    {
        QString fileNameWithoutPath = QFileInfo(fileName).fileName();
        QString candidate = QDir::fromNativeSeparators(newProjectPath + QDir::separator() + fileNameWithoutPath);
        if (searchedPaths) searchedPaths->push_back(candidate);

        if (QFile::exists(candidate))
        {
            return candidate;
        }
    }

    // Then find the possible move of a directory structure where projects and files referenced are moved in "paralell"

    QFileInfo gridFileInfo(QDir::fromNativeSeparators(fileName));
    QString gridFilePath = gridFileInfo.path();
    QString gridFileNameWoPath = gridFileInfo.fileName();
    QStringList gridPathElements = gridFilePath.split("/", QString::KeepEmptyParts);

    QString oldProjPath = QDir::fromNativeSeparators(oldProjectPath);
    QStringList oldProjPathElements = oldProjPath.split("/", QString::KeepEmptyParts);

    QString newProjPath = QDir::fromNativeSeparators(newProjectPath);
    QStringList newProjPathElements = newProjPath.split("/", QString::KeepEmptyParts);

    // Find the possible equal start of the old project path, and the referenced file

    bool pathStartsAreEqual = false;
    bool pathEndsDiffer = false;
    int firstDiffIdx = 0;
    for (firstDiffIdx = 0; firstDiffIdx < gridPathElements.size() && firstDiffIdx < oldProjPathElements.size(); ++firstDiffIdx)
    {
        if (gridPathElements[firstDiffIdx] == oldProjPathElements[firstDiffIdx])
        {
            pathStartsAreEqual = pathStartsAreEqual || !gridPathElements[firstDiffIdx].isEmpty();
        }
        else
        {
            pathEndsDiffer = true;
            break;
        }
    }

    if (!pathEndsDiffer && firstDiffIdx < gridPathElements.size() || firstDiffIdx < oldProjPathElements.size())
    {
        pathEndsDiffer = true;
    }

    // If the path starts are equal, try to substitute it in the referenced file, with the corresponding new project path start

    if (pathStartsAreEqual)
    {
        if (pathEndsDiffer)
        {
            QString oldGridFilePathEnd;
            for (int i = firstDiffIdx; i < gridPathElements.size(); ++i)
            {
                oldGridFilePathEnd += gridPathElements[i];
                oldGridFilePathEnd += "/";
            }

            // Find the new Project File Start Path

            QStringList oldProjectFilePathEndElements;
            for (int i = firstDiffIdx; i < oldProjPathElements.size(); ++i)
            {
                oldProjectFilePathEndElements.push_back(oldProjPathElements[i]);
            }

            int ppIdx = oldProjectFilePathEndElements.size() - 1;
            int lastDiffIdx = newProjPathElements.size() - 1;

            for (; lastDiffIdx >= 0 && ppIdx >= 0; --lastDiffIdx, --ppIdx)
            {
                if (oldProjectFilePathEndElements[ppIdx] != newProjPathElements[lastDiffIdx])
                {
                    break;
                }
            }

            QString newProjecetFileStartPath;
            for (int i = 0; i <= lastDiffIdx; ++i)
            {
                newProjecetFileStartPath += newProjPathElements[i];
                newProjecetFileStartPath += "/";
            }

            QString relocationPath = newProjecetFileStartPath + oldGridFilePathEnd;

            QString relocatedFileName = relocationPath + gridFileNameWoPath;

            if (searchedPaths) searchedPaths->push_back(relocatedFileName);

            if (QFile::exists(relocatedFileName))
            {
                return relocatedFileName;
            }
        }
        else
        {
            // The Grid file was located in the same dir as the Project file. This is supposed to be handled above.
            // So we did not find it
        }
    }

    // return the unchanged filename, if we could not find a valid relocation file
    if (foundFile) *foundFile = false;

    return fileName;
}
