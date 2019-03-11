/// @file
/// Implementacja interfejsu kalsy przechowującej przekierowania numerów
/// telefonicznych
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 06.05.2018

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phone_forward.h"
#include "trie.h"
#include "util.h"

/// @brief Struktura przechowująca przekierowania numerów telefonów.
/// Struktura przechowująca przekierowania numerów telefonów (w postaci drzewa
/// Trie), i wszystkie prefiksy na które są przekierowania (w postaci drugiego
/// Trie). Taka reprezentacja pozwala na szybkie wykonywane zapytań zarówno o
/// prefiksy jak i o przekierowania.
struct PhoneForward {
  /// @brief Drzewo Trie zawierające przekierowania.
  /// Drzewo Trie które zawiera wszystkie przekierowania, a jako wartości
  /// number na, który przekierowanie następuje. Niezmiennik: Każdy wierzchołek
  /// w tym drzewie zawiera co najwyżej jendną wartość.
  struct TrieNode *redirections;

  /// @brief Drzewo Trie zawierające numery, na które są przekierowania.
  /// Drzewo Trie zawierające prefiksy, na które są przekierowania. Jako
  /// wartości trzymane są numery, które sa przekierowywane.
  struct TrieNode *prefixes;
};

/// @brief Struktura przechowująca ciąg numerów telefonów.
/// Struktura przechowująca, posortowane leksykograficznie, bez powtórzeń,
/// numery telefonów. Posiada @ref size numerów, do których dostęp odbywa się za
/// pomocą funckji @ref phfwdGet.
struct PhoneNumbers {
  /// Ilość numerów w danej strukturze.
  size_t size;

  /// @brief Maksymalna ilość numerów jakie można dodać.
  /// Używany podczas tworzania struktury, gdy nie wiadomo ile numerów
  /// znajdzie się w strukturze.
  size_t capacity;

  /// Tablica wskaźników na numery telefonów, posortowana leksykograficznie.
  char **numbers;
};

/// @brief Tworzy nową strukturę.
/// Tworzy nową strukturę PhoneNumbers, posiadającą miejsce na @p initCapacity
/// numerów telefonów. Alokuje pamięć, która musi być zwolniona używając @ref
/// phnumDelete.  Nie jest to cześć interfejsu modułu, ponieważ strukturę
/// PhoneNumbers można uzyskać jedynie przez @ref phnumGet oraz @ref
/// phfwdReverse.
/// @param[in] initCapacity – startowa wartość @ref PhoneNumbers.capacity. Musi
///                           być wieksza od 0.
/// @return Wskaźnik na zaalokowaną strukturę, lub @p NULL, gdy nie udało się
///         zaalokować pamięci.
static inline struct PhoneNumbers *phnumNewEmpty(int initCapacity) {
  struct PhoneNumbers *result = malloc(sizeof(struct PhoneNumbers));
  if (result) {
    (*result) = (struct PhoneNumbers){0, initCapacity,
                                      malloc(sizeof(char *) * initCapacity)};

    if (!result->numbers) {
      free(result);
      return NULL;
    }
  }

  return result;
}

/// @brief Sprawdza czy napis jest numerem telefonu.
/// Sprawdza czy napis @p str jest spełniającym warunki zadania (w chwili
/// obecnej oznacza to, że jest niepusty i składa się ze znaków od '0' do '9').
/// @param[in] str – napis, który chcemy sprawdzić.
/// @return @p true jeśli @p str jest numerem telefonu spełniającym warunki
///         zadania, @p false w przeciwnym wypadku.
static bool isValidPhnum(const char *str) {
  if (!str || str[0] == '\0')
    return false;

  for (unsigned int i = 0; str[i] != '\0'; ++i)
    if (!inRange(str[i], '0', '9') && str[i] != ';' && str[i] != ':')
      return false;

  return true;
}

struct PhoneForward *phfwdNew(void) {
  struct PhoneForward *result = malloc(sizeof(struct PhoneForward));
  if (result) {
    // Initialize both trie trees.
    result->redirections = trieNodeNew(NULL);
    result->prefixes = trieNodeNew(NULL);
    if (!result->redirections || !result->prefixes) {
      if (result->prefixes)
        trieDeleteSubtree(result->prefixes, result->prefixes);
      if (result->redirections)
        trieDeleteSubtree(result->redirections, result->redirections);

      return NULL;
    }
    return result;
  }
  return NULL;
}

