/// @file
/// Interfejs udostępniający podstawowe własności parsera.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 27.05.2018

#ifndef __INPUT_PARSER_H__
#define __INPUT_PARSER_H__

/// Typ pojedynczej operacji udostępnianej przez program.
enum OperationType {
  OT_ADD,           ///< Dodanie nowej bazy przekierowań.
  OT_DEL_PHONE_NUM, ///< Usunięcie przekierowania z aktualnej bazu.
  OT_DEL_DATABASE,  ///< Usunięcie bazy przekierowań.
  OT_REDIRECT,      ///< Dodanie przekierowania.
  OT_GET,           ///< Wypisanie przekierowania z numeru.
  OT_REVERSE,       ///< Wypisanie wszystkich przekierowań na numer.
  OT_NON_TRIV       ///< Policzenie nietrywialnych numerów o znakach danego numeru.
};

/// Informacja zwrotna funckji parsujących wejście. Gdy funckja zwraca IF_ERROR
/// oznacza to, że została już wypisana przez nią stosowna informacja na wyjście
/// diagnostyczne.
enum InputFeedback {
  IF_EOF,  ///< Parer napotkał koniec lini.
  IF_OK,   ///< Wczytywanie zakończone powodzeniem.
  IF_ERROR ///< Wczytywanie zakończone błędem.
};

/// @brief Pojedyńcza operacja.
/// Struktura pojedynczej operacji jaką udostępnia program.
struct Operation {
  /// Argumenty operacji.
  char *args[2];

  /// Typ operacji. Jedna wartość z enumeracji @p OperationType
  enum OperationType performed_operation;

  /// Indeks pierwszego znaku operatora operacji w standardowym wejściu.
  int operator_idx;
};

/// @brief Wypisuje błąd użycia operatora.
/// Wypisuje na standardowy wyjście diagnostyczne informacje o błędzie użycia
/// operatora w formacje zgodnym z treścią zadania..
/// @param[in] op – Operacja, której wywołanie zakończyło się błędem.
void printOperationError(const struct Operation *op);

/// @brief Wczytuje jedną operacje.
/// Wczytuje następną operacje do wykonania ze standardowego wejścia.
/// @param[out] out_result – Wskaźnik na strukturę operacji, na którą ma zostać
///                          zapisany wynik, gdy wczytywanie było udane.
/// @return IF_OK, gdy operacja się udała, IF_EOF gdy zamiast operacji napotkano
///         EOF, IF_ERROR, gdy wystąpił błąd parsera. (Zotał on wtedy zgłoszony
///         przez funkcje które wywołuje ta procedura ).
enum InputFeedback inputParseNextOperation(struct Operation *out_result);

#endif /* __INPUT_PARSER_H__ */
