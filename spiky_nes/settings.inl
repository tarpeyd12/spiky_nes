#ifndef SPKN_SETTINGS_INL_INCLUDED
#define SPKN_SETTINGS_INL_INCLUDED

namespace spkn
{
    template < typename T >
    T
    Variables::get( const std::string& key, const T& _default ) const
    {
        auto it = vars.find( key );
        if( it != vars.end() )
        {
            return neat::xml::from_string<T>( it->second );
        }
        return _default;
    }

    template < typename T >
    T
    Variables::operator()( const std::string& key, const T& _default ) const
    {
        return get<T>( key, _default );
    }
}

#endif // SPKN_SETTINGS_INL_INCLUDED