void phfwdDelete(struct PhoneForward *pf) {
  if (pf) {
    trieDeleteSubtree(pf->prefixes, pf->prefixes);
    trieDeleteSubtree(pf->redirections, pf->redirections);
    free(pf);
  }
}

bool phfwdAdd(struct PhoneForward *pf, const char *num1, const char *num2) {
  assert(pf);

  // If num1/2 are not telepfone numbers or if they are equal return false.
  if (!isValidPhnum(num1) || !isValidPhnum(num2) || strcmp(num1, num2) == 0) {
    return false;
  }

  struct DataNode *dataToAdd = dataNodeNew(num2);
  if (!dataToAdd)
    return false;

  // There is no reason to initialize this, except the GCC warning.
  struct DataNode *prevData = NULL;

  if (!trieAddText(pf->redirections, num1, dataToAdd, false, &prevData))
    return false;

  if (prevData) {
    assert(!prevData->next);
    trieRemoveOneEntry(pf->prefixes, prevData->text, num1);
    dataNodeDelete(prevData);
  }

  // Init dataToAdd again, since we now insert to the second tree.
  dataToAdd = dataNodeNew(num1);
  if (!dataToAdd)
    return false;

  trieAddText(pf->prefixes, num2, dataToAdd, true, NULL);

  return true;
}

void phfwdRemove(struct PhoneForward *pf, const char *num) {
  if (!isValidPhnum(num))
    return;

  struct TrieNode *currentNode = pf->redirections;

  for (int i = 0; num[i] != '\0'; ++i) {
    int currentBranchIdx = num[i] - '0';
    assert(inRange(currentBranchIdx, 0, ALPHABET_SIZE - 1));

    if (!currentNode->childs[currentBranchIdx])
      return;

    assert(currentNode->nonNullChilds > 0);
    currentNode = currentNode->childs[currentBranchIdx];
  }
  assert(currentNode->nonNullChilds >= 0);

  trieDeleteSubtree(pf->redirections, currentNode);
}

struct PhoneNumbers const *phfwdGet(struct PhoneForward *pf, const char *num) {
  struct PhoneNumbers *result = phnumNewEmpty(1);
  if (!result)
    return NULL;

  // Result is empty, if [num] does not represent the number.
  if (!isValidPhnum(num))
    return result;

  struct TrieNode *currentNode = pf->redirections;
  struct TrieNode *last_forwarded_node = NULL;
  int last_forwarded_prefix_size = 0;

  if (currentNode->data != NULL) {
    last_forwarded_node = currentNode;
    last_forwarded_prefix_size = 0;
  }

  for (int i = 0; num[i] != '\0'; ++i) {
    assert(0 <= num[i] - '0' && num[i] - '0' < ALPHABET_SIZE);
    if (currentNode->childs[num[i] - '0'] == NULL)
      break;

    currentNode = currentNode->childs[num[i] - '0'];

    if (currentNode->data != NULL) {
      assert(currentNode->data->text);
      last_forwarded_node = currentNode;
      last_forwarded_prefix_size = i + 1;
    }
  }

  // Decide how does the forwarded prefix look like. Variable
  // [last_forwarded_prefix_size] tells us how many characters from the input
  // string are redirected into that prefix. Must be 0 if last_forwarded_node
  // is NULL!
  char *forwarded_prefix =
      last_forwarded_node ? last_forwarded_node->data->text : "";
  if (!last_forwarded_node)
    assert(last_forwarded_prefix_size == 0);

  assert(result);
  int result_redirection_len =
      strlen(forwarded_prefix) + strlen(num) - last_forwarded_prefix_size;

  // This contains only one number.
  result->size = 1;
  result->numbers[0] = malloc(sizeof(char) * (result_redirection_len + 1));

  if (!result->numbers[0])
    return NULL;

  result->numbers[0][0] = '\0';
  strcat(strcat(result->numbers[0], forwarded_prefix),
         (num + last_forwarded_prefix_size));

  return result;
}

const char *phnumGet(const struct PhoneNumbers *pnum, size_t idx) {
  if (idx >= pnum->size)
    return NULL;

  return pnum->numbers[idx];
}

