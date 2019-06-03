#ifndef NEAT_MUTATION_HPP_INCLUDED
#define NEAT_MUTATION_HPP_INCLUDED

namespace neat
{
    namespace Mutations
    {
        class Mutation_base;
    }
}

#include "network.hpp"
#include "innovation_generator.hpp"
#include "neat.hpp"

namespace neat
{
    namespace Mutations
    {

        long double Gaussian( std::shared_ptr< Rand::RandomFunctor > rand );

        struct Mutation_base
        {
            public:

                Mutation_base() = default;

                // virtual destructor
                virtual ~Mutation_base() = default;

                // actual function
                virtual uint64_t operator()( NetworkGenotype& genotypeToMutate, InnovationGenerator& innovationTracker, const MutationRates& rate, const MutationLimits& limits, std::shared_ptr< Rand::RandomFunctor > rand = nullptr ) const = 0;

            protected:

                // support, 'cause we need to get to the protected data in the NodeGenotype, and I'm not adding 20+ friends to it
                static std::vector< NodeDef >& GetNodeList( NetworkGenotype& genotype );
                static std::vector< ConnectionDef >& GetConnList( NetworkGenotype& genotype );
        };

        //  of mutations

        class Mutation_Multi : public Mutation_base
        {
            private:

                struct MutatorChances
                {
                    double baseChance;
                    double nodeChance;
                    double connChance;
                    std::shared_ptr< Mutation_base > mutator; // using shared pointers because I'm not sure who to give the data lifetime responsibility to
                };

                std::vector< MutatorChances > mutators;

            public:

                Mutation_Multi();
                ~Mutation_Multi() = default;

                size_t numMutators() const;
                size_t addMutator( double chance, std::shared_ptr< Mutation_base > mutator );
                size_t addMutator( double baseChance, double nodeChance, double connChance, std::shared_ptr< Mutation_base > mutator );

                template < typename mutatorType >
                size_t addMutator( double chance ) { return addMutator( chance, std::make_shared< mutatorType >() ); }

                template < typename mutatorType >
                size_t addMutator( double baseChance, double nodeChance, double connChance ) { return addMutator( baseChance, nodeChance, connChance, std::make_shared< mutatorType >() ); }

                uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override;
        };

        class Mutation_Multi_one : public Mutation_base
        {
            private:

                std::vector< std::shared_ptr< Mutation_base > > mutators;

            public:

                Mutation_Multi_one();
                ~Mutation_Multi_one() = default;

                size_t numMutators() const;
                size_t addMutator( std::shared_ptr< Mutation_base > mutator );

                template < typename mutatorType >
                size_t addMutator() { return addMutator( std::make_shared< mutatorType >() ); }

                uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override;
        };

        // Node property mutations

        struct Mutation_Node_thresh_min       : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_thresh_min_new   : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_thresh_max       : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_thresh_max_new   : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_decays_value     : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_decays_value_new : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_decays_activ     : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_decays_activ_new : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_pulses_fast      : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_pulses_fast_new  : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_pulses_slow      : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Node_pulses_slow_new  : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };

        // Connection property mutations

        struct Mutation_Conn_weight        : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Conn_weight_new    : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Conn_length        : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Conn_length_new    : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Conn_enable        : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };

        // Structural mutations

        struct Mutation_Add_node           : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Add_conn           : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Add_conn_unique    : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Add_conn_dup       : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };

        struct Mutation_Add_conn_multi_in  : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };
        struct Mutation_Add_conn_multi_out : public Mutation_base { uint64_t operator()( NetworkGenotype&, InnovationGenerator&, const MutationRates&, const MutationLimits&, std::shared_ptr< Rand::RandomFunctor > ) const override; };

    }
}

#endif // NEAT_MUTATION_HPP_INCLUDED
