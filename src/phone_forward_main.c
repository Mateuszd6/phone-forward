/// @file
/// Plik definiujący entry point programu phone_forward.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 27.05.2018

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "input_parser.h"
#include "phone_forward.h"
#include "redirections_db.h"

/// @brief Wywołuje operację @p op.
/// @param[in] op – Struktura typu @ref Operation; Operacja, która ma zostać
///                 wykonana.
/// @return 1 Gdy operacja powiodła się, 0 w przypadku błędu wykonania.
int preformOperation(const struct Operation *op) {
  switch (op->performed_operation) {
  case OT_ADD:
    assert(op->args[0]);
    return setOrCreateDatabaseWithName(op->args[0]);

  case OT_DEL_DATABASE:
    assert(op->args[0]);
    return deleteDatabaseWithName(op->args[0]);

  case OT_DEL_PHONE_NUM: {
    assert(op->args[0]);
    if (!current_database)
      return 0;

    // We assume that this function cannot fail...
    phfwdRemove(current_database->phfwd, op->args[0]);
    return 1;
  }

  case OT_REDIRECT: {
    assert(op->args[0]);
    assert(op->args[1]);
    if (!current_database)
      return 0;

    return phfwdAdd(current_database->phfwd, op->args[0], op->args[1]);
  }

  case OT_NON_TRIV: {
    assert(op->args[0]);
    if (!current_database)
      return 0;

    int len = strlen(op->args[0]) - 12;
    if (len < 0)
      len = 0;

    size_t result =
        phfwdNonTrivialCount(current_database->phfwd, op->args[0], len);
    printf("%zu\n", result);
    return 1;
  }

  case OT_GET: {
    assert(op->args[0]);
    if (!current_database)
      return 0;

    const struct PhoneNumbers *result =
        phfwdGet(current_database->phfwd, op->args[0]);

    if (result) {
      printf("%s\n", phnumGet(result, 0));
      assert(!phnumGet(result, 1));
      phnumDelete(result);
      return 1;
    } else
      return 0;
  }

  case OT_REVERSE: {
    assert(op->args[0]);
    if (!current_database)
      return 0;

    const struct PhoneNumbers *result =
        phfwdReverse(current_database->phfwd, op->args[0]);

    if (result) {
      const char *num;
      int idx = 0;
      while ((num = phnumGet(result, idx++)) != NULL)
        printf("%s\n", num);
      phnumDelete(result);
      return 1;
    } else
      return 0;
  }

  // NOTE: Should not reach.
  default:
    assert(!"Unexpected operation type.");
    return 0;
  }
}

/// Entry point parsera i programu a przekierowaniach numerów telefonów.
int main() {
  struct Operation nextOperation;
  int feedback;

  while ((feedback = inputParseNextOperation(&nextOperation)) == IF_OK) {
    if (!preformOperation(&nextOperation)) {
      for (int i = 0; i < 2; ++i)
        if (nextOperation.args[i]) {
          free(nextOperation.args[i]);
          nextOperation.args[i] = NULL;
        }
      printOperationError(&nextOperation);
      feedback = IF_ERROR;
      break;
    }

    for (int i = 0; i < 2; ++i)
      if (nextOperation.args[i]) {
        free(nextOperation.args[i]);
        nextOperation.args[i] = NULL;
      }
  }

  clearAllRedirectionsDatabase();

  if (feedback == IF_ERROR)
    return 1;

  return 0;
}
