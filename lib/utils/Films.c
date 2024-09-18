#include "Films.h"
#include <string.h>


// Define the arrays declared in the header file
char* FILM_TYPES[] = {TEST_FILM};
int FILM_CAPACITIES[] = {36};

int isValidFilmType(const char* filmType) {
    for (int i = 0; i < sizeof(FILM_TYPES) / sizeof(FILM_TYPES[0]); ++i) {
        if (strcmp(FILM_TYPES[i], filmType) == 0) {
            return 1;
        }
    }
    return 0;
}

int getFilmIndex(const char* filmType) {
    for (int i = 0; i < sizeof(FILM_TYPES) / sizeof(FILM_TYPES[0]); ++i) {
        if (strcmp(FILM_TYPES[i], filmType) == 0) {
            return i;
        }
    }
    return -1;
}

int getFilmCapacity(int filmIndex) {
    if (filmIndex >= 0 && filmIndex < sizeof(FILM_CAPACITIES) / sizeof(FILM_CAPACITIES[0])) {
        return FILM_CAPACITIES[filmIndex];
    }
    return -1;
}

int getFilmCapacity(const char* filmType) {
    int index = getFilmIndex(filmType);
    if (index != -1) {
        return getFilmCapacity(index);
    }
    return -1;
}