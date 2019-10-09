#include <cstring>

#include "xml.hpp"
#include "xml_datablob.hpp"

namespace neat
{
    namespace xml
    {
        bool
        BlobReference::ContainsBlobRef( const rapidxml::xml_node<> * data_node )
        {
            return FindAttribute( "__blob_pos", data_node ) != nullptr && FindAttribute( "__blob_len", data_node ) != nullptr;
        }

        BlobReference::BlobReference( const rapidxml::xml_node<> * data_node )
         : BlobReference()
        {
            position = xml::GetAttributeValue<size_t>( "__blob_pos", data_node );
            length   = xml::GetAttributeValue<size_t>( "__blob_len", data_node );
        }

        bool
        BlobReference::is_null() const
        {
            return position == 0 && length == 0;
        }

        void
        BlobReference::SaveToXML_asAttributes( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool )
        {
            destination->append_attribute( Attribute( "__blob_pos", xml::to_string( position ), mem_pool ) );
            destination->append_attribute( Attribute( "__blob_len", xml::to_string( length ), mem_pool ) );
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
                std::string encoding_type = GetAttributeValue<std::string>( "encoding", blob_node );

                if( encoding_type == "base-64" )
                {
                    data = base64_decode( Value( blob_node ) );
                }
                else
                {
                    // error unknown encoding type
                    throw std::invalid_argument( "Unknown DataBlob data encoding type." );
                }

                assert( expected_length == size() );
            }

            {
                std::string hash_str = "";
                readSimpleValueNode( "hash", hash_str, data_blob_node );

                std::stringstream current_hash_str;
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

        size_t
        DataBlob::hash() const
        {
            return std::hash< std::string >{}( data );
        }

        size_t
        DataBlob::hash_of( const BlobReference& ref ) const
        {
            return std::hash< std::string >{}( extract_data_copy( ref ) );
        }

        void
        DataBlob::SaveToXML( rapidxml::xml_node<> * destination, rapidxml::memory_pool<> * mem_pool ) const
        {
            auto data_blob_node = Node( "data_blob", "", mem_pool );

            {
                std::stringstream ss;
                ss << std::hex << hash();
                appendSimpleValueNode( "hash", ss.str(), data_blob_node, mem_pool );
            }

            {
                auto blob_node = Node( "blob", base64_encode( data ), mem_pool );
                blob_node->append_attribute( Attribute( "length", to_string( size() ), mem_pool ) );
                blob_node->append_attribute( Attribute( "encoding", "base-64", mem_pool ) );
                data_blob_node->append_node( blob_node );
            }

            destination->append_node( data_blob_node );
        }
    }
}
