#ifndef PAIR_HASH_HPP
#define PAIR_HASH_HPP

struct pair_hash {
    template <typename T1, typename T2>
    size_t operator()( const std::pair<T1, T2>& pair_ ) const {

        auto first_hash = std::hash<T1>{}( pair_.first );
        auto second_hash = std::hash<T2>{}( pair_.second );

        return first_hash ^ ( second_hash << 1 );
    }
};

#endif
