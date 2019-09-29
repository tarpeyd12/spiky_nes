#include <vector>

#include "network.hpp"

namespace neat
{

    // treat this as the constructor of NetworkGenotype, I explicitly did not add a constructor like this to NetworkGenotype as to convey the idea of it just being data
    NetworkGenotype
    make_genotype( const std::vector< NodeDef >& nodes, const std::vector< ConnectionDef >& conns, const SpeciesID parent_species_id )
    {
        NetworkGenotype out;

        out.nodeGenotype = nodes;
        out.connectionGenotype = conns;

        out.parentSpeciesID = parent_species_id;

        return out;
    }

}

