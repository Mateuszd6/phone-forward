/// @file
/// Interfejs klasy przechowującej przekierowania numerów telefonicznych
///
/// @author Marcin Peczarski <marpe@mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 09.04.2018

#ifndef __PHONE_FORWARD_H__
#define __PHONE_FORWARD_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

struct PhoneForward;

struct PhoneNumbers;

/// @brief Tworzy nową strukturę.
/// Tworzy nową strukturę niezawierającą żadnych przekierowań.
/// @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
///         zaalokować pamięci.
struct PhoneForward *phfwdNew(void);

/// @brief Usuwa strukturę.
/// Usuwa strukturę wskazywaną przez @p pf. Nic nie robi, jeśli wskaźnik ten ma
/// wartość NULL.
/// @param[in,out] pf – wskaźnik na usuwaną strukturę.
void phfwdDelete(struct PhoneForward *pf);

/// @brief Dodaje przekierowanie.
/// Dodaje przekierowanie wszystkich numerów mających prefiks @p num1, na
/// numery, w których ten prefiks zamieniono odpowiednio na prefiks @p num2.
/// Każdy numer jest swoim własnym prefiksem. Jeśli wcześniej zostało dodane
/// przekierowanie z takim samym parametrem @p num1, to jest ono zastępowane.
/// @param[in,out] pf – wskaźnik na strukturę przechowującą przekierowania
///                     numerów;
/// @param[in] num1 – wskaźnik na napis reprezentujący prefiks numerów
///                   przekierowywanych;
/// @param[in] num2 – wskaźnik na napis reprezentujący prefiks numerów, na które
///                   jest wykonywane przekierowanie.
/// @return Wartość @p true, jeśli przekierowanie zostało dodane.
///         Wartość @p false, jeśli wystąpił błąd, np. podany napis nie
///         reprezentuje numeru, oba podane numery są identyczne lub nie udało
///         się zaalokować pamięci.
bool phfwdAdd(struct PhoneForward *pf, const char *num1, const char *num2);

/// @brief Usuwa przekierowania.
/// Usuwa wszystkie przekierowania, w których parametr @p num jest prefiksem
/// parametru @p num1 użytego przy dodawaniu. Jeśli nie ma takich przekierowań
/// lub napis nie reprezentuje numeru, nic nie robi.
///
/// @param[in,out] pf – wskaźnik na strukturę przechowującą przekierowania
///                     numerów;
/// @param[in] num – wskaźnik na napis reprezentujący prefiks numerów.
void phfwdRemove(struct PhoneForward *pf, const char *num);

/// @brief Wyznacza przekierowanie numeru.
/// Wyznacza przekierowanie podanego numeru. Szuka najdłuższego pasującego
/// prefiksu. Wynikiem jest co najwyżej jeden numer. Jeśli dany numer nie został
/// przekierowany, to wynikiem jest ten numer. Jeśli podany napis nie
/// reprezentuje numeru, wynikiem jest pusty ciąg. Alokuje strukturę
/// @p PhoneNumbers, która musi być zwolniona za pomocą funkcji @ref
/// phnumDelete.
/// @param[in,out] pf – wskaźnik na strukturę przechowującą przekierowania
///                     numerów;
/// @param[in] num – wskaźnik na napis reprezentujący numer.
/// @return Wskaźnik na strukturę przechowującą ciąg numerów lub NULL, gdy nie
///         udało się zaalokować pamięci.
const struct PhoneNumbers *phfwdGet(struct PhoneForward *pf, const char *num);

/// @brief Wyznacza przekierowania na dany numer.
/// Wyznacza wszystkie przekierowania na podany numer. Wynikowy ciąg zawiera też
/// dany numer. Wynikowe numery są posortowane leksykograficznie i nie mogą się
/// powtarzać. Jeśli podany napis nie reprezentuje numeru, wynikiem jest pusty
/// ciąg. Alokuje strukturę @p PhoneNumbers, która musi być zwolniona za pomocą
/// funkcji @ref phnumDelete.
/// @param[in,out] pf – wskaźnik na strukturę przechowującą przekierowania
///                     numerów;
/// @param[in] num – wskaźnik na napis reprezentujący numer.
/// @return Wskaźnik na strukturę przechowującą ciąg numerów lub NULL, gdy nie
///         udało się zaalokować pamięci.
const struct PhoneNumbers *phfwdReverse(struct PhoneForward *pf,
                                        const char *num);

/// @brief Oblicza liczbę nietrywialnych numerów danej długości o cyfrach z
/// danego zbioru.
/// Oblicza liczbę nietrywialnych numerów długości len zawierających tylko
/// cyfry, które znajdują się w napisie @p set. Obliczenia dokonywane są modulo
/// dwa do potęgi liczba bitów reprezentacji typu @p size_t.
/// @param[in,out] pf – wskaźnik na strukturę przechowującą ciąg numerów;
/// @param[in] set – zbiór cyfr jakie są dopuszczalne w zbiorze wynikowym. Może
///                  zawierać dowolne znaki.
/// @param[in] len – długość numerów z szukanego zbioru.
/// @return Jeśli wskaźnik @p pf ma wartość @p NULL, @p set ma wartość @p NULL,
///         @p set jest pusty, @p set nie zawiera żadnej cyfry lub parametr @p
///         len jest równy zeru, wynikiem jest zero, w przciwnym razie jest to
///         liczba nietrywialnych numerów długości @p len zawierających tylko
///         cyfry z @p set.
size_t phfwdNonTrivialCount(struct PhoneForward *pf, const char *set,
                            size_t len);

/// @brief Usuwa strukturę.
/// Usuwa strukturę wskazywaną przez @p pnum. Nic nie robi, jeśli wskaźnik ten
/// ma wartość NULL.
/// @param[in] pnum – wskaźnik na usuwaną strukturę.
void phnumDelete(const struct PhoneNumbers *pnum);

/// @brief Udostępnia numer.
/// Udostępnia wskaźnik na napis reprezentujący numer. Napisy są indeksowane
/// kolejno od zera.
/// @param[in] pnum – wskaźnik na strukturę przechowującą ciąg napisów;
/// @param[in] idx – indeks napisu.
/// @return Wskaźnik na napis. Wartość NULL, jeśli wskaźnik @p pnum ma wartość
///         NULL lub indeks ma za dużą wartość.
const char *phnumGet(const struct PhoneNumbers *pnum, size_t idx);

#endif /* __PHONE_FORWARD_H__ */
