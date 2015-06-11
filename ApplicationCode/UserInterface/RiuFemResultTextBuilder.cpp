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

#include "RiuFemResultTextBuilder.h"

#include "RigGeoMechCaseData.h"
#include "RimGeoMechView.h"
#include "RimGeoMechCase.h"
#include "RigFemPartCollection.h"
#include "RigFemPart.h"
#include "RigFemPartGrid.h"
#include "RimGeoMechResultSlot.h"
#include "RigFemPartResultsCollection.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuFemResultTextBuilder::RiuFemResultTextBuilder(RimGeoMechView* reservoirView, int gridIndex, int cellIndex, int timeStepIndex)
{
    CVF_ASSERT(reservoirView);
    
    m_reservoirView = reservoirView;
	m_gridIndex = gridIndex;
	m_cellIndex = cellIndex;
	m_timeStepIndex = timeStepIndex;

    m_intersectionPoint = cvf::Vec3d::UNDEFINED;
    m_face = cvf::StructGridInterface::NO_FACE;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::setIntersectionPoint(cvf::Vec3d intersectionPoint)
{
    m_intersectionPoint = intersectionPoint;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::setFace(cvf::StructGridInterface::FaceType face)
{
    m_face = face;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::mainResultText()
{
    QString text;

    text = closestNodeResultText(m_reservoirView->cellResult());

    if (!text.isEmpty()) text += "\n";

    QString topoText = this->topologyText("\n");
    text += topoText;
    text += "\n";

    appendDetails(text, gridResultDetails());

	return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::topologyText(QString itemSeparator)
{
    QString text;

    if (m_reservoirView->geoMechCase())
    {
        RigGeoMechCaseData* geomData = m_reservoirView->geoMechCase()->geoMechData();
        if (geomData)
        {
         
            RigFemPart* femPart = geomData->femParts()->part(m_gridIndex);
            int elementId = femPart->elmId(m_cellIndex);
            text += QString("Element : Id[%1]").arg(elementId);

            size_t i = 0;
            size_t j = 0;
            size_t k = 0;
            if (geomData->femParts()->part(m_gridIndex)->structGrid()->ijkFromCellIndex(m_cellIndex, &i, &j, &k))
            {
                // Adjust to 1-based Eclipse indexing
                i++;
                j++;
                k++;

                cvf::Vec3d domainCoord = m_intersectionPoint; //  + geomCase->femParts()->displayModelOffset();

                //cvf::StructGridInterface::FaceEnum faceEnum(m_face);

                //QString faceText = faceEnum.text();

                //text += QString("Face : %1").arg(faceText) + itemSeparator;
                //text += QString("Fem Part %1").arg(m_gridIndex) + itemSeparator;
                text += QString(", ijk[%1, %2, %3]").arg(i).arg(j).arg(k) + itemSeparator;

                QString formattedText;
                formattedText.sprintf("Intersection point : [E: %.2f, N: %.2f, Depth: %.2f]", domainCoord.x(), domainCoord.y(), -domainCoord.z());

                text += formattedText;
            }
        }
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::gridResultDetails()
{
	QString text;

	if (m_reservoirView->geoMechCase() && m_reservoirView->geoMechCase()->geoMechData())
	{
		RigGeoMechCaseData* eclipseCaseData = m_reservoirView->geoMechCase()->geoMechData();

		this->appendTextFromResultSlot(eclipseCaseData, m_gridIndex, m_cellIndex, m_timeStepIndex, m_reservoirView->cellResult(), &text);

        if (!text.isEmpty())
        {
            text.prepend("-- Element result details --\n");
        }
	}

	return text;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::appendTextFromResultSlot(RigGeoMechCaseData* geomData, int gridIndex, int cellIndex, int timeStepIndex, RimGeoMechResultSlot* resultSlot, QString* resultInfoText)
{
	if (!resultSlot)
	{
		return;
	}

    if (resultSlot->hasResult())
    {
        const std::vector<float>& scalarResults = geomData->femPartResults()->resultValues(resultSlot->resultAddress(), gridIndex, timeStepIndex);
        if (scalarResults.size())
        {
            caf::AppEnum<RigFemResultPosEnum> resPosAppEnum = resultSlot->resultPositionType();
            resultInfoText->append(resPosAppEnum.uiText() + ", ");
            resultInfoText->append(resultSlot->resultFieldName()+ ", ") ;
            resultInfoText->append(resultSlot->resultComponentName() + ":\n");


            RigFemPart* femPart = geomData->femParts()->part(gridIndex);
            RigElementType elmType =  femPart->elementType(cellIndex);
            const int* elmentConn = femPart->connectivities(cellIndex);
            int elmNodeCount = RigFemTypes::elmentNodeCount(elmType);
            for (int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx)
            {
               
                float scalarValue = std::numeric_limits<float>::infinity();
                int nodeIdx = elmentConn[lNodeIdx];
                if (resultSlot->resultPositionType() == RIG_NODAL)
                {
                   
                    scalarValue = scalarResults[nodeIdx];
                } 
                else 
                {
                    size_t resIdx = femPart->elementNodeResultIdx(cellIndex, lNodeIdx);
                    scalarValue = scalarResults[resIdx];
                }

                resultInfoText->append(QString("\tN:%1 \t: %2\n").arg(femPart->nodes().nodeIds[nodeIdx]).arg(scalarValue));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuFemResultTextBuilder::appendDetails(QString& text, const QString& details)
{
    if (!details.isEmpty())
    {
        text += "\n";
        text += details;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuFemResultTextBuilder::closestNodeResultText(RimGeoMechResultSlot* resultSlot)
{
    QString text;
	if (!resultSlot)
	{
		return text;
	}

    if (resultSlot->hasResult())
    {
    	if (! (m_reservoirView->geoMechCase() && m_reservoirView->geoMechCase()->geoMechData())) return text;
	
		RigGeoMechCaseData* geomData = m_reservoirView->geoMechCase()->geoMechData();

        const std::vector<float>& scalarResults = geomData->femPartResults()->resultValues(resultSlot->resultAddress(), m_gridIndex, m_timeStepIndex);
        if (scalarResults.size())
        {

            RigFemPart* femPart = geomData->femParts()->part(m_gridIndex);
            RigElementType elmType =  femPart->elementType(m_cellIndex);
            const int* elmentConn = femPart->connectivities(m_cellIndex);
            int elmNodeCount = RigFemTypes::elmentNodeCount(elmType);

            // Find the closest node
            int closestLocalNode = -1;
            float minDist = std::numeric_limits<float>::infinity();
            for (int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx)
            {
                int nodeIdx = elmentConn[lNodeIdx];
                cvf::Vec3f nodePos = femPart->nodes().coordinates[nodeIdx];
                float dist = (nodePos - cvf::Vec3f(m_intersectionPoint)).lengthSquared();
                if (dist < minDist) 
                {
                    closestLocalNode = lNodeIdx;
                    minDist = dist;
                }
            }

            // Create a text showing the results from the closest node
            if (closestLocalNode >= 0)
            {
               
                float scalarValue = std::numeric_limits<float>::infinity();
                int nodeIdx = elmentConn[closestLocalNode];
                if (resultSlot->resultPositionType() == RIG_NODAL)
                {
                   
                    scalarValue = scalarResults[nodeIdx];
                } 
                else 
                {
                    size_t resIdx = femPart->elementNodeResultIdx(m_cellIndex, closestLocalNode);
                    scalarValue = scalarResults[resIdx];
                }

                text.append(QString("Closest result: N[%1], %2\n").arg(femPart->nodes().nodeIds[nodeIdx]).arg(scalarValue));
            }
        }
    }
   
    return text;
}
