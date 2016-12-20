
#pragma once


#include <opm/flowdiagnostics/ConnectivityGraph.hpp>
#include <opm/flowdiagnostics/ConnectionValues.hpp>
#include <opm/flowdiagnostics/Toolbox.hpp>

#include <opm/utility/ECLGraph.hpp>
#include <opm/utility/ECLWellSolution.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace RigFlowDiagInterfaceTools {


    inline Opm::FlowDiagnostics::ConnectionValues
    extractFluxField(const Opm::ECLGraph& G)
    {
        using ConnVals = Opm::FlowDiagnostics::ConnectionValues;

        using NConn = ConnVals::NumConnections;
        using NPhas = ConnVals::NumPhases;

        const auto nconn = NConn{G.numConnections()};
        const auto nphas = NPhas{3};

        auto flux = ConnVals(nconn, nphas);

        auto phas = ConnVals::PhaseID{0};

        for (const auto& p : { Opm::ECLGraph::PhaseIndex::Aqua   ,
                               Opm::ECLGraph::PhaseIndex::Liquid ,
                               Opm::ECLGraph::PhaseIndex::Vapour })
        {
            const std::vector<double> pflux = G.flux(p);

            if (! pflux.empty()) {
                assert (pflux.size() == nconn.total);

                auto conn = ConnVals::ConnID{0};

                for (const auto& v : pflux) {
                    flux(conn, phas) = v;

                    conn.id += 1;
                }
            }

            phas.id += 1;
        }

        return flux;
    }

    template <class WellFluxes>
    Opm::FlowDiagnostics::CellSetValues
    extractWellFlows(const Opm::ECLGraph& G,
                     const WellFluxes&    well_fluxes)
    {
        Opm::FlowDiagnostics::CellSetValues inflow;
        for (const auto& well : well_fluxes) {
            for (const auto& completion : well.completions) {
                const int grid_index = completion.grid_index;
                const auto& ijk = completion.ijk;
                const int cell_index = G.activeCell(ijk, grid_index);
                if (cell_index >= 0) {
                    inflow.emplace(cell_index, completion.reservoir_inflow_rate);
                }
            }
        }

        return inflow;
    }

    namespace Hack {
        inline Opm::FlowDiagnostics::ConnectionValues
        convert_flux_to_SI(Opm::FlowDiagnostics::ConnectionValues&& fl)
        {
            using Co = Opm::FlowDiagnostics::ConnectionValues::ConnID;
            using Ph = Opm::FlowDiagnostics::ConnectionValues::PhaseID;

            const auto nconn = fl.numConnections();
            const auto nphas = fl.numPhases();

            for (auto phas = Ph{0}; phas.id < nphas; ++phas.id) {
                for (auto conn = Co{0}; conn.id < nconn; ++conn.id) {
                    fl(conn, phas) /= 86400;
                }
            }

            return fl;
        }
    }

    inline Opm::FlowDiagnostics::Toolbox
    initialiseFlowDiagnostics(const Opm::ECLGraph& G)
    {
        const  Opm::FlowDiagnostics::ConnectivityGraph connGraph = 
           Opm::FlowDiagnostics::ConnectivityGraph{ static_cast<int>(G.numCells()),
                                                    G.neighbours() };

        // Create the Toolbox.

        Opm::FlowDiagnostics::Toolbox tool = Opm::FlowDiagnostics::Toolbox{ connGraph };

        tool.assignPoreVolume(G.poreVolume());

        Opm::FlowDiagnostics::ConnectionValues connectionsVals = Hack::convert_flux_to_SI(extractFluxField(G));
        tool.assignConnectionFlux(connectionsVals);

        Opm::ECLWellSolution wsol = Opm::ECLWellSolution{};

        const std::vector<Opm::ECLWellSolution::WellData> well_fluxes =
            wsol.solution(G.rawResultData(), G.numGrids());

        tool.assignInflowFlux(extractWellFlows(G, well_fluxes));

        return tool;
    }

  

}



