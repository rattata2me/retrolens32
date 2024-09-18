#ifndef RETROLENS_FILMS_H
#define RETROLENS_FILMS_H

/**
 * @brief A test film type.
 */
#define TEST_FILM "test_film"

/**
 * @brief Array of available film types.
 */
extern char* FILM_TYPES[];

/**
 * @brief Array of film capacities corresponding to each film type.
 */
extern int FILM_CAPACITIES[];

/**
 * @brief Check if the given film type is valid.
 * 
 * @param filmType The film type to check.
 * @return true if the film type is valid, false otherwise.
 */
bool isValidFilmType(const char* filmType);

/**
 * @brief Get the index of the given film type.
 * 
 * @param filmType The film type to find.
 * @return The index of the film type, or -1 if not found.
 */
int getFilmIndex(const char* filmType);

/**
 * @brief Get the capacity of the film at the given index.
 * 
 * @param filmIndex The index of the film.
 * @return The capacity of the film, or -1 if the index is invalid.
 */
int getFilmCapacity(int filmIndex);

/**
 * @brief Get the capacity of the given film type.
 * 
 * @param filmType The film type to find the capacity for.
 * @return The capacity of the film, or -1 if the film type is invalid.
 */
int getFilmCapacity(const char* filmType);

#endif // RETROLENS_FILMS_H