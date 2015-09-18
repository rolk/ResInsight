/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellLogFile.h"

#include <QStringList>
#include <QFileInfo>


CAF_PDM_SOURCE_INIT(RimWellLogFile, "WellLogFile");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile::RimWellLogFile()
{
    CAF_PDM_InitObject("Well LAS File Info", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_wellName, "WellName", "",  "", "", "");
    m_wellName.uiCapability()->setUiReadOnly(true);
    m_wellName.uiCapability()->setUiHidden(true);
    m_wellName.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_fileName, "FileName", "Filename",  "", "", "");
    m_fileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "",  "", "", "");
    m_name.uiCapability()->setUiReadOnly(true);
    m_name.uiCapability()->setUiHidden(true);
    m_name.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_wellLogChannelNames, "WellLogFileChannels", "",  "", "", "");
    m_wellLogChannelNames.uiCapability()->setUiHidden(true);
    m_wellLogChannelNames.xmlCapability()->setIOWritable(false);

    m_wellLogDataFile = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFile::~RimWellLogFile()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellLogFile::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogFile::readFile()
{
    if (!m_wellLogDataFile.p())
    {
        m_wellLogDataFile = new RigWellLogFile;
    }

    if (!m_wellLogDataFile->open(m_fileName))
    {
        return false;
    }

    m_wellName = m_wellLogDataFile->wellName();
    m_name = QFileInfo(m_fileName).fileName();

    m_wellLogChannelNames.deleteAllChildObjects();

    QStringList wellLogNames = m_wellLogDataFile->wellLogChannelNames();
    for (int logIdx = 0; logIdx < wellLogNames.size(); logIdx++)
    {
        RimWellLogFileChannel* wellLog = new RimWellLogFileChannel();
        wellLog->setName(wellLogNames[logIdx]);
        m_wellLogChannelNames.push_back(wellLog);
    }

    RimWellPath* wellPath;
    firstAnchestorOrThisOfType(wellPath);
    if (wellPath)
    {
        if (wellPath->filepath().isEmpty())
        {
            wellPath->name = m_wellName;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellLogFile::wellName() const
{
    return m_wellName;
}
