/// @file
/// Implementacja parsera weścia dla drugiej części zadania o telefonach.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 27.05.2018

#include <assert.h>
#include <ctype.h> // for isspace
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "input_parser.h"
#include "util.h"

/// @brief Enumeracja opisująca typ pojedyńczego leksemu.
enum InputType {
  IN_PHONE_NUMBER = 1,       ///< Numer telefonu.
  IN_IDENTIFIER = 2,         ///< Indentyfikator bazy przekierowań.
  IN_OPERATOR_NEW = 4,       ///< Operator ustawienia nowej aktualnej bazy.
  IN_OPERATOR_DEL = 8,       ///< Operator usunięcia bazy.
  IN_OPERATOR_GET = 16,      ///< Operator '?' funckji Get i Reverse.
  IN_OPERATOR_REDIRECT = 32, ///< Operator dodawania przekierowań telefonów.
  IN_OPERATOR_NON_TRIV = 64  ///< Operator funkcji NonTrivialCount.
};

/// @brief Pojedyńczy leksem pojawiający się w wejściu.
/// Jego typ określa enumeracja @ref InputType, w przypadku numerów telefonu
/// oraz identifikatorów posiada też on zaalokowany text.
struct InputUnit {
  /// @brief Typ leksemu.
  /// Typ opisywany przez enumeracje @ref InputType.
  enum InputType type;

  /// @brief Zawartość leksemu, jeśli nie jest on operatorem.
  /// W przypadku, gdy @ref type nie jest typu @ref IN_PHONE_NUMBER lub @ref
  /// IN_IDENTIFIER, ma wartość @p NULL.
  char *value;
};

/// Index ostatniego wczytanego znaku przez parser, 0, gdy nie został wczytany
/// jeszcze żadnen znak lub EOF, gdby wczytano EOF.
static int currentCharacterIdx = 0;

/// @brief Wczytuje kolekny znak z wejścia.
/// Wczytuje pojedyńczy znak wejścia, zwiększając @ref currentCharacterIdx o
/// 1. Gdy wczytano EOF, nie zwiększa już @ref currentCharacterIdx.
/// @return Wczytany znak, (gdy napotkano EOF, zwraca EOF).
static unsigned char getNextCharacter() {
  int result = getchar();
  if (result != EOF)
    currentCharacterIdx++;

  return ((unsigned char)result);
}

/// @brief Cofa wczytanie ostatniego znaku przez parser.
/// Wywołuje ungetc by oddać aktualny znak zpowrotem do strumienia wejścia.
/// Zakłada powodzenie tej operacji. Gdy dostaje EOF, nie robi nic.
/// @param[in] c – znak, który ma być oddany do strumienia.
static void ungetPrevCharacter(const char c) {
  if (c == EOF)
    return;

  int return_value = ungetc(c, stdin);
  currentCharacterIdx--;

  // Hide the unused variable warning, when compiling without asserts.
  (void)return_value;
  assert(return_value != EOF); // We assume ungetc succeded.
}

/// @brief Sprawdza czy znak jest białym znakiem.
/// Sprawdza czy znak jest białym znakiem. Kożysta z funkcji @p isspace z @p
/// ctype.h
/// @param[in] c – znak do sprawdzenia.
/// @return 1, gdy @p c jest uznawane za biały znak, 0 w przeciwnym wypadku.
static inline int isWhitespace(const char c) {
  if (isspace(c))
    return 1;
  return 0;
}

/// @brief Sprawdza czy znak jest cyfrą.
/// Sprawdza czy znak @p c jest cyfrą (w ascii).
/// @param[in] c – znak do sprawdzenia.
/// @return 1, gdy @p c jest cyfrą, 0 w przeciwnym wypadku.
static inline int isNumeric(const char c) { return ('0' <= c && c <= '9'); }

