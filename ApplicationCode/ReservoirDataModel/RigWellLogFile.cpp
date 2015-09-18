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

#include "RigWellLogFile.h"

#include "well.hpp"
#include "laswell.hpp"

#include <QString>
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogFile::RigWellLogFile()
    : cvf::Object()
{
    m_wellLogFile = NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogFile::~RigWellLogFile()
{
    close();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellLogFile::open(const QString& fileName)
{
    close();

    int wellFormat = NRLib::Well::LAS;
    NRLib::Well* well = NRLib::Well::ReadWell(fileName.toStdString(), wellFormat);
    if (!well)
    {
        // TODO: Error handling
        return false;
    }

    QStringList wellLogNames;

    const std::map<std::string, std::vector<double> >& contLogs = well->GetContLog();
    std::vector<std::string> contLogNames;
    std::map<std::string, std::vector<double> >::const_iterator itCL;
    for (itCL = contLogs.begin(); itCL != contLogs.end(); itCL++)
    {
        wellLogNames.append(QString::fromStdString(itCL->first));
    }

    // TODO: Possibly handle discrete logs

//     const std::map<std::string, std::vector<int> >& discLogs = well->GetDiscLog();
//     std::vector<std::string> discLogNames;
//     std::map<std::string, std::vector<int> >::const_iterator itDL;
//     for (itDL = discLogs.begin(); itDL != discLogs.end(); itDL++)
//     {
//         wellLogNames.append(QString::fromStdString(itDL->first));
//     }

    m_wellLogChannelNames = wellLogNames;
    m_wellLogFile = well;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogFile::close()
{
    if (m_wellLogFile)
    {
        delete m_wellLogFile;
        m_wellLogFile = NULL;
    }

    m_wellLogChannelNames.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellLogFile::wellName() const
{
    CVF_ASSERT(m_wellLogFile);
    return QString::fromStdString(m_wellLogFile->GetWellName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RigWellLogFile::wellLogChannelNames() const
{
    return m_wellLogChannelNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogFile::depthValues() const
{
    return values("DEPT");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogFile::values(const QString& name) const
{
    // TODO: Possibly handle discrete logs

    CVF_ASSERT(m_wellLogFile);

    if (m_wellLogFile->HasContLog(name.toStdString()))
    {
        return m_wellLogFile->GetContLog(name.toStdString());
    }

    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RigWellLogFile::depthUnit() const
{
    QString unit;

    NRLib::LasWell* lasWell = dynamic_cast<NRLib::LasWell*>(m_wellLogFile);
    if (lasWell)
    {
        unit = QString::fromStdString(lasWell->depthUnit());
    }

    return unit;
}
