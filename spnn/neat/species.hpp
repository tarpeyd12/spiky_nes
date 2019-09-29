#ifndef NEAT_SPECIES_HPP_INCLUDED
#define NEAT_SPECIES_HPP_INCLUDED

#include <map>
#include <list>
#include <shared_mutex>

namespace neat
{
    struct SpeciesDistanceParameters;
    class SpeciesManager;
    enum class SpeciationMethod : uint8_t { FirstForward = 0, FirstReverse, Closest, CloseEnough };
}

#include "network.hpp"
#include "neat.hpp"
#include "thread_pool.hpp"
#include "xml.hpp"

namespace neat
{
    struct SpeciesDistanceParameters
    {
        union { double c1, excess; };
        union { double c2, disjoint; };
        union { double c3, weights; };
        union { double c4, lengths; };

        union { double n1, activations; };
        union { double n2, decays; };
        union { double n3, pulses; };
        union { double n4, nodes; };

        union { double dt, threshold; };

        // TODO(dot##1/17/2019): implement this
        // SpeciesDistanceParameters NormalizeBy( const MutationLimits& ranges ) const;
    };

    class SpeciesManager
    {
        private:

            // FIXME(dot#9#2/27/2019): MAKE THIS THREAD SAFE

            const SpeciesID FirstAvailableSpecies = 1;

            mutable std::mutex speciesCounter_mutex;
            SpeciesID speciesCounter;

            SpeciationMethod speciationMethod;

        protected:

            std::map< SpeciesID, std::list< std::pair< uint64_t, std::shared_ptr< NetworkGenotype > > > > historicSpeciesIDMapList;

            mutable std::shared_timed_mutex speciesIDMap_mutex;
            std::map< SpeciesID, std::shared_ptr< NetworkGenotype > > speciesIDMap;

            std::map< SpeciesID, SpeciesID > speciesTaxonomyMap;

            SpeciesDistanceParameters classificationParameters;

        public:

            SpeciesManager( const SpeciesDistanceParameters& th, SpeciationMethod specMethod = SpeciationMethod::FirstForward );
            SpeciesManager( const rapidxml::xml_node<> * species_manager_node, const SpeciesDistanceParameters& th );

            SpeciesManager( const SpeciesManager& other );

            virtual ~SpeciesManager() = default;

            void updateSpeciesArchtype( SpeciesID id, const NetworkGenotype& genotype );
            void archiveSpeciesArchetypes( uint64_t generation, bool significantChangesOnly = true );

            const std::shared_ptr< NetworkGenotype > getSpeciesArchetype( SpeciesID id ) const;
            SpeciesID getSpeciesID( const NetworkGenotype& genotype );

            std::vector< SpeciesID > getSpeciesIDsOfGenotypes( tpl::pool& thread_pool, const std::vector< NetworkGenotype >& genotypeList, bool preSpeciate = true );

            SpeciesDistanceParameters getClassificationParameters() const;
            size_t getNumTrackedSpecies() const;

            void printSpeciesArchetypes( std::ostream& out );

            void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );

        private:

            SpeciesID getNextSpeciesIDAndIncrement();
            SpeciesID getLastSpeciesID() const;

            void __updateSpeciesArchtype_no_lock( SpeciesID id, const NetworkGenotype& genotype );

            //SpeciesID getSpeciesID( const NetworkGenotype& genotype ) const = delete;
    };

    double NetworkGenotypeDistance( const NetworkGenotype& networkA, const NetworkGenotype& networkB, const SpeciesDistanceParameters& params );
}

#endif // NEAT_SPECIES_HPP_INCLUDED
