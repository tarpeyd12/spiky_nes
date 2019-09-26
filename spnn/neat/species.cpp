#include <iostream>
#include <string>

#include "species.hpp"

#include "xml.hpp"

namespace neat
{

    SpeciesManager::SpeciesManager( const SpeciesDistanceParameters& th, SpeciationMethod specMethod )
         : speciesCounter_mutex(), speciesCounter( 0 ), speciationMethod( specMethod ), historicSpeciesIDMapList(), speciesIDMap_mutex(), speciesIDMap(), classificationParameters( th )
    {
        assert( getClassificationParameters().threshold > 0.0 );

        /*  */
    }



    SpeciesManager::SpeciesManager( const SpeciesManager& other )
         : speciesCounter_mutex(), speciesCounter( 0 ), speciationMethod( other.speciationMethod ), historicSpeciesIDMapList(), speciesIDMap_mutex(), speciesIDMap(), classificationParameters( other.classificationParameters )
    {
        // copy constructor

        assert( getClassificationParameters().threshold > 0.0 );

        // define locks for this and other
        std::shared_lock< std::shared_timed_mutex > other_map_lock( other.speciesIDMap_mutex, std::defer_lock );
        std::unique_lock< std::shared_timed_mutex > this_map_lock( speciesIDMap_mutex, std::defer_lock );

        std::unique_lock< std::mutex > other_count_lock( other.speciesCounter_mutex, std::defer_lock );
        std::unique_lock< std::mutex > this_count_lock( speciesCounter_mutex, std::defer_lock );

        // lock them all for data copy
        std::lock( other_map_lock, this_map_lock, other_count_lock, this_count_lock );

        // copy the data, hopefully the shared pointers all point to the same stuff correctly
        speciesCounter = other.speciesCounter;
        historicSpeciesIDMapList = other.historicSpeciesIDMapList;
        speciesIDMap = other.speciesIDMap;
    }

    void
    SpeciesManager::updateSpeciesArchtype( SpeciesID id, const NetworkGenotype& genotype )
    {
        // lock for writing
        std::unique_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        // call the non-locking method to update the archetype
        __updateSpeciesArchtype_no_lock( id, genotype );
    }

    void
    SpeciesManager::archiveSpeciesArchetypes( uint64_t generation, bool significantChangesOnly )
    {
        // lock for writing
        std::unique_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        for( const auto& currentSpecies : speciesIDMap )
        {
            auto ID = currentSpecies.first;
            auto genotype = currentSpecies.second;

            auto it = historicSpeciesIDMapList.find( ID );
            if( it != historicSpeciesIDMapList.end() )
            {
                if( significantChangesOnly && !it->second.empty() && it->second.back().second != nullptr )
                {
                    double d = NetworkGenotypeDistance( *genotype, *it->second.back().second, getClassificationParameters() );
                    if( !(d < 0.0) && !(d > 0.0) )
                    {
                        it->second.push_back( { generation, genotype } );
                    }
                }
                else
                {
                    it->second.push_back( { generation, genotype } );
                }
            }
            else
            {
                historicSpeciesIDMapList[ ID ].push_back( { generation, genotype } );
            }
        }
        speciesIDMap.clear();
    }

    const std::shared_ptr< NetworkGenotype >
    SpeciesManager::getSpeciesArchetype( SpeciesID id ) const
    {
        // lock for reading
        std::shared_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        if( id >= getLastSpeciesID() )
        {
            return nullptr;
        }

        auto it = speciesIDMap.find( id );

        if( it == speciesIDMap.end() )
        {
            return nullptr;
        }

        return it->second;
    }

    SpeciesID
    SpeciesManager::getSpeciesID( const NetworkGenotype& genotype )
    {
        // find the closest species to the given genotype

        // lock for writing
        std::unique_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        auto min_it = speciesIDMap.begin();
        double min_dist = 0.0; // not the actual initial value

        for( auto it = speciesIDMap.begin(); it != speciesIDMap.end(); ++it )
        {
            double d = NetworkGenotypeDistance( genotype, *it->second, getClassificationParameters() );
            if( d < min_dist || it == speciesIDMap.begin() )
            {
                min_dist = d;
                min_it = it;
            }
        }

        if( min_it == speciesIDMap.end() || min_dist > getClassificationParameters().threshold )
        {
            SpeciesID newSpeciesID = getNextSpeciesIDAndIncrement();
            //updateSpeciesArchtype( newSpeciesID, genotype ); // DOUBLE LOCKED????????

            // call the non-locking method to update the archetype
            __updateSpeciesArchtype_no_lock( newSpeciesID, genotype );

            return newSpeciesID;
        }

        return min_it->first;
    }