/// @brief Sprawdza czy znak jest alfanumeryczny.
/// Sprawdza czy znak @p c jest cyfrą, lub literą z alfabetu angielskiego (dużą
/// lub małą).
/// @param[in] c – znak do sprawdzenia.
/// @return 1, gdy @p c jest alfanumeryczny, 0 w przeciwnym wypadku.
static inline int isAlphaNumeric(const char c) {
  return (isNumeric(c) || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
}

/// @brief Sprawdza czy znak może być 'cyfrą' numeru telefonu.
/// Sprawdza czy znak @p c jest cyfrą, ':', ';' (w ascii).
/// @param[in] c – znak do sprawdzenia.
/// @return 1, gdy @p c jest cyfrą, dwukropkiem lub średnikiem, 0 w przeciwnym
///         wypadku.
static inline int isPhoneNumberDigit(const char c) {
  return ('0' <= c && c <= ';');
}

/// @brief Zgłasza błąd syntaktyczny parsera.
/// Wypisuje informacje o błędzie na standardowy wyjście diagnostyczne w
/// formacjie opisanym w treści drugiej częsci zadania.
/// @param[in] characterIdx – Znak na, którym zdarzył się bład. (Może być EOF).
static inline void printSyntaxError(const int characterIdx) {
  if (characterIdx == EOF)
    fprintf(stderr, "ERROR EOF\n");
  else
    fprintf(stderr, "ERROR %d\n", characterIdx);
}

/// @brief Zwraca liczbę znaków leksemu.
/// Dla każdego operatora rozmiar jest stały, dla identyfikatorów i numerów
/// wywoływana jest funckja strlen na atrybucie przekazanej struktury.
/// @param[in] inunit – leksem, dla którego należy policzyć długość.
/// @return Liczbę znaków przekazanego leksemu.
static int inputUnitGetSize(struct InputUnit *inunit) {
  switch (inunit->type) {
  case IN_PHONE_NUMBER:
  case IN_IDENTIFIER:
    return strlen(inunit->value);

  case IN_OPERATOR_NEW:
  case IN_OPERATOR_DEL:
    return 3;

  case IN_OPERATOR_GET:
  case IN_OPERATOR_REDIRECT:
  case IN_OPERATOR_NON_TRIV:
    return 1;

  default:
    assert(!"Unrecognized unit type!");
    return 0;
  }
}

/// @brief Parsuje następny leskem z weścia.
/// Pomija białe znaki i komentarze i parsuje następny leksem ze standardowego
/// wejścia. Alokuje pamięć tylko, gdy zwrócone zostanie @p IF_OK. Gdy zwrócony
/// jest IF_ERROR, funkcja wypisała już informacje o błędzie na standardowe
/// wyjście.
/// @param[out] out_result – wskaźnik na strukturę przechowująca wynikowy
///                          leksem.
/// @param[out] first_character_idx – wskaźnik na indeks pierwszej litery
///                                   wynikowego leksemu.
/// @return Jedną z wartości enumeracji @p InputFeedback. IN_OK, gdy udało się
///         wczytać leksem, IF_ERROR, gdy napotkano błąd składniowy, IF_EOF gdy
///         zamiast leksemu napotkano EOF.
static enum InputFeedback inputGetNextUnit(struct InputUnit *out_result,
                                           int *first_character_idx) {
  char c;
  do {
    c = getNextCharacter();
  } while (isWhitespace(c));

  switch (c) {
  // No more reading from input. It does NOT have to be an error, but it
  // can. We just pass information about EOF into previous function.
  case EOF: {
    return IF_EOF;
  }

  // $ exists only as a comment. So next sign must also be $. Otherwise
  // its an error.
  case '$': {
    char next = getNextCharacter();
    if (next != '$') {
      // a character after a $ -> $ cannot be interpreted.
      printSyntaxError(next == EOF ? currentCharacterIdx
                                   : currentCharacterIdx - 1);
      return IF_ERROR;
    }
    // Skip the comment.
    char current = getNextCharacter();
    char prev;
    do {
      // EOF when we are inside comment gives always: ERROR EOF
      if (current == EOF) {
        printSyntaxError(EOF);
        return IF_ERROR;
      }

      prev = current;
      current = getNextCharacter();
    } while (!(prev == '$' && current == '$'));

    // Now after we skip a comment, we call the same function once more.
    return inputGetNextUnit(out_result, first_character_idx);
  }

  case '?': {
    out_result->type = IN_OPERATOR_GET;
    out_result->value = NULL;
    break;
  }

  case '@': {
    out_result->type = IN_OPERATOR_NON_TRIV;
    out_result->value = NULL;
    break;
  }

  case '>': {
    out_result->type = IN_OPERATOR_REDIRECT;
    out_result->value = NULL;
    break;
  }

  default: {
    // If true there is a beginning of either phone number, or identifier.
    if (isAlphaNumeric(c) || isPhoneNumberDigit(c)) {
      // It it is 0, we check for alhpa numeric values.
      int parse_phone_number;

      // We search for a phone number.
      if (isPhoneNumberDigit(c))
        parse_phone_number = 1;
      // We parse an identifier.
      else {
        assert(inRange(c, 'a', 'z') || inRange(c, 'A', 'Z'));
        parse_phone_number = 0;
      }

      int buffer_capacity = 32;
      int buffer_idx = 1;
      char *buffer = malloc(sizeof(char) * buffer_capacity);
      buffer[0] = c;
      buffer[1] = '\0';

      char next = getNextCharacter();

      while (parse_phone_number ? isPhoneNumberDigit(next)
                                : isAlphaNumeric(next)) {
        if (buffer_idx >= buffer_capacity - 1) {
          buffer_capacity *= 2;

          char *newBuffer = realloc(buffer, sizeof(char) * buffer_capacity);
          if (newBuffer == NULL) {
            // In case that realloc fails, the old memory is still valid, so we
            // have to free 'it!
            free(buffer);

            // If memory error has occured, error is returned with the character
            // index, that caused buffer overflow.
            printSyntaxError(currentCharacterIdx);
            return IF_ERROR;
          } else
            buffer = newBuffer;

          assert(buffer_idx < buffer_capacity);
        }

        buffer[buffer_idx++] = next;
        buffer[buffer_idx] = '\0';

        next = getNextCharacter();
      }

      // Push the non-matching character back to the stream.
      if (next != EOF)
        ungetPrevCharacter(next);

      if (strcmp("NEW", buffer) == 0) {
        free(buffer);
        out_result->type = IN_OPERATOR_NEW;
        out_result->value = NULL;
      } else if (strcmp("DEL", buffer) == 0) {
        free(buffer);
        out_result->type = IN_OPERATOR_DEL;
        out_result->value = NULL;
      } else {
        out_result->type = parse_phone_number ? IN_PHONE_NUMBER : IN_IDENTIFIER;
        out_result->value = buffer;
      }
    }

    else {
      // Non alhpa numeric character that cannot be interpreted with this
      // context.
      printSyntaxError(currentCharacterIdx);
      return IF_ERROR;
    }
  }
  }

  assert(out_result);
  (*first_character_idx) =
      currentCharacterIdx + 1 - inputUnitGetSize(out_result);
  return IF_OK;
}

/// @brief Wczytuje leksem określonego typu.
/// Próbuje wczytać leksem określonego typu; gdy nie ma błędu składniowego, ale
/// typ wczytanego leksemu nie należy do zbioru oczekiwanych zkłasza bład i
/// zwraca IF_ERROR.
/// @param[out] out_res – wskaźnik na strukturę przechowująca wynikowy leksem.
/// @param[out] out_first_character_idx – wskaźnik na indeks pierwszej litery
///                                       wynikowego leksemu.
/// @param[in] expected_type – zbiór akceptowalnych typów wynikowych leksemów, w
///                            postaci maski bitowej wartości enumeracji @ref
///                            InputType.
/// @param[in] handle_eof_as_error – gdy nie 0, brak wczytanej struktury z
///                                  powodu końca pliku traktowany jest jak
///                                  bład, wypisany jest stosowny komunikat, a
///                                  funckja zwraca IF_ERORR zamiat IF_EOF.
static enum InputFeedback inputReadUnitWithType(struct InputUnit *out_res,
                                                int *out_first_character_idx,
                                                enum InputType expected_type,
                                                int handle_eof_as_error) {
  struct InputUnit current_unit = {0, NULL};
  int current_unit_input_idx = 0;
  enum InputFeedback feedback =
      inputGetNextUnit(&current_unit, &current_unit_input_idx);

  if (feedback == IF_ERROR)
    return feedback;

  // This function assumes that there is a unit. If it is not, ERROR EOF is
  // reported.
  if (feedback == IF_EOF) {
    if (handle_eof_as_error) {
      printSyntaxError(EOF);
      return IF_ERROR;
    } else
      return IF_EOF;
  }

  assert(feedback == IF_OK);

  if (current_unit.type & expected_type) {
    // Fill the out data:
    (*out_res) = current_unit;
    (*out_first_character_idx) = current_unit_input_idx;

    return IF_OK;
  } else {
    // Free current_unit content, which was allocated.
    if (current_unit.value)
      free(current_unit.value);

    // Error because unit of this type was not expected in this context.
    printSyntaxError(current_unit_input_idx);
    return IF_ERROR;
  }
}

/// @brief Wypisuje błąd użycia operatora.
/// Wypisuje na standardowy wyjście diagnostyczne informacje o błędzie użycia
/// operatora w formacje zgodnym z treścią zadania..
/// @param[in] op – Operacja, której wywołanie zakończyło się błędem.
void printOperationError(const struct Operation *op) {
  char *operator_name = "";

  switch (op->performed_operation) {
  case OT_ADD:
    operator_name = "NEW";
    break;

  case OT_DEL_PHONE_NUM:
  case OT_DEL_DATABASE:
    operator_name = "DEL";
    break;

  case OT_REDIRECT:
    operator_name = ">";
    break;

  case OT_GET:
  case OT_REVERSE:
    operator_name = "?";
    break;

  case OT_NON_TRIV:
    operator_name = "@";
    break;

  // NOTE: Should not reach.
  default:
    assert(!"Unexpected operation type.");
    break;
  }

  fprintf(stderr, "ERROR %s %d\n", operator_name, op->operator_idx);
}

/// @brief Pomocnicze makro wykorzystywane w @ref inputParseNextOperation.
/// Wczytuje ono następny leksem na element o numerze IDX w tablicy leksemów,
/// 'zwraca' 1, gdy operacja się udała, 0 w przeciwnym wypadku.
#define LOAD_UNIT_WITH_TYPE(IDX, IN_TYPE, EOF_AS_ERROR)                        \
  (((current_feedback[(IDX)] = inputReadUnitWithType(                          \
         &current_unit[(IDX)], &current_unit_input_idx[(IDX)], IN_TYPE,        \
         EOF_AS_ERROR)) == IF_OK)                                              \
       ? 1                                                                     \
       : 0)

/// @brief Pomocnicze makro wykorzystywane w @ref inputParseNextOperation.
/// Czyści tablice leksemów do elementu o numerze IDX i zwraca wynik wczytania
/// ostatniego z nich.
#define CLEAR_AND_RETURN_LAST_FEEDBACK(IDX)                                    \
  do {                                                                         \
    for (int _i = 0; _i <= (IDX); ++_i)                                        \
      if (current_unit[_i].value) {                                            \
        free(current_unit[_i].value);                                          \
        current_unit[_i].value = NULL;                                         \
      }                                                                        \
    return current_feedback[(IDX)];                                            \
  } while (0)

enum InputFeedback inputParseNextOperation(struct Operation *out_result) {
  // NOTE: Possible scenarios:
  //   NEW identifier
  //   DEL identifier
  //   DEL number
  //   number > number
  //   number ?
  //   ? number
  //   @ number

  const int MAX_UNITS_IN_STATEMENT = 3;
  struct InputUnit current_unit[MAX_UNITS_IN_STATEMENT];
  int current_unit_input_idx[MAX_UNITS_IN_STATEMENT];
  enum InputFeedback current_feedback[MAX_UNITS_IN_STATEMENT];
  for (int i = 0; i < 3; ++i)
    current_unit[i] = (struct InputUnit){0};

  if (LOAD_UNIT_WITH_TYPE(0,
                          IN_OPERATOR_NEW | IN_OPERATOR_DEL | IN_PHONE_NUMBER |
                              IN_OPERATOR_GET | IN_OPERATOR_NON_TRIV,
                          0)) {
    switch (current_unit[0].type) {
    case IN_OPERATOR_NEW: {
      if (LOAD_UNIT_WITH_TYPE(1, IN_IDENTIFIER, 1)) {
        (*out_result) =
            (struct Operation){.args[0] = duplicateStr(current_unit[1].value),
                               .args[1] = NULL,
                               .performed_operation = OT_ADD,
                               .operator_idx = current_unit_input_idx[0]};
      }

      CLEAR_AND_RETURN_LAST_FEEDBACK(1);
    }

    case IN_OPERATOR_DEL: {
      if (LOAD_UNIT_WITH_TYPE(1, IN_IDENTIFIER | IN_PHONE_NUMBER, 1)) {
        (*out_result) = (struct Operation){
            .args[0] = duplicateStr(current_unit[1].value),
            .args[1] = NULL,
            .performed_operation = current_unit[1].type == IN_IDENTIFIER
                                       ? OT_DEL_DATABASE
                                       : OT_DEL_PHONE_NUM,
            .operator_idx = current_unit_input_idx[0]};
      }

      CLEAR_AND_RETURN_LAST_FEEDBACK(1);
    }

    case IN_PHONE_NUMBER: {
      if (LOAD_UNIT_WITH_TYPE(1, IN_OPERATOR_GET | IN_OPERATOR_REDIRECT, 1)) {
        if (current_unit[1].type == IN_OPERATOR_GET) {
          (*out_result) =
              (struct Operation){.args[0] = duplicateStr(current_unit[0].value),
                                 .args[1] = NULL,
                                 .performed_operation = OT_GET,
                                 .operator_idx = current_unit_input_idx[1]};
        } else {
          assert(current_unit[1].type == IN_OPERATOR_REDIRECT);
          if (LOAD_UNIT_WITH_TYPE(2, IN_PHONE_NUMBER, 1)) {
            (*out_result) = (struct Operation){
                .args[0] = duplicateStr(current_unit[0].value),
                .args[1] = duplicateStr(current_unit[2].value),
                .performed_operation = OT_REDIRECT,
                .operator_idx = current_unit_input_idx[1]};
          }

          CLEAR_AND_RETURN_LAST_FEEDBACK(2);
        }

        CLEAR_AND_RETURN_LAST_FEEDBACK(1);
      }
      CLEAR_AND_RETURN_LAST_FEEDBACK(1);
    }

    case IN_OPERATOR_GET: {
      if (LOAD_UNIT_WITH_TYPE(1, IN_PHONE_NUMBER, 1)) {
        (*out_result) =
            (struct Operation){.args[0] = duplicateStr(current_unit[1].value),
                               .args[1] = NULL,
                               .performed_operation = OT_REVERSE,
                               .operator_idx = current_unit_input_idx[0]};
      }

      CLEAR_AND_RETURN_LAST_FEEDBACK(1);
    }

    case IN_OPERATOR_NON_TRIV: {
      if (LOAD_UNIT_WITH_TYPE(1, IN_PHONE_NUMBER, 1)) {
        (*out_result) =
            (struct Operation){.args[0] = duplicateStr(current_unit[1].value),
                               .args[1] = NULL,
                               .performed_operation = OT_NON_TRIV,
                               .operator_idx = current_unit_input_idx[0]};
      }

      CLEAR_AND_RETURN_LAST_FEEDBACK(1);
    }

    // NOTE: Should not reach.
    default:
      assert(!"Unexpected input type.");
      return IF_ERROR;
    }
  } else {
    CLEAR_AND_RETURN_LAST_FEEDBACK(0);
  }
}
