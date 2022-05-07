#ifndef GETORDEFAULT_H
#define GETORDEFAULT_H

#include <utility>

template<class M, class K>
inline auto getOrDefault(M &&map, const K &key, const decltype(std::declval<M>().at(std::declval<K>())) &def) -> decltype(std::declval<M>().at(std::declval<K>())) {
    auto it = map.find(key);
    return (it != map.end() ? it->second : def);
}

#endif // GETORDEFAULT_H