    std::vector< SpeciesID >
    SpeciesManager::getSpeciesIDsOfGenotypes( tpl::pool& thread_pool, const std::vector< NetworkGenotype >& genotypeList, bool /*preSpeciate*/ )
    {
        // lock for writing
        std::unique_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        struct id_package
        {
            size_t index;
            bool found_id;
            SpeciesID id;
        };

        // this only reads from speciesIDMap
        auto find_id_from_known_species_map = [this]( size_t index, const NetworkGenotype& genotype ) -> id_package
        {
            auto min_it = speciesIDMap.begin();
            double min_dist = 0.0; // not the actual initial value

            for( auto it = speciesIDMap.begin(); it != speciesIDMap.end(); ++it )
            {
                double d = NetworkGenotypeDistance( genotype, *it->second, getClassificationParameters() );
                if( d < min_dist || it == speciesIDMap.begin() )
                {
                    min_dist = d;
                    min_it = it;
                }
            }

            if( min_it == speciesIDMap.end() || min_dist > getClassificationParameters().threshold )
            {
                return { index, false, SpeciesID(~1) };
            }

            return { index, true, min_it->first };
        };

        std::vector< SpeciesID > out( genotypeList.size(), SpeciesID(~1) ); // initialize a full list, with max-value SpeciesID in preparation for data.

        std::list< tpl::future< id_package > > id_package_futures;

        for( size_t index = 0; index < genotypeList.size(); ++index )
        {
            id_package_futures.push_back( thread_pool.submit( find_id_from_known_species_map, index, std::cref( genotypeList[ index ] ) ) );
        }

        std::list< id_package > unidentified_ids;

        while( !id_package_futures.empty() )
        {
            id_package id_pack = id_package_futures.front().get();
            id_package_futures.pop_front();

            if( !id_pack.found_id )
            {
                unidentified_ids.push_back( id_pack );
                continue;
            }

            out[ id_pack.index ] = id_pack.id;
        }

        // handle unknowns
        {
            // this needs no locks
            auto find_id_from_single_known = [this]( const NetworkGenotype& genotype1, const NetworkGenotype& genotype2, SpeciesID id, id_package pack ) -> id_package
            {
                auto classific = getClassificationParameters();

                if( NetworkGenotypeDistance( genotype1, genotype2, classific ) > classific.threshold )
                {
                    return { pack.index, false, SpeciesID(~1) };
                }

                return { pack.index, true, id };
            };

            assert( id_package_futures.empty() );

            while( !unidentified_ids.empty() )
            {
                id_package known_pack = unidentified_ids.front();
                unidentified_ids.pop_front();

                // assign the first of the unknowns a new species ID
                {
                    SpeciesID newSpeciesID = getNextSpeciesIDAndIncrement();

                    // call the non-locking method to update the archetype
                    __updateSpeciesArchtype_no_lock( newSpeciesID, genotypeList[ known_pack.index ] );

                    known_pack.found_id = true;
                    known_pack.id = newSpeciesID;

                    out[ known_pack.index ] = newSpeciesID;
                }

                // check all the other unknowns against the new single known
                while( !unidentified_ids.empty() )
                {
                    id_package unknown_pack = unidentified_ids.front();
                    unidentified_ids.pop_front();

                    id_package_futures.push_back( thread_pool.submit( find_id_from_single_known, std::cref( genotypeList[ known_pack.index ] ), std::cref( genotypeList[ unknown_pack.index ] ), known_pack.id, unknown_pack ) );
                }

                while( !id_package_futures.empty() )
                {
                    id_package id_pack = id_package_futures.front().get();
                    id_package_futures.pop_front();

                    if( !id_pack.found_id )
                    {
                        unidentified_ids.push_back( id_pack );
                        continue;
                    }

                    out[ id_pack.index ] = id_pack.id;
                }

            }
        }

        return out;
    }

