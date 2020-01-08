#include <cstring>

#include "xml.hpp"
#include "xml_datablob.hpp"

#include "../../zlib/zlib.hpp"

namespace neat
{
    namespace xml
    {
        bool
        BlobReference::ContainsBlobRef( const rapidxml::xml_node<> * data_node )
        {
            return FindAttribute( "__blob_ref", data_node ) != nullptr;
        }

        BlobReference::BlobReference( const rapidxml::xml_node<> * data_node )
         : BlobReference()
        {
            std::string data = xml::GetAttributeValue<std::string>( "__blob_ref", data_node );
            auto colon_pos = data.find(':');
            position = xml::from_string<size_t>( data.substr( 0, colon_pos ) );
            length   = xml::from_string<size_t>( data.substr( colon_pos + 1 ) );
        }

        bool
        BlobReference::is_null() const
        {
            return position == 0 && length == 0;
        }

        void
        BlobReference::SaveToXML_asAttributes( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            std::ostringstream ss;
            ss << position << ":" << length;
            destination->append_attribute( Attribute( "__blob_ref", ss.str(), mem_pool ) );
        }


        DataBlob::DataBlob()
         : data()
        {
            /*  */
        }

        DataBlob::DataBlob( const rapidxml::xml_node<> * data_blob_node )
         : DataBlob()
        {
            assert( data_blob_node && neat::xml::Name( data_blob_node ) == "data_blob" );

            {
                auto blob_node = FindNode( "blob", data_blob_node );
                size_t expected_length = GetAttributeValue<size_t>( "length", blob_node );
                std::string encoding_types = GetAttributeValue<std::string>( "encoding", blob_node );

                // extract the data
                {
                    data = "";
                    auto block_node = FindNode( "block", blob_node );
                    if( block_node != nullptr )
                    {
                        while( block_node != nullptr )
                        {
                            if( Name( block_node ) != "block" )
                            {
                                continue;
                            }
                            data.append( Value( block_node ) );
                            block_node = block_node->next_sibling();
                        }
                    }
                    else
                    {
                        data = Value( blob_node );
                    }
                }

                std::istringstream ss( encoding_types );
                std::string encoding_type;
                while( std::getline( ss, encoding_type, ':' ) )
                {
                    if( encoding_type == "base64" )
                    {
                        data = base64_decode( data );
                    }
                    else if( encoding_type == "zlib" )
                    {
                        data = zlib::decompress_string( data );
                    }
                    else if( encoding_type == "zlib-0" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-1" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-2" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-3" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-4" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-5" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-6" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-7" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-8" ) { data = zlib::decompress_string( data ); }
                    else if( encoding_type == "zlib-9" ) { data = zlib::decompress_string( data ); }
                    /*else if( encoding_type == "" )
                    {
                        ;
                    }*/
                    else
                    {
                        // error unknown encoding type
                        throw std::invalid_argument( "Unknown DataBlob data encoding type." );
                    }
                }

                assert( expected_length == size() );
            }

            {
                std::string hash_str = "";
                readSimpleValueNode( "hash", hash_str, data_blob_node );

                std::ostringstream current_hash_str;
                current_hash_str << std::hex << hash();

                assert( current_hash_str.str() == hash_str );
            }
        }

        size_t
        DataBlob::size() const
        {
            return data.size();
        }

        BlobReference
        DataBlob::add_data( const unsigned char* ptr, size_t len )
        {
            if( ptr == nullptr || len == 0 )
            {
                return BlobReference();
            }

            BlobReference out_ref( size(), len );

            data.append( reinterpret_cast<const char*>( ptr ), len );

            return out_ref;
        }

        BlobReference
        DataBlob::add_data( const std::string& str )
        {
            if( str.empty() )
            {
                return BlobReference();
            }

            BlobReference out_ref( size(), str.size() );

            data.append( str );

            return out_ref;
        }

        std::string
        DataBlob::extract_data_copy( const BlobReference& ref ) const
        {
            size_t pos = ref.get_pos();
            size_t len = ref.get_len();

            if( ref.is_null() || pos >= size() || pos + len > size() )
            {
                throw std::out_of_range( "BlobReference ill-defined for DataBlob." );
            }

            return data.substr( pos, len );
        }

        void
        DataBlob::extract_data_raw( const BlobReference& ref, unsigned char * ptr_out ) const
        {
            size_t pos = ref.get_pos();
            size_t len = ref.get_len();

            if( ref.is_null() || pos >= size() || pos + len > size() )
            {
                throw std::out_of_range( "BlobReference ill-defined for DataBlob." );
            }

            std::memcpy( ptr_out, reinterpret_cast<const void*>( &data[pos] ), len );
        }

        void
        DataBlob::extract_data_raw( const BlobReference& ref, unsigned char * ptr_out, size_t& length ) const
        {
            extract_data_raw( ref, ptr_out );
            length = ref.get_len();
        }