void phnumDelete(const struct PhoneNumbers *pnum) {
  if (pnum) {
    for (unsigned int i = 0; i < pnum->size; ++i)
      free(pnum->numbers[i]);
    free(pnum->numbers);
  }

  free((void *)pnum);
}

/// @brief Usuwa powtórzenia z tablicy.
/// Usuwa powtórzenia z tablicy wskaźników na napisy. Zakłada że tablica jest
/// posortowana. Jeśli jakiś napis się powtarza zwalnia jego zawartość używając
/// @p free. Zwraca nowy rozmiar tablicy. Zawartość tablicy za zwróconym
/// indeksem jest nieokreślona.
/// @param[in] arr      – Wskaźnik na pierwszy element tablicy.
/// @param[in] arr_size – Ilość elementów w wejściowej tablicy.
/// @return Nowy rozmiar tablicy. Rozmiar tablicy nie jest zmieniony, ale
/// zawartośc poza zwróconym indeksem jest niezdefiniowana.
static int unique(char **arr, int arr_size) {
  int write_idx = 0;
  for (int i = 0; i < arr_size; ++i) {
    // NOTE: This frees skipped entires in the array.
    while (i < arr_size - 1 && strcmp(arr[i], arr[i + 1]) == 0) {
      free(arr[i]);
      arr[i] = NULL;
      ++i;
    }

    assert(i >= write_idx);
    if (i > write_idx)
      arr[write_idx] = arr[i];

    ++write_idx;
  }

  return write_idx;
}

/// @brief Leksykograficzne porównanie dwóch napisów.
/// Leksykograficzne porównanie dwóch napisów, przekazanych przez void *. Oba
/// parametry są rzutowane na const char **
/// @param[in] self  – pierwszy napis do porównania.
/// @param[in] other – drugi napis do porównania.
/// @return -1, 0 lub 1, gdy @p self jest mniejszy leksykograficzne, równy, lub
///         większy od @p other.
static int lexicographicalCompare(const void *self, const void *other) {
  const char *_self = *(const char **)self;
  const char *_other = *(const char **)other;

  for (int i = 0;; ++i) {
    if (_self[i] == '\0' && _other[i] == '\0')
      return 0;
    else if (_self[i] == '\0' || _self[i] < _other[i])
      return -1;
    else if (_other[i] == '\0' || _other[i] < _self[i])
      return 1;
  }
}

const struct PhoneNumbers *phfwdReverse(struct PhoneForward *pf,
                                        const char *num) {
  assert(pf);
  char currentPrefix[strlen(num) + 2];
  currentPrefix[0] = '\0';

  struct PhoneNumbers *result = phnumNewEmpty(32);
  if (!result)
    return NULL;

  if (!isValidPhnum(num))
    return result;

  struct TrieNode *current = pf->prefixes;
  int currentPrefixSize = 0;

  for (const char *currentChar = num; (*currentChar) != '\0'; currentChar++) {
    int currentBranchIdx = (*currentChar) - '0';
    assert(inRange(currentBranchIdx, 0, ALPHABET_SIZE - 1));

    if (!current->childs[currentBranchIdx])
      // No more prefixes to find.
      break;

    current = current->childs[currentBranchIdx];
    currentPrefix[currentPrefixSize] = (*currentChar);
    currentPrefixSize++;
    currentPrefix[currentPrefixSize] = '\0';

    struct DataNode *redirection = current->data;
    struct DataNode *prev_redirection = NULL;

    while (redirection) {
      assert(!prev_redirection || prev_redirection->next == redirection);
      if (!trieValueUnderPrefixExists(pf->redirections, redirection->text,
                                      currentPrefix)) {
        // Remove this entry from the list, because its old. This is a lazy
        // deletion. This entry might have been removed long ago from the
        // redirections tree.

        // If this is a first one...
        if (!prev_redirection) {
          current->data = redirection->next;
        } else {
          prev_redirection->next = redirection->next;
        }

        struct DataNode *next_redirection = redirection->next;
        redirection->next = NULL;
        dataNodeDelete(redirection);

        redirection = next_redirection;
        continue;
      }

      if (result->size == result->capacity) {
        result->capacity *= 2;
        result->numbers =
            realloc(result->numbers, sizeof(char *) * result->capacity);
        if (!result->numbers)
          return NULL;
      }

      result->numbers[result->size] =
          malloc(sizeof(char *) * (strlen(redirection->text) + strlen(num) -
                                   currentPrefixSize + 1));

      result->numbers[result->size][0] = '\0';
      strcat(strcat(result->numbers[result->size], redirection->text),
             num + currentPrefixSize);
      result->size++;

      prev_redirection = redirection;
      redirection = redirection->next;
    }
  }

  if (result->size == result->capacity) {
    result->capacity *= 2;
    result->numbers =
        realloc(result->numbers, sizeof(char *) * result->capacity);
    if (!result->numbers)
      return NULL;
  }

  result->numbers[result->size] = duplicateStr(num);
  if (!result->numbers[result->size])
    return NULL;

  result->size++;

  qsort(result->numbers, result->size, sizeof(result->numbers[0]),
        lexicographicalCompare);
  int new_size = unique(result->numbers, result->size);
  result->size = new_size;

  return result;
}

