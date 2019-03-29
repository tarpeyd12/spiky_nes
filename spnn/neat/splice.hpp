#ifndef NEAT_SPLICE_HPP_INCLUDED
#define NEAT_SPLICE_HPP_INCLUDED

#include "network.hpp"

namespace neat
{
    NetworkGenotype SpliceGenotypes( const NetworkGenotype& parent1, const NetworkGenotype& parent2, std::shared_ptr< Rand::RandomFunctor > rand = nullptr );
    NetworkGenotype SpliceGenotypes( const std::vector< const NetworkGenotype* >& genotypes, std::shared_ptr< Rand::RandomFunctor > rand = nullptr );
}

#endif // NEAT_SPLICE_HPP_INCLUDED