        inline
        size_t
        __large_string_hash( const std::string& str_in )
        {
            // maximum string size that std::hash< std::string >{}() can handle ( at least on my compiler/stdlib combo )
            const size_t chunk_size = 0x7fffffff;

            // if less than chunk size do normal hash, this is the break case
            if( str_in.size() <= chunk_size )
            {
                return std::hash< std::string >{}( str_in );
            }

            // intermediate store of chunk hashes
            std::string other_hashes;

            size_t start_pos = 0;
            while( start_pos < str_in.size() )
            {
                // get the hash for the current chunk
                size_t chunk_hash = __large_string_hash( str_in.substr( start_pos, chunk_size ) );

                // convert the hash number to char string
                char * chunk_hash_data = reinterpret_cast< char * >( &chunk_hash );

                // append hash byte-wise to the intermediate string
                for( size_t i = 0; i < sizeof( size_t ); ++i )
                {
                    other_hashes += chunk_hash_data[i];
                }

                // increment chunk
                start_pos += chunk_size;
            }

            // hash the hashes of the chunks, we recurse in the _extremly_ unlikely case that the intermediate hash string is larger than chunk size
            return __large_string_hash( other_hashes );
        }

        size_t
        DataBlob::hash() const
        {
            // note std::hash< std::string >{}() crashes for strings larger than 2^31 bytes
            //return std::hash< std::string >{}( data );

            // custom large string hash
            return __large_string_hash( data );
        }

        size_t
        DataBlob::hash_of( const BlobReference& ref ) const
        {
            // note std::hash< std::string >{}() crashes for strings larger than 2^31 bytes
            //return std::hash< std::string >{}( extract_data_copy( ref ) );

            // custom large string hash
            return __large_string_hash( extract_data_copy( ref ) );
        }

        void
        DataBlob::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool, const std::vector< std::string >& encoding_sequence ) const
        {
            auto data_blob_node = Node( "data_blob", "", mem_pool );

            {
                std::ostringstream ss;
                ss << std::hex << hash();
                appendSimpleValueNode( "hash", ss.str(), data_blob_node, mem_pool );
            }

            {
                auto blob_node = Node( "blob", "", mem_pool );
                blob_node->append_attribute( Attribute( "length", to_string( size() ), mem_pool ) );

                std::string encoded_data = data;
                {
                    std::string encoding = "";

                    for( auto enc_type = encoding_sequence.begin(); enc_type != encoding_sequence.end(); ++enc_type )
                    {
                        if( *enc_type == "base64" )
                        {
                            encoded_data = base64_encode( encoded_data );
                        }
                        else if( *enc_type == "zlib" )
                        {
                            encoded_data = zlib::compress_string( encoded_data, -1 );
                        }
                        else if( *enc_type == "zlib-0" ) { encoded_data = zlib::compress_string( encoded_data, 0 ); }
                        else if( *enc_type == "zlib-1" ) { encoded_data = zlib::compress_string( encoded_data, 1 ); }
                        else if( *enc_type == "zlib-2" ) { encoded_data = zlib::compress_string( encoded_data, 2 ); }
                        else if( *enc_type == "zlib-3" ) { encoded_data = zlib::compress_string( encoded_data, 3 ); }
                        else if( *enc_type == "zlib-4" ) { encoded_data = zlib::compress_string( encoded_data, 4 ); }
                        else if( *enc_type == "zlib-5" ) { encoded_data = zlib::compress_string( encoded_data, 5 ); }
                        else if( *enc_type == "zlib-6" ) { encoded_data = zlib::compress_string( encoded_data, 6 ); }
                        else if( *enc_type == "zlib-7" ) { encoded_data = zlib::compress_string( encoded_data, 7 ); }
                        else if( *enc_type == "zlib-8" ) { encoded_data = zlib::compress_string( encoded_data, 8 ); }
                        else if( *enc_type == "zlib-9" ) { encoded_data = zlib::compress_string( encoded_data, 9 ); }
                        /*else if( *enc_type == "" )
                        {
                            ;
                        }*/
                        else
                        {
                            continue;
                        }

                        encoding = *enc_type + std::string( encoding.empty() ? "" : ":" ) + encoding;
                    }

                    // finish with base64
                    {
                        encoded_data = base64_encode( encoded_data );
                        encoding = "base64" + std::string( encoding.empty() ? "" : ":" ) + encoding;
                    }

                    blob_node->append_attribute( Attribute( "encoding", encoding, mem_pool ) );
                }

                {
                    const size_t block_size = 1024*1024*1;
                    size_t pos = 0;
                    while( pos < encoded_data.size() )
                    {
                        blob_node->append_node( Node( "block", encoded_data.substr( pos, block_size ), mem_pool ) );
                        //blob_node->append_node( mem_pool->allocate_node( rapidxml::node_element, "block", mem_pool->allocate_string( &encoded_data[pos], block_size ), 0, std::min<size_t>( block_size, encoded_data.size() - pos ) ) );
                        pos += block_size;
                    }
                }

                data_blob_node->append_node( blob_node );
            }

            destination->append_node( data_blob_node );
        }
    }
}
