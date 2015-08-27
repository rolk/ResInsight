#include "RimView.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellRangeFilterCollection.h"
#include "RimManagedViewCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "RivWellPathCollectionPartMgr.h"

#include "cafFrameAnimationControl.h"
#include "cafPdmObjectFactory.h"
#include "cvfCamera.h"
#include "cvfModel.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfScene.h"
#include "cvfViewport.h"

#include <limits.h>


namespace caf {

template<>
void caf::AppEnum< RimView::MeshModeType >::setUp()
{
    addItem(RimView::FULL_MESH,      "FULL_MESH",       "All");
    addItem(RimView::FAULTS_MESH,    "FAULTS_MESH",      "Faults only");
    addItem(RimView::NO_MESH,        "NO_MESH",        "None");
    setDefault(RimView::FULL_MESH);
}

template<>
void caf::AppEnum< RimView::SurfaceModeType >::setUp()
{
    addItem(RimView::SURFACE,              "SURFACE",             "All");
    addItem(RimView::FAULTS,               "FAULTS",              "Faults only");
    addItem(RimView::NO_SURFACE,           "NO_SURFACE",          "None");
    setDefault(RimView::SURFACE);
}

} // End namespace caf


#include "cafPdmAbstractClassSourceInit.h"

CAF_PDM_ABSTRACT_SOURCE_INIT(RimView, "GenericView"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView::RimView(void)
{
    RiaApplication* app = RiaApplication::instance();
    RiaPreferences* preferences = app->preferences();
    CVF_ASSERT(preferences);


    CAF_PDM_InitField(&name, "UserDescription", QString(""), "Name", "", "", "");

    CAF_PDM_InitField(&showWindow, "ShowWindow", true, "Show 3D viewer", "", "", "");
    showWindow.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&cameraPosition, "CameraPosition", cvf::Mat4d::IDENTITY, "", "", "", "");
    cameraPosition.uiCapability()->setUiHidden(true);

    double defaultScaleFactor = 1.0;
    if (preferences) defaultScaleFactor = preferences->defaultScaleFactorZ;
    CAF_PDM_InitField(&scaleZ, "GridZScale", defaultScaleFactor, "Z Scale", "", "Scales the scene in the Z direction", "");

    cvf::Color3f defBackgColor = preferences->defaultViewerBackgroundColor();
    CAF_PDM_InitField(&backgroundColor, "ViewBackgroundColor", defBackgColor, "Background", "", "", "");

    CAF_PDM_InitField(&maximumFrameRate, "MaximumFrameRate", 10, "Maximum frame rate", "", "", "");
    maximumFrameRate.uiCapability()->setUiHidden(true);
    CAF_PDM_InitField(&hasUserRequestedAnimation, "AnimationMode", false, "Animation Mode", "", "", "");
    hasUserRequestedAnimation.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_currentTimeStep, "CurrentTimeStep", 0, "Current Time Step", "", "", "");
    m_currentTimeStep.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&managedViewCollection, "ManagedViewCollection", "Managed View Collection", "", "", "");
    managedViewCollection = new RimManagedViewCollection;
    managedViewCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_overlayInfoConfig, "OverlayInfoConfig", "Info Box", "", "", "");
    m_overlayInfoConfig = new Rim3dOverlayInfoConfig();
    m_overlayInfoConfig->setReservoirView(this);
    m_overlayInfoConfig.uiCapability()->setUiHidden(true);

    caf::AppEnum<RimView::MeshModeType> defaultMeshType = NO_MESH;
    if (preferences->defaultGridLines) defaultMeshType = FULL_MESH;
    CAF_PDM_InitField(&meshMode, "MeshMode", defaultMeshType, "Grid lines",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&surfaceMode, "SurfaceMode", "Grid surface",  "", "", "");

    CAF_PDM_InitField(&m_disableLighting, "DisableLighting", false, "Disable Results Lighting", "", "Disable light model for scalar result colors", "");

    m_previousGridModeMeshLinesWasFaults = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView::~RimView(void)
{
    delete this->m_overlayInfoConfig();

    if (m_viewer)
    {
        RiuMainWindow::instance()->removeViewer(m_viewer);
    }

    delete m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuViewer* RimView::viewer()
{
    return m_viewer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateViewerWidget()
{
    if (showWindow())
    {
        bool isViewerCreated = false;
        if (!m_viewer)
        {
            QGLFormat glFormat;
            glFormat.setDirectRendering(RiaApplication::instance()->useShaders());

            m_viewer = new RiuViewer(glFormat, NULL);
            m_viewer->setOwnerReservoirView(this);

            RiuMainWindow::instance()->addViewer(m_viewer);
            m_viewer->setMinNearPlaneDistance(10);
           
            this->resetLegendsInViewer();

            m_viewer->updateNavigationPolicy();
            m_viewer->enablePerfInfoHud(RiaApplication::instance()->showPerformanceInfo());

            isViewerCreated = true;
        }

        RiuMainWindow::instance()->setActiveViewer(m_viewer);

        if (isViewerCreated) m_viewer->mainCamera()->setViewMatrix(cameraPosition);
        m_viewer->mainCamera()->viewport()->setClearColor(cvf::Color4f(backgroundColor()));

        m_viewer->update();
    }
    else
    {
        if (m_viewer)
        {
            if (m_viewer->layoutWidget()->parentWidget())
            {
                m_viewer->layoutWidget()->parentWidget()->hide();
            }
            else
            {
                m_viewer->layoutWidget()->hide(); 
            }
        }
    }

    updateViewerWidgetWindowTitle();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::scheduleCreateDisplayModelAndRedraw()
{
    RiaApplication::instance()->scheduleDisplayModelUpdateAndRedraw(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setCurrentTimeStep(int frameIndex)
{
    m_currentTimeStep = frameIndex;
    this->hasUserRequestedAnimation = true;
    this->updateCurrentTimeStep();
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::updateCurrentTimeStepAndRedraw()
{
    this->updateCurrentTimeStep();
    
    if (m_viewer) m_viewer->update();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::createDisplayModelAndRedraw()
{
    if (m_viewer)
    {
        this->clampCurrentTimestep();

        createDisplayModel();
        updateDisplayModelVisibility();
    }

    RiuMainWindow::instance()->refreshAnimationActions();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setDefaultView()
{
    if (m_viewer)
    {
        m_viewer->setDefaultView();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::endAnimation()
{
    this->hasUserRequestedAnimation = false;
    this->updateStaticCellColors();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setupBeforeSave()
{
    if (m_viewer)
    {
        hasUserRequestedAnimation = m_viewer->isAnimationActive(); // JJS: This is not conceptually correct. The variable is updated as we go, and store the user intentions. But I guess that in practice...
        cameraPosition = m_viewer->mainCamera()->viewMatrix();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
// Surf: No Fault Surf
//  Mesh -------------
//    No F  F     G
// Fault F  F     G
//  Mesh G  G     G
//
//--------------------------------------------------------------------------------------------------
bool RimView::isGridVisualizationMode() const
{
    return (   this->surfaceMode() == SURFACE 
            || this->meshMode()    == FULL_MESH);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setMeshOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
		meshMode.uiCapability()->setValueFromUi(FULL_MESH);
    }
    else
    {
		meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
    }

	surfaceMode.uiCapability()->setValueFromUi(NO_SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setMeshSurfDrawstyle()
{
    if (isGridVisualizationMode())
    {
		surfaceMode.uiCapability()->setValueFromUi(SURFACE);
		meshMode.uiCapability()->setValueFromUi(FULL_MESH);
    }
    else
    {
		surfaceMode.uiCapability()->setValueFromUi(FAULTS);
		meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setFaultMeshSurfDrawstyle()
{
    // Surf: No Fault Surf
    //  Mesh -------------
    //    No FF  FF    SF
    // Fault FF  FF    SF
    //  Mesh SF  SF    SF
    if (this->isGridVisualizationMode())
    {
		surfaceMode.uiCapability()->setValueFromUi(SURFACE);
    }
    else
    {
		surfaceMode.uiCapability()->setValueFromUi(FAULTS);
    }

	meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setSurfOnlyDrawstyle()
{
    if (isGridVisualizationMode())
    {
		surfaceMode.uiCapability()->setValueFromUi(SURFACE);
    }
    else
    {
		surfaceMode.uiCapability()->setValueFromUi(FAULTS);
    }

	meshMode.uiCapability()->setValueFromUi(NO_MESH);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setShowFaultsOnly(bool showFaults)
{
    if (showFaults)
    {
        m_previousGridModeMeshLinesWasFaults = meshMode() == FAULTS_MESH;
		if (surfaceMode() != NO_SURFACE) surfaceMode.uiCapability()->setValueFromUi(FAULTS);
		if (meshMode() != NO_MESH) meshMode.uiCapability()->setValueFromUi(FAULTS_MESH);
    }
    else
    {
		if (surfaceMode() != NO_SURFACE) surfaceMode.uiCapability()->setValueFromUi(SURFACE);
		if (meshMode() != NO_MESH) meshMode.uiCapability()->setValueFromUi(m_previousGridModeMeshLinesWasFaults ? FAULTS_MESH : FULL_MESH);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setSurfaceDrawstyle()
{
	if (surfaceMode() != NO_SURFACE) surfaceMode.uiCapability()->setValueFromUi(SURFACE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::disableLighting(bool disable)
{
    m_disableLighting = disable;
    updateCurrentTimeStepAndRedraw();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimView::isLightingDisabled() const
{
    return m_disableLighting();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &meshMode)
    {
        createDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
    else if (changedField == &scaleZ)
    {
        if (scaleZ < 1) scaleZ = 1;

        // Regenerate well paths
        RimOilField* oilFields = RiaApplication::instance()->project() ? RiaApplication::instance()->project()->activeOilField() : NULL;
        RimWellPathCollection* wellPathCollection = (oilFields) ? oilFields->wellPathCollection() : NULL;
        if (wellPathCollection) wellPathCollection->wellPathCollectionPartMgr()->scheduleGeometryRegen();

        if (m_viewer)
        {
            cvf::Vec3d poi = m_viewer->pointOfInterest();
            cvf::Vec3d eye, dir, up;
            eye = m_viewer->mainCamera()->position();
            dir = m_viewer->mainCamera()->direction();
            up  = m_viewer->mainCamera()->up();

            eye[2] = poi[2]*scaleZ()/this->scaleTransform()->worldTransform()(2, 2) + (eye[2] - poi[2]);
            poi[2] = poi[2]*scaleZ()/this->scaleTransform()->worldTransform()(2, 2);

            m_viewer->mainCamera()->setFromLookAt(eye, eye + dir, up);
            m_viewer->setPointOfInterest(poi);

            updateScaleTransform();
            createDisplayModelAndRedraw();
            m_viewer->update();
        }

        RiuMainWindow::instance()->updateScaleValue();
    }
    else if (changedField == &surfaceMode)
    {
        createDisplayModel();
        updateDisplayModelVisibility();
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
    else if (changedField == &m_disableLighting)
    {
        createDisplayModel();
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
    else if (changedField == &name)
    {
        updateViewerWidgetWindowTitle();
    }
    else if (changedField == &m_currentTimeStep)
    {
        if (m_viewer)
        {
            m_viewer->update();

            managedViewCollection->updateTimeStep(m_currentTimeStep);
        }
    }
    else if (changedField == &backgroundColor)
    {
        if (viewer() != NULL)
        {
            updateViewerWidget();
        }
    }
    else if (changedField == &maximumFrameRate)
    {
        // !! Use cvf::UNDEFINED_INT or something if we end up with frame rate 0?
        // !! Should be able to specify legal range for number properties
        if (m_viewer)
        {
            m_viewer->animationControl()->setTimeout(maximumFrameRate != 0 ? 1000/maximumFrameRate : INT_MAX);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::addWellPathsToModel(cvf::ModelBasicList* wellPathModelBasicList, 
                                  const cvf::Vec3d& displayModelOffset,  
                                  double characteristicCellSize, 
                                  const cvf::BoundingBox& wellPathClipBoundingBox, 
                                  cvf::Transform* scaleTransform)
{
    RimOilField* oilFields =                                    RiaApplication::instance()->project()   ? RiaApplication::instance()->project()->activeOilField() : NULL;
    RimWellPathCollection* wellPathCollection =                 oilFields                               ? oilFields->wellPathCollection() : NULL;
    RivWellPathCollectionPartMgr* wellPathCollectionPartMgr =   wellPathCollection                      ? wellPathCollection->wellPathCollectionPartMgr() : NULL;

    if (wellPathCollectionPartMgr)
    {
        wellPathCollectionPartMgr->appendStaticGeometryPartsToModel(wellPathModelBasicList, 
                                                                    displayModelOffset,
                                                                    scaleTransform, 
                                                                    characteristicCellSize, 
                                                                    wellPathClipBoundingBox);
    }

    wellPathModelBasicList->updateBoundingBoxesRecursive();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCellRangeFilterCollection* RimView::rangeFilterCollection()
{
    if (m_overrideRangeFilterCollection)
    {
        return m_overrideRangeFilterCollection;
    }
    else
    {
        return m_rangeFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimCellRangeFilterCollection* RimView::rangeFilterCollection() const
{
    if (m_overrideRangeFilterCollection)
    {
        return m_overrideRangeFilterCollection;
    }
    else
    {
        return m_rangeFilterCollection;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimView::setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc)
{
    m_overrideRangeFilterCollection = rfc;
}
