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

#pragma once

#include "RigFemResultAddress.h"

#include "cvfCollection.h"
#include "cvfObject.h"
#include <map>
#include <vector>

class RifGeoMechReaderInterface;
class RigFemScalarResultFrames;
class RigFemPartResultsCollection;
class RigFemPartResults;
class RigStatisticsDataCache;

class RigFemPartResultsCollection: public cvf::Object
{
public:
    RigFemPartResultsCollection(RifGeoMechReaderInterface* readerInterface, int partCount);
    ~RigFemPartResultsCollection();

    std::map<std::string, std::vector<std::string> > scalarFieldAndComponentNames(RigFemResultPosEnum resPos);
    std::vector<std::string>                         stepNames();
    bool                                             assertResultsLoaded(const RigFemResultAddress& resVarAddr);
    const std::vector<float>&                        resultValues(const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex); 

    int                                              frameCount();


    void                                             minMaxScalarValues (const RigFemResultAddress& resVarAddr, int frameIndex,  double* localMin, double* localMax);
    void                                             posNegClosestToZero(const RigFemResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero);
    void                                             minMaxScalarValues (const RigFemResultAddress& resVarAddr, double* globalMin, double* globalMax);
    void                                             posNegClosestToZero(const RigFemResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero);
    void                                             meanScalarValue(const RigFemResultAddress& resVarAddr, double* meanValue);
    void                                             p10p90ScalarValues(const RigFemResultAddress& resVarAddr, double* p10, double* p90);
    const std::vector<size_t>&                       scalarValuesHistogram(const RigFemResultAddress& resVarAddr);

private:
    RigFemScalarResultFrames*                        findOrLoadScalarResult(int partIndex,
                                                                            const RigFemResultAddress& resVarAddr);

    RigFemScalarResultFrames*                        calculateDerivedResult(int partIndex, const RigFemResultAddress& resVarAddr);

    friend class RigFemNativeStatCalc;                                                                                      
    cvf::Collection<RigFemPartResults>               m_femPartResults;
    cvf::ref<RifGeoMechReaderInterface>              m_readerInterface;

    RigStatisticsDataCache*                          statistics(const RigFemResultAddress& resVarAddr);
    std::vector< RigFemResultAddress>                getResAddrToComponentsToRead(const RigFemResultAddress& resVarAddr);
    std::map<RigFemResultAddress, cvf::ref<RigStatisticsDataCache> >  m_resultStatistics;
};



