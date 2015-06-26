/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#pragma once

#include "cvfBase.h"
#include "cafPdmPointer.h"
#include "cvfStructGrid.h"
#include <QString>

class RimGeoMechView;
class RimResultSlot;
class RigGeoMechCaseData;
class RimGeoMechCellColors;

namespace cvf {
	class Part;
}

//==================================================================================================
//
//
//==================================================================================================
class RiuFemResultTextBuilder
{
public:
	RiuFemResultTextBuilder(RimGeoMechView* reservoirView, int gridIndex, int cellIndex, int timeStepIndex);
    void setFace(cvf::StructGridInterface::FaceType face);
    void setIntersectionPoint(cvf::Vec3d intersectionPoint);

    QString mainResultText();

	QString topologyText(QString itemSeparator);
	
private:
    void appendDetails(QString& text, const QString& details);

    QString gridResultDetails();

    QString closestNodeResultText(RimGeoMechCellColors* resultSlot);

	void appendTextFromResultSlot(RigGeoMechCaseData* eclipseCase, int gridIndex, int cellIndex, int timeStepIndex, RimGeoMechCellColors* resultSlot, QString* resultInfoText);

private:
    caf::PdmPointer<RimGeoMechView> m_reservoirView;

	int m_gridIndex;
	int m_cellIndex;
	int m_timeStepIndex;

	cvf::StructGridInterface::FaceType m_face;

	cvf::Vec3d m_intersectionPoint;
};
