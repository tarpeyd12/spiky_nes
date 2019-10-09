#ifndef NEAT_XML_DATABLOB_INL_INCLUDED
#define NEAT_XML_DATABLOB_INL_INCLUDED

namespace neat
{
    namespace xml
    {
        template < typename T >
        BlobReference
        DataBlob::add_data( const T& data_in )
        {
            return add_data( reinterpret_cast<const unsigned char*>( *data_in ), sizeof( T ) );
        }

        template < typename T >
        BlobReference
        DataBlob::add_data( const std::vector<T>& data_vec_in )
        {
            return add_data( reinterpret_cast<const unsigned char*>( data_vec_in.data() ), data_vec_in.size() * sizeof( T ) );
        }

        template < typename T >
        void
        DataBlob::get_data( const BlobReference& ref, T& data_out ) const
        {
            if( ref.get_len() != sizeof( T ) )
            {
                // error
            }
            extract_data_raw( ref, reinterpret_cast<unsigned char*>( data_out ) );
        }

        template < typename T >
        void
        DataBlob::get_data( const BlobReference& ref, std::vector<T>& data_vec_out ) const
        {
            if( ref.get_len() % sizeof( T ) != 0 )
            {
                // error
            }
            data_vec_out.clear();
            data_vec_out.resize( ref.get_len() / sizeof( T ) );
            extract_data_raw( ref, reinterpret_cast<unsigned char*>( data_vec_out.data() ) );
        }
    }
}

#endif // NEAT_XML_DATABLOB_INL_INCLUDED
