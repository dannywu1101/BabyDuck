#ifndef STACK_H
#define STACK_H

#include <stack>
#include <stdexcept>
#include <iostream>

using namespace std;

// Pila (LIFO: el ultimo en entrar es el primero en salir).
// Usa stack de la libreria estandar por debajo.
template <typename T>
class Stack {
private:
    stack<T> data;

public:
    // Agrega un elemento en el tope
    void push(const T& value) { data.push(value); }

    // Quita y regresa el elemento del tope
    T pop() {
        if (data.empty()) throw out_of_range("pop() en pila vacia");
        T value = data.top();
        data.pop();
        return value;
    }

    // Regresa el elemento del tope sin quitarlo
    T peek() const {
        if (data.empty()) throw out_of_range("peek() en pila vacia");
        return data.top();
    }

    bool isEmpty() const { return data.empty(); }
    size_t size() const { return data.size(); }

    // Vacia la pila
    void clear() { while (!data.empty()) data.pop(); }
};

#endif
