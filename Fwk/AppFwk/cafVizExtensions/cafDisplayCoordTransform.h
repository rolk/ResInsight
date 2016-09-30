
#pragma once

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

namespace caf {

//==================================================================================================
//
//
//==================================================================================================
class DisplayCoordTransform : public cvf::Object
{
public:
    DisplayCoordTransform();

    void setScale(const cvf::Vec3d& scale);
    void setTranslation(const cvf::Vec3d& translation);

    cvf::Vec3d transformToDisplayCoord(const cvf::Vec3d& domainCoord) const;
    cvf::Vec3d scaleToDisplaySize(const cvf::Vec3d& domainSize) const;

    cvf::Vec3d transformToDomainCoord(const cvf::Vec3d& displayCoord) const;
    cvf::Vec3d scaleToDomainSize(const cvf::Vec3d& displaySize) const;

private:
    cvf::Vec3d m_scale;
    cvf::Vec3d m_translation;
};

}
