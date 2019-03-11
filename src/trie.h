/// @file
/// Interfejs modułu drzewa Trie.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 06.05.2018

#ifndef __TRIE_H__
#define __TRIE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

/// Makro ustalające maksymalną liczbę dzieci w wierzchołku drzewa Trie.
#define ALPHABET_SIZE (12)

/// Struktura stanowiąca liste jednostronną napisów przechowywanych w Trie.
struct DataNode {
  /// Napis przechowywany w wierzchołku drzewa.
  char *text;

  /// Wskaźnik na następny element listy, lub @p NULL, gdy ten jest ostatni.
  struct DataNode *next;
};

/// @brief Pojedyńczy wierzchołek Trie.
/// Ma listę synów, rozmiaru `ALPHABET_SIZE`, wskaźnik na ojca, oraz na obiekt
/// typu DataNode, jeśli do tego wierzchołka przypisana jest konkretna
/// wartość. Do tego przechowywana jest wartość, ile dzieci dokładnie znajduje
/// się w węźle.
struct TrieNode {
  /// @brief Informacja ile dzieci ma dany węzeł.
  /// Pomocna przy usuwaniu, a aktualizowanie jej jest mniej kosztowne, niż
  /// przechodzenie całej tablicy za każdym razem.
  int nonNullChilds;

  /// Tablica rozmiaru `ALPHABET_SIZE` dzieci danego węzła.
  struct TrieNode *childs[ALPHABET_SIZE];

  /// Wskaźnik na ojca danego wierzchołka.
  /// Może być @p NULL, gdy np. wierzchołek jest korzeniem drzewa.
  struct TrieNode *parent;

  /// Gdy nie @p NULL, wskazuje na początek listy elementów przypisanych do
  /// danego węzła.
  struct DataNode *data;
};

/// @brief Tworzy nową strukturę.
/// Tworzy nową strukturę zawierającą wskaźnik na kopię napisu @p text.
/// @param[in] text – tekst jaki ma zawierać nowa struktura.
/// @return Wskaźnik na nowo utworzoną strukturę lub NULL, gdy nie udało się
/// zaalokować pamięci.
struct DataNode *dataNodeNew(const char *text);

/// @brief Usuwa strukturę.
/// Usuwa całą zawartość struktury, do końca listy. Nic nie robi, jeśli
/// @p node_to_delete jest @p NULL.
/// @param[in] node_to_delete – Wskaźnik na pierwszy element do usunięcia.
void dataNodeDelete(struct DataNode *node_to_delete);

/// @brief Sprawdza czy choć jedna wartość przypisana do @p trieNode jest
/// aktualna. Sprawdza aktualność listy wartości z @p trieNode z drzewem @p
/// redirectionsRoot. Nieznajdujące się wartości zostają usunięte z listy. Nie
/// sprawdza całej listy, przerywa sprawdzanie kiedy tylko znajdzie pierwszą
/// pasującą wartość.
/// @param [in] redirectionsRoot – wskaźnik na korzeń drzewa, w którym
///                                szukane wartości.
/// @param [in] trieNode – Wskaźnika na węzeł drzewa, którego aktualność
///                        sprawdzamy.
/// @return @p true jeśli choć jedna wartość z @p trieNode jest aktualna, to
///            znaczy istnieje wartość pod tym prefiksem w drzewie @p
///            redirectionsRoot, false w przeciwnym wypadku.
bool dataListContaisEntryThatExists(struct TrieNode *redirectionsRoot,
                                    struct TrieNode *trieNode);

/// @brief Tworzy nową strukturę.
/// Tworzy nową strukturę typu TrieNode, ustawiając wkaźnik na ojca
/// tworzonego wierzchołka w drzewie trie. Wywołujący procedurę musi sam ustawić
/// wskaźnik @p childs[index] w strukturze @p parent przy dodawaniu tego
/// wierzchołka do drzewa. Alokuje pamięć, która musi być zwolniona używając
/// free.
/// @param[in] parent – wskaźnik na ojca danego wierzchołka, może być @p NULL,
///                     gdy np. tworzony jest wierzchołek drzewa.
/// @return Wskaźnik na zaalokowaną strukturę, lub @p NULL, gdy nie udało się
///         zaalokować pamięci.
struct TrieNode *trieNodeNew(struct TrieNode *parent);