    SpeciesDistanceParameters
    SpeciesManager::getClassificationParameters() const
    {
        return classificationParameters;
    }

    size_t
    SpeciesManager::getNumTrackedSpecies() const
    {
        // lock for reading
        std::shared_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        size_t count = historicSpeciesIDMapList.size();

        for( auto it : speciesIDMap )
        {
            if( historicSpeciesIDMapList.find( it.first ) == historicSpeciesIDMapList.end() )
            {
                count++;
            }
        }

        return count;
    }

    void
    SpeciesManager::printSpeciesArchetypes( std::ostream& out )
    {
        // lock for reading
        std::shared_lock< std::shared_timed_mutex > lock( speciesIDMap_mutex );

        //rapidxml::xml_document<> doc;

        for( auto it : speciesIDMap )
        {
            out << "Species " << it.first << ":\n";
            it.second->printGenotype( out );
        }
    }

    void
    SpeciesManager::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
    {
        std::unique_lock< std::shared_timed_mutex > map_lock( speciesIDMap_mutex, std::defer_lock );
        std::unique_lock< std::mutex > count_lock( speciesCounter_mutex, std::defer_lock );
        std::lock( map_lock, count_lock );

        // note: classificationParameters should already be handled by neat::Population

        auto node = xml::Node( "species_tracker", "", mem_pool );
        xml::appendSimpleValueNode( "species_counter", speciesCounter, node, mem_pool );
        xml::appendSimpleValueNode<size_t>( "method", static_cast<size_t>( speciationMethod ), node, mem_pool );

        auto species_node = xml::Node( "species_archetypes", "", mem_pool );
        species_node->append_attribute( xml::Attribute( "N", xml::to_string( speciesIDMap.size() ), mem_pool ) );

        for( const auto& s : speciesIDMap )
        {
            auto archtype = xml::Node( "species", "", mem_pool );
            archtype->append_attribute( xml::Attribute( "ID", xml::to_string( s.first ), mem_pool ) );
            xml::Encode_NetworkGenotype( *s.second, archtype, mem_pool );
            species_node->append_node( archtype );
        }
        node->append_node( species_node );
        destination->append_node( node );
    }

    SpeciesManager::SpeciesManager( const rapidxml::xml_node<> * species_manager_node, const SpeciesDistanceParameters& th )
         : SpeciesManager( th, SpeciationMethod::FirstForward ) // first forward is temporary
    {
        assert( species_manager_node && neat::xml::Name( species_manager_node ) == "population" );

        xml::readSimpleValueNode( "species_counter", speciesCounter, species_manager_node );
        speciationMethod = static_cast< SpeciationMethod >( xml::GetAttributeValue< size_t >( "value", xml::FindNode( "method", species_manager_node ) ) );

        auto species_archatype_node = xml::FindNode( "species_archetypes", species_manager_node );

        size_t _expected_num_species = xml::GetAttributeValue<size_t>( "N", species_archatype_node );

        auto species_node = species_archatype_node->first_node();

        while( species_node != nullptr )
        {
            if( xml::Name( species_node ) == "species" )
            {
                species_node = species_node->next_sibling();
                continue;
            }

            SpeciesID species_id = xml::GetAttributeValue<SpeciesID>( "ID", species_node );

            speciesIDMap[ species_id ] = std::make_shared< NetworkGenotype >( xml::Decode_NetworkGenotype( xml::FindNode( "genotype", species_node ) ) );

            species_node = species_node->next_sibling();
        }

        assert( speciesIDMap.size() == _expected_num_species );
    }

    SpeciesID
    SpeciesManager::getNextSpeciesIDAndIncrement()
    {
        std::unique_lock< std::mutex > lock( speciesCounter_mutex );

        return speciesCounter++;
    }

    SpeciesID SpeciesManager::getLastSpeciesID() const
    {
        std::unique_lock< std::mutex > lock( speciesCounter_mutex );

        return speciesCounter;
    }

    void
    SpeciesManager::__updateSpeciesArchtype_no_lock( SpeciesID id, const NetworkGenotype& genotype )
    {
        // if the id does not exist, then add it, if it does exist overwrite it
        speciesIDMap[ id ] = std::make_shared< NetworkGenotype >( genotype );
    }
}
