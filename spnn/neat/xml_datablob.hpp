#ifndef NEAT_XML_DATABLOB_HPP_INCLUDED
#define NEAT_XML_DATABLOB_HPP_INCLUDED

#include <string>

namespace neat
{
    namespace xml
    {
        class BlobReference;
        class DataBlob;
    }
}

#include "xml.hpp"

namespace neat
{
    namespace xml
    {
        class BlobReference
        {
            private:

                size_t position;
                size_t length;

            public:

                static bool ContainsBlobRef( const rapidxml::xml_node<> * data_node );

                BlobReference() : BlobReference( 0, 0 ) { /*  */ }
                explicit BlobReference( size_t pos, size_t len ) : position( pos ), length( len ) { /*  */ }
                BlobReference( const rapidxml::xml_node<> * data_node );

                bool is_null() const;

                void SaveToXML_asAttributes( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool );

                inline size_t size() const { return length; }

            protected:

                inline size_t get_pos() const { return position; }
                inline size_t get_len() const { return length; }

                friend class DataBlob;
        };

        class DataBlob
        {
            private:

                std::string data;

            public:

                DataBlob();
                DataBlob( const rapidxml::xml_node<> * data_blob_node );

                size_t size() const;

                BlobReference add_data( const unsigned char* ptr, size_t len );
                BlobReference add_data( const std::string& str );

                template < typename T >
                BlobReference add_data( const T& data_in );

                template < typename T >
                BlobReference add_data( const std::vector<T>& data_vec_in );

                template < typename T >
                void get_data( const BlobReference& ref, T& data_out ) const;

                template < typename T >
                void get_data( const BlobReference& ref, std::vector<T>& data_vec_out ) const;

                std::string extract_data_copy( const BlobReference& ref ) const;
                void extract_data_raw( const BlobReference& ref, unsigned char * ptr_out ) const;
                void extract_data_raw( const BlobReference& ref, unsigned char * ptr_out, size_t & length ) const;

                size_t hash() const;
                size_t hash_of( const BlobReference& ref ) const;

                void SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool, const std::vector< std::string >& encoding_sequence = {} ) const;
        };
    }
}

#include "xml_datablob.inl"

#endif // NEAT_XML_DATABLOB_HPP_INCLUDED
