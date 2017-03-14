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



#include "RivWellHeadPartMgr.h"

#include "RiaApplication.h"

#include "RigActiveCellInfo.h"
#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"

#include "RivPipeGeometryGenerator.h"
#include "RivPartPriority.h"
#include "RivSimWellPipeSourceInfo.h"

#include "cafEffectGenerator.h"

#include "cvfArrowGenerator.h"
#include "cvfDrawableGeo.h"
#include "cvfDrawableText.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfTransform.h"
#include "cvfqtUtils.h"
#include "cafDisplayCoordTransform.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellHeadPartMgr::RivWellHeadPartMgr(RimEclipseView* reservoirView, RimEclipseWell* well)
{
    m_rimReservoirView = reservoirView;
    m_rimWell = well;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellHeadPartMgr::~RivWellHeadPartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::buildWellHeadParts(size_t frameIndex)
{
    clearAllGeometry();

    if (m_rimReservoirView.isNull()) return;
    
    RigEclipseCaseData* rigReservoir = m_rimReservoirView->eclipseCase()->reservoirData();

    RimEclipseWell* well = m_rimWell;
    
    double characteristicCellSize = rigReservoir->mainGrid()->characteristicIJCellSize();

    cvf::Vec3d whEndPos;
    cvf::Vec3d whStartPos;
    {
        well->wellHeadTopBottomPosition(frameIndex, &whEndPos, &whStartPos);

        cvf::ref<caf::DisplayCoordTransform> transForm = m_rimReservoirView->displayCoordTransform();
        whEndPos   = transForm->transformToDisplayCoord(whEndPos);
        whStartPos = transForm->transformToDisplayCoord(whStartPos);
        whEndPos.z() += characteristicCellSize;
    }

    

    if (!well->wellResults()->hasWellResult(frameIndex)) return;

    const RigWellResultFrame& wellResultFrame = well->wellResults()->wellResultFrame(frameIndex);

    double pipeRadius = m_rimWell->pipeRadius();

    if (wellResultFrame.m_isOpen)
    {
        // Use slightly larger well head arrow when well is open
        pipeRadius *= 1.1;
    }

    // Upper part of simulation well pipe is defined to use branch index 0
    cvf::ref<RivSimWellPipeSourceInfo> sourceInfo = new RivSimWellPipeSourceInfo(m_rimWell, 0);

    cvf::Vec3d arrowPosition = whEndPos;
    arrowPosition.z() += pipeRadius;

    // Well head pipe geometry
    {
        cvf::ref<cvf::Vec3dArray> wellHeadPipeCoords = new cvf::Vec3dArray;
        wellHeadPipeCoords->resize(2);
        wellHeadPipeCoords->set(0, whStartPos);
        wellHeadPipeCoords->set(1, whEndPos);

        cvf::ref<RivPipeGeometryGenerator> pipeGeomGenerator = new RivPipeGeometryGenerator;
        pipeGeomGenerator->setPipeCenterCoords(wellHeadPipeCoords.p());
        pipeGeomGenerator->setPipeColor(well->wellPipeColor());
        pipeGeomGenerator->setCrossSectionVertexCount(m_rimReservoirView->wellCollection()->pipeCrossSectionVertexCount());


        pipeGeomGenerator->setRadius(pipeRadius);

        cvf::ref<cvf::DrawableGeo> pipeSurface = pipeGeomGenerator->createPipeSurface();
        cvf::ref<cvf::DrawableGeo> centerLineDrawable = pipeGeomGenerator->createCenterLine();

        if (pipeSurface.notNull())
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("RivWellHeadPartMgr: surface " + cvfqt::Utils::toString(well->name()));
            part->setDrawable(pipeSurface.p());

            caf::SurfaceEffectGenerator surfaceGen(cvf::Color4f(well->wellPipeColor()), caf::PO_1);
            if (m_rimReservoirView && m_rimReservoirView->isLightingDisabled())
            {
                surfaceGen.enableLighting(false);
            }

            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

            part->setEffect(eff.p());
            part->setSourceInfo(sourceInfo.p());

            m_wellHeadPipeSurfacePart = part;
        }

        if (centerLineDrawable.notNull())
        {
            cvf::ref<cvf::Part> part = new cvf::Part;
            part->setName("RivWellHeadPartMgr: centerline " + cvfqt::Utils::toString(well->name()));
            part->setDrawable(centerLineDrawable.p());

            caf::MeshEffectGenerator meshGen(well->wellPipeColor());
            cvf::ref<cvf::Effect> eff = meshGen.generateCachedEffect();

            part->setEffect(eff.p());
            part->setSourceInfo(sourceInfo.p());

            m_wellHeadPipeCenterPart = part;
        }
    }

    double arrowLength = characteristicCellSize * m_rimReservoirView->wellCollection()->wellHeadScaleFactor() * m_rimWell->wellHeadScaleFactor();

    if (wellResultFrame.m_isOpen)
    {
        // Use slightly larger well head arrow when well is open
        arrowLength = 1.1 * arrowLength;
    }

    cvf::Vec3d textPosition = arrowPosition;
    textPosition.z() += 1.2 * arrowLength;
    
    cvf::Mat4f matr;
    if (wellResultFrame.m_productionType != RigWellResultFrame::PRODUCER)
    {
        matr = cvf::Mat4f::fromRotation(cvf::Vec3f(1.0f, 0.0f, 0.0f), cvf::Math::toRadians(180.0f));
    }

    double ijScaleFactor = arrowLength / 6;
    if (wellResultFrame.m_isOpen)
    {
        ijScaleFactor *= 1.1;
    }
    matr(0, 0) *= ijScaleFactor;
    matr(1, 1) *= ijScaleFactor;
    matr(2, 2) *= arrowLength;

    if (wellResultFrame.m_productionType != RigWellResultFrame::PRODUCER)
    {
        arrowPosition.z() += arrowLength;
    }

    matr.setTranslation(cvf::Vec3f(arrowPosition));

    cvf::GeometryBuilderFaceList builder;
    cvf::ArrowGenerator gen;
    gen.setShaftRelativeRadius(0.5f);
    gen.setHeadRelativeRadius(1.0f);
    gen.setHeadRelativeLength(0.4f);
    gen.setNumSlices(m_rimReservoirView->wellCollection()->pipeCrossSectionVertexCount());
    gen.generate(&builder);

    cvf::ref<cvf::Vec3fArray> vertices = builder.vertices();
    cvf::ref<cvf::UIntArray> faceList = builder.faceList();

    size_t i;
    for (i = 0; i < vertices->size(); i++)
    {
        cvf::Vec3f v = vertices->get(i);
        v.transformPoint(matr);
        vertices->set(i, v);
    }

    cvf::ref<cvf::DrawableGeo> geo1 = new cvf::DrawableGeo;
    geo1->setVertexArray(vertices.p());   
    geo1->setFromFaceList(*faceList);
    geo1->computeNormals();


    {
        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivWellHeadPartMgr: arrow " + cvfqt::Utils::toString(well->name()));
        part->setDrawable(geo1.p());

        cvf::Color4f headColor(cvf::Color3::GRAY);

        RimEclipseWellCollection* wellColl = nullptr;
        if (m_rimWell)
        {
            m_rimWell->firstAncestorOrThisOfType(wellColl);
        }

        if (wellColl && wellColl->showConnectionStatusColors())
        {
            if (wellResultFrame.m_isOpen)
            {
                if (wellResultFrame.m_productionType == RigWellResultFrame::PRODUCER)
                {
                    headColor = cvf::Color4f(cvf::Color3::GREEN);
                }
                else if (wellResultFrame.m_productionType == RigWellResultFrame::OIL_INJECTOR)
                {
                    headColor = cvf::Color4f(cvf::Color3::ORANGE);
                }
                else if (wellResultFrame.m_productionType == RigWellResultFrame::GAS_INJECTOR)
                {
                    headColor = cvf::Color4f(cvf::Color3::RED);
                }
                else if (wellResultFrame.m_productionType == RigWellResultFrame::WATER_INJECTOR)
                {
                    headColor = cvf::Color4f(cvf::Color3::BLUE);
                }
            }
        }
        else
        {
            headColor = cvf::Color4f(m_rimWell->wellPipeColor());
        }

        caf::SurfaceEffectGenerator surfaceGen(headColor, caf::PO_1);
        if (m_rimReservoirView && m_rimReservoirView->isLightingDisabled())
        {
            surfaceGen.enableLighting(false);
        }
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();

        part->setEffect(eff.p());
        part->setSourceInfo(sourceInfo.p());

        m_wellHeadArrowPart = part;
    }

    if (well->showWellLabel() && !well->name().isEmpty())
    {
        cvf::Font* font = RiaApplication::instance()->customFont();

        cvf::ref<cvf::DrawableText> drawableText = new cvf::DrawableText;
        drawableText->setFont(font);
        drawableText->setCheckPosVisible(false);
        drawableText->setDrawBorder(false);
        drawableText->setDrawBackground(false);
        drawableText->setVerticalAlignment(cvf::TextDrawer::CENTER);
        drawableText->setTextColor(m_rimReservoirView->wellCollection()->wellLabelColor());

        cvf::String cvfString = cvfqt::Utils::toString(m_rimWell->name());

        cvf::Vec3f textCoord(textPosition);
        drawableText->addText(cvfString, textCoord);

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("RivWellHeadPartMgr: text " + cvfString);
        part->setDrawable(drawableText.p());

        cvf::ref<cvf::Effect> eff = new cvf::Effect;

        part->setEffect(eff.p());
        part->setPriority(RivPartPriority::PartType::Text);
        part->setSourceInfo(sourceInfo.p());

        m_wellHeadLabelPart = part;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::clearAllGeometry()
{
    m_wellHeadArrowPart = nullptr;
    m_wellHeadLabelPart = nullptr;
    m_wellHeadPipeCenterPart = nullptr;
    m_wellHeadPipeSurfacePart = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellHeadPartMgr::appendDynamicGeometryPartsToModel(cvf::ModelBasicList* model, size_t frameIndex)
{
    if (m_rimReservoirView.isNull()) return;
    if (m_rimWell.isNull()) return;

    RimEclipseWellCollection* wellCollection = nullptr;
    m_rimWell->firstAncestorOrThisOfType(wellCollection);
    if (!wellCollection) return;

    if (!m_rimWell->isWellPipeVisible(frameIndex)) return;

    buildWellHeadParts(frameIndex);

    // Always add pipe part of well head
    if (m_wellHeadPipeCenterPart.notNull()) model->addPart(m_wellHeadPipeCenterPart.p());
    if (m_wellHeadPipeSurfacePart.notNull()) model->addPart(m_wellHeadPipeSurfacePart.p());

    if (m_rimWell->showWellLabel() && 
        m_wellHeadLabelPart.notNull())
    {
        model->addPart(m_wellHeadLabelPart.p());
    }
    
    if (m_rimWell->showWellHead() &&
        m_wellHeadArrowPart.notNull())
    {
        model->addPart(m_wellHeadArrowPart.p());
    }
}


