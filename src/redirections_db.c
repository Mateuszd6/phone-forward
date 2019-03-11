/// @file
/// Moduł implementujący operacje na kolekcji baz przekierowań.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 27.05.2018

#include <string.h>

#include "phone_forward.h"
#include "redirections_db.h"

/// Pojedyńczy węzeł kolekcji baz przekierowań, który implementujemy jak listę.
struct RedirationsDBNode {
  /// Aktualna wartość w wierzchołku.
  struct RedirectionsDatabase *phone_forward_data;

  /// Wskaźnik na następny element.
  struct RedirationsDBNode *next;
};

/// @brief Początek struktury bazy przekierowań.
/// Wskaźnik na początek listy wszystkich przechowywanych baz przekierowań
/// telefonów.
struct RedirationsDBNode *redirections_database_head = NULL;

struct RedirectionsDatabase *current_database = NULL;

/// @brief Tworzy nową strukturę.
/// Tworzy nowy obiekt typu redirectionsDBNew posiadający nazwę @p name.
/// @param[in] name – Nazwa jaka zostanie nadana stworzonej strukturze.
/// @return Wskaźnik na zaalokowaną strukturę, lub NULL, gdy nie udało się
///         zarezerwować pamięci.
static struct RedirectionsDatabase *redirectionsDBNew(const char *name) {
  struct RedirectionsDatabase *result =
      malloc(sizeof(struct RedirectionsDatabase));

  if (result) {
    result->phfwd = phfwdNew();
    result->name = malloc(sizeof(char) * (strlen(name) + 2));
    if (!name) {
      phfwdDelete(result->phfwd);
      free(result);

      return NULL;
    }

    strcpy(result->name, name);
    if (!result->phfwd) {
      free(result);
      return NULL;
    }
  }

  return result;
}

/// @brief Usuwa strukturę.
/// Usuwa strukruę RedirectionsDatabase. Nie robi nic, gdy @p red_db jest @p
/// NULL.
/// @param[in] red_db – Wskaźnik na strukruę która ma zostać usunięta.
static inline void redirectionsDBDelete(struct RedirectionsDatabase *red_db) {
  if (red_db) {
    free(red_db->name);
    phfwdDelete(red_db->phfwd);
    free(red_db);
  }
}

/// @brief Szuka stkrukutry bazy przekierowań. w bazie danych struktur
/// przekierowań. Szuka stkrukutry bazy przekierowań w bazie danych struktur
/// przekierowań.
/// @param[in] name – Nazwa stkrukutry jaką należy wyszukać.
/// @return Wskaźnik na znalezioną strukturę, lub @p NULL, gdy taka nie
///         istnieje.
static inline struct RedirectionsDatabase *
redirectionsDBFind(const char *name) {
  struct RedirationsDBNode *current = redirections_database_head;

  while (current) {
    if (strcmp(current->phone_forward_data->name, name) == 0)
      return current->phone_forward_data;

    current = current->next;
  }

  return NULL;
}

int setOrCreateDatabaseWithName(const char *name) {
  struct RedirectionsDatabase *db = redirectionsDBFind(name);
  if (!db) {
    db = redirectionsDBNew(name);
    if (!db)
      return 0;

    struct RedirationsDBNode *prev_head = redirections_database_head;

    redirections_database_head = malloc(sizeof(struct RedirationsDBNode));

    if (!redirections_database_head) // Memory error - could not allocate
                                     // memory.
      return 0;

    (*redirections_database_head) = (struct RedirationsDBNode){db, prev_head};
    current_database = db;
  }

  current_database = db;
  return 1;
}

int deleteDatabaseWithName(const char *name) {
  struct RedirationsDBNode *current = redirections_database_head;
  struct RedirationsDBNode *prev = NULL;

  while (current) {
    if (strcmp(current->phone_forward_data->name, name) == 0) {
      if (prev)
        prev->next = current->next;
      else
        redirections_database_head = current->next;

      if (current->phone_forward_data == current_database)
        current_database = NULL;

      redirectionsDBDelete(current->phone_forward_data);
      free(current);

      return 1;
    }

    prev = current;
    current = current->next;
  }

  return 0;
}

void clearAllRedirectionsDatabase() {
  struct RedirationsDBNode *current = redirections_database_head;
  while (current) {
    redirectionsDBDelete(current->phone_forward_data);

    struct RedirationsDBNode *next = current->next;
    free(current);

    current = next;
  }

  redirections_database_head = NULL;
}
