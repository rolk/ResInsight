/*
 Copyright 2016 Statoil ASA.

 This file is part of the Open Porous Media project (OPM).

 OPM is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 OPM is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPM_ECLIPSE_CONFIG_HPP
#define OPM_ECLIPSE_CONFIG_HPP

#include <memory>

#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/EclipseState/InitConfig/InitConfig.hpp>
#include <opm/parser/eclipse/EclipseState/SimulationConfig/SimulationConfig.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/RestartConfig.hpp>

namespace Opm {

    class Deck;
    class GridDims;
    class Eclipse3DProperties;
    class IOConfig;
    class ParseContext;

    class EclipseConfig
    {
    public:
        EclipseConfig(const Deck& deck,
                      const Eclipse3DProperties& eclipse3DProperties,
                      const GridDims& gridDims,
                      const Schedule& schedule,
                      const ParseContext& parseContext);

        const InitConfig& init() const;
        const IOConfig& io() const;
        IOConfig& io();
        const SimulationConfig & simulation() const;
        const SummaryConfig& summary() const;
        const RestartConfig& restart() const;

        std::shared_ptr<const IOConfig> getIOConfigConst() const;
        std::shared_ptr<IOConfig> getIOConfig() const;
        const InitConfig& getInitConfig() const;
        const SimulationConfig & getSimulationConfig() const;
        const SummaryConfig& getSummaryConfig() const;
        const RestartConfig& getRestartConfig() const;

    private:
        std::shared_ptr<IOConfig> m_ioConfig;
        const InitConfig m_initConfig;
        const SimulationConfig m_simulationConfig;
        SummaryConfig m_summaryConfig;
        RestartConfig m_restartConfig;
    };
}

#endif // OPM_ECLIPSE_CONFIG_HPP
