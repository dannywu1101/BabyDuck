#ifndef QUEUE_H
#define QUEUE_H

#include <queue>
#include <stdexcept>
#include <iostream>

using namespace std;

// Fila / Cola (FIFO: el primero en entrar es el primero en salir).
// Usa queue de la libreria estandar por debajo.
template <typename T>
class Queue {
private:
    queue<T> data;

public:
    // Agrega un elemento al final
    void enqueue(const T& value) { data.push(value); }

    // Quita y regresa el elemento del frente
    T dequeue() {
        if (data.empty()) throw out_of_range("dequeue() en fila vacia");
        T value = data.front();
        data.pop();
        return value;
    }

    // Consulta el frente sin quitarlo
    T front() const {
        if (data.empty()) throw out_of_range("front() en fila vacia");
        return data.front();
    }

    // Consulta el final sin quitarlo
    T back() const {
        if (data.empty()) throw out_of_range("back() en fila vacia");
        return data.back();
    }

    bool isEmpty() const { return data.empty(); }
    size_t size() const { return data.size(); }

    // Vacia la fila
    void clear() { while (!data.empty()) data.pop(); }
};

#endif
