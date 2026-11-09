#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <unordered_map>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>

using namespace std;

// Tabla Hash / Diccionario (pares clave -> valor).
// Usa unordered_map (tabla hash, acceso promedio O(1)).
// Tambien ofrece toOrderedMap() para recorrer las claves en orden.
template <typename K, typename V>
class HashTable {
private:
    unordered_map<K, V> data;

public:
    // Inserta o actualiza el valor de una clave
    void put(const K& key, const V& value) { data[key] = value; }

    // Regresa el valor de una clave (error si no existe)
    V get(const K& key) const {
        auto it = data.find(key);
        if (it == data.end()) throw out_of_range("get(): la clave no existe");
        return it->second;
    }

    // Indica si la clave existe
    bool contains(const K& key) const { return data.find(key) != data.end(); }

    // Elimina una clave. Regresa true si existia.
    bool remove(const K& key) { return data.erase(key) > 0; }

    bool isEmpty() const { return data.empty(); }
    size_t size() const { return data.size(); }
    void clear() { data.clear(); }

    // Lista de claves (sin orden garantizado)
    vector<K> keys() const {
        vector<K> result;
        for (const auto& par : data) result.push_back(par.first);
        return result;
    }

    // Pares clave-valor ordenados por clave (usa map)
    map<K, V> toOrderedMap() const {
        return map<K, V>(data.begin(), data.end());
    }
};

#endif
