/// @file
/// Interfejs udostępniający operacje na kolekcji baz przekierowań.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 27.05.2018

#ifndef __REDIRECTIONS_DB_H__
#define __REDIRECTIONS_DB_H__

/// @brief Struktura pojedynczej bazy przekierowań.
/// Struktura pojedynczej bazy przekierowań. Przechowuje strukturę PhoneForward
/// posiadającą informacje o przekierowaniach oraz unikalną nazwę.
struct RedirectionsDatabase {
  /// Unikalna nazwa struktury.
  char *name;

  /// Struktura przechowująca przekierowania telefonów.
  struct PhoneForward *phfwd;
};

/// @brief Obecnie używana baza przekierowań.
/// Wskaźnik na aktualnie wybraną bazę przekierowań, lub @p NULL, gdy takowej
/// nie ma.
extern struct RedirectionsDatabase *current_database;

/// @brief Ustawia bazę danych jako aktualną.
/// Ustawia bazę danych o nazwie @p name na aktualną. Jeśli nie istnieje baza o
/// nazwie @p name, to zostaje ona stworzona.
/// @param[in] name – Nazwa ustawianej bazy danych.
/// @return 1, gdy operacja się powiodła, 0 gdy wystąpił błąd wykonania.
int setOrCreateDatabaseWithName(const char *name);

/// @brief Usuwa bazę przekierowań.
/// Usuwa bazę przekierowań o nazwie @p name. Jeśli jest ona aktualna, wartość
/// @ref current_database zostaje zmieniona na @p NULL i co za tym idzie nie ma
/// aktualnej bazy danych do ustawienia nowej.
/// @param[in] name – Nazwa usuwanej struktury.
/// @return 1, gdy operacja się powiodła, 0 gdy wystąpił błąd wykonania.
int deleteDatabaseWithName(const char *name);

/// @brief Usuwa wszystkie bazy przekierowań.
/// Usuwa całą kolekcję danych baz przekierowań, nie ma po tej operacji
/// aktualnej bazy.
void clearAllRedirectionsDatabase();

#endif /* __REDIRECTIONS_DB_H__ */