/// @brief Dodaje tekst to Trie.
/// Dodaje obiekt @p data do Trie wskazywanego przez @p tireRoot, pod prefiksem
/// @p text.
/// @param[in] trieRoot – Korzeń drzewa Trie do którego dodawana jest wartość.
/// @param[in] text – Prefiks pod jakim ma być dodana wartość Pamięć na
///                   wszystkie wierzchołki, których nie ma, zostaje
///                   zaalokowana.
/// @param[in] data – Obiekt jaki ma zostać dodany.
/// @param[in] append – Gdy @p true, wartość w @p data zostanie dodana na koniec
///                     listy w wierzchołku pod prefiksem @p text. Gdy @p false
///                     poprzednia wartość zostane zastąpiona obecną.
/// @param[out] prevData – Jeśli @p append jest @p true, to poprzednia wartość
///                        zostaje zapisana do tej zmiennej.  Wywołujący musi
///                        sam zwolnić ten obiekt, bo nie ma go już w
///                        drzewie. Jeśli @p append jest @p false, ten wskaźnik
///                        jest ignorowany.
/// @return @p true jeśli operacja powiodła się, @p false, gdy nie udało się
///            zaalokować pamięci.
bool trieAddText(struct TrieNode *trieRoot, const char *text,
                 struct DataNode *data, bool append,
                 struct DataNode **prevData);

/// @brief Bezpiecznie usuwa poddrzewo.
/// Usuwa poddrzewo, ale dba o to, żeby poprawna struktura drzewa została
/// zachowana. Dokouje zmian w drzewie, potencjalnie zmienia korzeń usuwanego
/// poddrzewa na wyższy, by zapewnić optymalne zarządzanie pamięcią, następnie
/// wywołuje @ref trieFreeSubtree.
/// @param[in] treeRoot – Wskaźnik na korzeń drzewa. Korzenia drzewa nie można
///                       usunąć więc potrzebny jest wskaźnik, by nie zmienić na
///                       niego usuwanego korzenia.
/// @param[in] rootToDelete – Wskaźnik na korzeń usuwanego
///                           poddrzewa. Potencjalnie może usunąć więcej
///                           wierzchołków. Np. Gdy dane drzewo A -> B -> C ->
///                           D, a tylko B i D mają przypisane wartośći, a @p
///                           rootToDelete wskazuje na D, to usunięte zostanie
///                           całe poddrzewo C -> D.
void trieDeleteSubtree(struct TrieNode *treeRoot,
                       struct TrieNode *rootToDelete);

/// @brief Usuwa dokładnie jedną wartość z drzewa.
/// Usuwa dokładnie jedną wartość (@p entryToRemove) z drzewa wskazywanego przez
/// @p root, znajdującego się pod prefiksem @p text. Zakłada że wartość ta
/// znajduje się w drzewie!
/// @param[in] root – Wskaźnik do drzewa z jakiego wartość ma zostać
///                   usunięta.
/// @param[in] text – Tekst pod jakim znajduje się wartość która ma
///                   zostać usunięta.
/// @param[in] entryToRemove – Tekst, który ma zostać usunięty.
void trieRemoveOneEntry(struct TrieNode *root, const char *text,
                        const char *entryToRemove);

/// @brief Sprawdza czy pod prefikes @p text znajduje się wartość @p value.
/// Zwraca 1, gdy pod @p value znajduje sie w drzewie trie pod wskazanym
/// prefikem
/// @p text.
/// @param[in] root – Korzeń drzewa trie.
/// @param[in] prefix – Tekst pod, którym ma znajdować się wartość.
/// @param[in] value – Wartość której istnienie sprwdzamy w drzewie, gdy jest
///                    NULL, funcja zwraca 1, gdy jakikolwiek element znajduje
///                    sie pod danum prefiksem.
int trieValueUnderPrefixExists(struct TrieNode *root, const char *prefix,
                               const char *value);

#endif /* __TRIE_H__ */
