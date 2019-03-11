/// @file
/// @brief Implementacja niewielkiego zbioru funckji pomocniczych.
/// Implementacja niewielkiego zbioru funckji pomocniczych, głównie static
/// inline, nie nadająca się na oddzielną jednostkę kompilacji, gdyż chcemy żeby
/// kompliator miał możliwość inline'owania tych funckji.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 29.05.2018

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/// @brief Zwraca kopię otrzymanego napisu.
/// Alokuje pamięć do przechowywania napisu @p str_to_duplicate i zwraca jego
/// kopię. Ma dokładnie taką samą funkcjonalność jak @p strdup, ale choć na
/// studentsie @p strdup działa, na kilku niezależnych maszynach stanowi to
/// problem, więc żeby zmiejszyć liczę zależności zaimplementowałem tą fukcję.
/// Wywołujący fukcję musi zadbać o to, by zaalokowana pamięć została zwolniona.
/// @param[in] str_to_duplicate – napis, jaki ma zostać zduplikowany.
/// @return Wskaźnik na kopię napisu, lub @p NULL, gdy nie udało się zaalokować
///         pamięci.
static inline char *duplicateStr(const char *str_to_duplicate) {
  char *result = malloc(sizeof(char) * (strlen(str_to_duplicate) + 1));
  if (result)
    strcpy(result, str_to_duplicate);

  return result;
}

/// @brief podnosi @p value do odpowiedniej potęgi.
/// Wykonuje szybkie potęgowanie modularne, licząc wynik modulo dwa do potęgi
/// liczba bitów typu size_t.
/// @param[in] value – wartość jaką należy podnieść do potęgi.
/// @param[in] exp – wykładnik.
/// @return @p value podniesione do potęgi @p exp modulo dwa do potęgi liczba
///         bitów typu size_t. W przypadku 0^0 zwraca 1.
static inline size_t power(size_t value, size_t exp) {
  size_t result = 1;
  while (exp) {
    if (exp & 1)
      result *= value;

    value *= value;
    exp >>= 1;
  }

  return result;
}

/// @brief Sprawdza czy @p value jest w zadanym zakresie.
/// Sprawdza czy wartość @p value jest w zakresie [@p min; @p max].
/// @param[in] value – Wartość do sprawdzenia.
/// @param[in] min   – Dolna wartość przedziału. Musi być <= @p max.
/// @param[in] max   – Górna wartość przedziału. Musi być >= @p min.
/// @return @p true jeśli @p value znajduje się z zadanym zakresie, @p false w
///            przeciwnym przypadku.
static inline bool inRange(const int value, const int min, const int max) {
  assert(min <= max);
  return (min <= value && value <= max);
}

#endif /* UTIL_H__ */
