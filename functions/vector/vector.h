#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <vector>
#include <algorithm>


template <typename T>
void addUnique(std::vector<T>& vec, const T& element) {
    // Cherche l'élément dans le vecteur
    auto it = std::find(vec.begin(), vec.end(), element);

    // Si l'élément est trouvé, le retire
    if (it != vec.end()) {
        return;
    }
    // Ajoute l'élément à la fin du vecteur
    vec.push_back(element);
};

#endif