/// @brief Pomocnicza funckja rekurencyjna wywoływana przez
/// phfwdNonTrivialCount. Sprawdza czy w wierzchołku znajduje się jakaś aktualna
/// wartość i na tej podstawie oblicza liczbę nietrywialnych numerów telefonów o
/// prefiksie pod jakim znajduje się wierzchołek currentRoot.
/// @param [in] redirectionsRoot – wierzchołek drzewa trie trzymającego
///                                przekierowania telefonów. Z podowdu 'leniwego
///                                usuwania' musimy sprawdzać, czy używane przez
///                                nas wartości są aktualne.
/// @param [in] currentRoot – wskaźnik na aktualne poddrzewo w drzewie
///                           prefiksów.
/// @param [in] digit_set – Zbiór dozwolonych znaków jakie mogą zawierać numery,
///                         które zliczamy.
/// @param [in] current_deep – Głębokośc w drzewie prefiksów, na jakiej znajduje
///                            się @p currentRoot.
/// @param [in] len – długość napisów jakie należy zliczyć.
/// @return Liczbę nietrywialnych numerów telefonów o prefiksie pod jakim
///         znajduje się wierzchołek currentRoot modulo dwa do potęgi liczba
///         bitów typu size_t.
static size_t phfwdNonTrivialCountAux(struct TrieNode *redirectionsRoot,
                                      struct TrieNode *currentRoot,
                                      const int *digit_set,
                                      const size_t current_deep,
                                      const size_t len) {
  assert(len >= current_deep);
  assert(currentRoot);

  if (dataListContaisEntryThatExists(redirectionsRoot, currentRoot)) {
    int numbers_of_digits_in_set = 0;
    for (int i = 0; i < 12; ++i)
      if (digit_set[i])
        numbers_of_digits_in_set++;

    return power(numbers_of_digits_in_set, len - current_deep);
  } else if (len == current_deep)
    // We dont have to go deeper that [len] nodes.
    return 0;

  size_t result = 0;
  for (int i = 0; i < ';' - '0' + 1; ++i)
    if (currentRoot->childs[i] && digit_set[i]) {
      result +=
          phfwdNonTrivialCountAux(redirectionsRoot, currentRoot->childs[i],
                                  digit_set, current_deep + 1, len);
    }

  return result;
}

size_t phfwdNonTrivialCount(struct PhoneForward *pf, const char *set,
                            size_t len) {
  if (!pf || !set || !strlen(set) || !len)
    return 0;

  int number_mask[12];
  int found_any_digit = 0;
  for (int i = 0; i < 12; ++i)
    number_mask[i] = 0;

  for (int i = 0; set[i] != '\0'; ++i)
    if (inRange(set[i], '0', ';')) {
      number_mask[set[i] - '0'] = 1;
      found_any_digit = 1;
    }

  if (!found_any_digit)
    return 0;

  // We iterate over prefixes tree, and search for numbers that match
  // reqiurements. There is no point in going deeper than [len] nodes.
  assert(pf->prefixes);
  return phfwdNonTrivialCountAux(pf->redirections, pf->prefixes, number_mask, 0,
                                 len);
}
