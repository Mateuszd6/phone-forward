/// @file
/// Implementacja modułu drzewa Trie.
///
/// @author Mateusz Dudziński <md394171@students.mimuw.edu.pl>
/// @copyright Uniwersytet Warszawski
/// @date 06.05.2018

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"
#include "util.h"

/// @brief Całkowicie usuwa poddrzewo.
/// Całkowicie usuwa wkazywane przez @p rootToDelete poddrzewo. Usuwa wszystkie
/// dane z drzewa, łącznie z wartościami w węzłach, ale nawet jeśli @p
/// rootToDelete nie jest korzeniem drzewa, i posiada ojca, nie zmienia reszty
/// drzewa. To znaczy, że jeśli wywołujący funkcje usuwa tylko część swojej
/// struktury Trie, musi zadbać o to, żeby wartości @ref TrieNode.nonNullChilds
/// oraz @ref TrieNode.childs w przodkach korzenia usuwanego poddrzewa zostały
/// zaktualizowane.
/// @param[in] rootToDelete – wskaźnik na korzeń usuwanego poddrzewa.
static void trieFreeSubtree(struct TrieNode *rootToDelete) {
  for (int i = 0; i < ALPHABET_SIZE; ++i)
    if (rootToDelete->childs[i])
      trieFreeSubtree(rootToDelete->childs[i]);

  if (rootToDelete->data) {
    dataNodeDelete(rootToDelete->data);
    rootToDelete->data = NULL;
  }

  free(rootToDelete);
}

struct DataNode *dataNodeNew(const char *text) {
  struct DataNode *result = malloc(sizeof(struct DataNode));
  if (result) {
    result->next = NULL;
    result->text = duplicateStr(text);
    if (!result->text) {
      free(result);
      return NULL;
    }
  }

  return result;
}

void dataNodeDelete(struct DataNode *node_to_delete) {
  if (node_to_delete) {
    if (node_to_delete->next)
      dataNodeDelete(node_to_delete->next);

    free(node_to_delete->text);
    free(node_to_delete);
  }
}

struct TrieNode *trieNodeNew(struct TrieNode *parent) {
  struct TrieNode *result = malloc(sizeof(struct TrieNode));
  if (result) {
    for (int i = 0; i < ALPHABET_SIZE; ++i)
      result->childs[i] = NULL;

    result->data = NULL;
    result->nonNullChilds = 0;
    result->parent = parent;
  }

  return result;
}

bool dataListContaisEntryThatExists(struct TrieNode *redirectionsRoot,
                                    struct TrieNode *trieNode) {
  struct DataNode *currentData = trieNode->data;

  while (currentData) {
    if (trieValueUnderPrefixExists(redirectionsRoot, currentData->text, NULL))
      return true;
    else {
      trieNode->data = currentData->next;
      currentData->next = NULL;
      dataNodeDelete(currentData);
      currentData = trieNode->data;
    }
  }

  return false;
}

bool trieAddText(struct TrieNode *trieRoot, const char *text,
                 struct DataNode *data, bool append,
                 struct DataNode **prevData) {
  assert(trieRoot);
  assert(text);
  assert(data);

  struct TrieNode *currentNode = trieRoot;

  for (int i = 0; text[i] != '\0'; ++i) {
    int currentBranchIdx = text[i] - '0';
    assert(inRange(currentBranchIdx, 0, ALPHABET_SIZE - 1));

    struct TrieNode *nextNode = currentNode->childs[currentBranchIdx];
    // If the node doesn't exist create it before going there.
    if (!nextNode) {
      nextNode = trieNodeNew(currentNode);

      if (!nextNode)
        return false; // An error has occured. Memory not allocated!

      currentNode->childs[currentBranchIdx] = nextNode;
      currentNode->nonNullChilds++;
    }

    currentNode = nextNode;
  }

  // If there is no data, append and replace do the same thing.
  if (!currentNode->data) {
    currentNode->data = data;

    if (!append)
      (*prevData) = NULL;
  } else {
    if (append) {
      struct DataNode *current = currentNode->data;
      while (current->next)
        current = current->next;
      current->next = data;
    } else {
      // We first save the prevous data in the prevData variable and then
      // insert a new one.
      (*prevData) = currentNode->data;
      currentNode->data = data;
    }
  }

  return true;
}

void trieDeleteSubtree(struct TrieNode *treeRoot,
                       struct TrieNode *rootToDelete) {
  if (treeRoot == rootToDelete)
    trieFreeSubtree(rootToDelete);
  else {
    // Cannot move to root, but move upwards unless there is a value in the
    // node, or there is more than one child.
    while (rootToDelete->parent != treeRoot &&
           rootToDelete->parent->nonNullChilds == 1 &&
           !rootToDelete->parent->data) {
      rootToDelete = rootToDelete->parent;
    }

    // Update the number of childs of the parent node, and find an index to
    // the current node.
    rootToDelete->parent->nonNullChilds--;
    int idxInParent = -1;
    for (int i = 0; i < ALPHABET_SIZE; ++i)
      if (rootToDelete->parent->childs[i] == rootToDelete) {
        idxInParent = i;
        break;
      }

    assert(idxInParent >= 0);
    assert(rootToDelete->parent->nonNullChilds > 0 ||
           rootToDelete->parent->data || rootToDelete->parent == treeRoot);

    // NULL-out the referece to the root of the removed subtree,
    // and now it is save to perform treeFreeSubtree.
    rootToDelete->parent->childs[idxInParent] = NULL;
    trieFreeSubtree(rootToDelete);
  }
}

void trieRemoveOneEntry(struct TrieNode *root, const char *text,
                        const char *entryToRemove) {
  assert(root);
  assert(text);
  assert(entryToRemove);

  struct TrieNode *currentNode = root;

  for (const char *currentChar = text; (*currentChar) != '\0'; currentChar++) {
    int currentBranchIdx = (*currentChar) - '0';
    assert(inRange(currentBranchIdx, 0, ALPHABET_SIZE - 1));

    if (!currentNode->childs[currentBranchIdx]) {
      assert(!"This assumes that [text] matches the TRIE!");
      return;
    }

    currentNode = currentNode->childs[currentBranchIdx];
  }

  struct DataNode *currentData = currentNode->data;
  struct DataNode *prevData = NULL;

  if (!currentData) {
    assert(
        !"This assumes that [entryToRemove] exists in the trie under [text]");
    return;
  }

  if (!currentData->next) {
    // If there is only one element in the data, we assert that this the one we
    // search for, because we assume that this one exists under the prefix in
    // the Trie.
    assert(strcmp(currentData->text, entryToRemove) == 0);
    dataNodeDelete(currentNode->data);
    currentNode->data = NULL;

    if (currentNode->nonNullChilds == 0)
      trieDeleteSubtree(root, currentNode);

    return;
  }

  /// Search for entryToRemove in the list. We assume it is there!
  while (currentData) {
    if (strcmp(currentData->text, entryToRemove) == 0)
      break;

    if (!currentData->next) {
      assert(!"[entryToremove] was not found at the [text] in the trie!");
      return;
    }

    prevData = currentData;
    currentData = currentData->next;
  }

  if (prevData)
    prevData->next = currentData->next;
  else
    currentNode->data = currentData->next;

  // Use the dataNode deletion funcion, but before, make sure only currentData
  // is freed.
  currentData->next = NULL;
  dataNodeDelete(currentData);

  // Because we handled the case when there is only one in a list.
  assert(currentNode->data);
}

int trieValueUnderPrefixExists(struct TrieNode *root, const char *prefix,
                               const char *value) {
  struct TrieNode *currentNode = root;

  for (const char *currentChar = prefix; (*currentChar) != '\0';
       currentChar++) {
    int currentBranchIdx = (*currentChar) - '0';
    currentNode = currentNode->childs[currentBranchIdx];
    if (!currentNode)
      return 0;
  }

  if (!currentNode)
    return 0;

  struct DataNode *current = currentNode->data;
  if (!value)
    return (current != NULL);

  while (current) {
    if (strcmp(current->text, value) == 0)
      return 1;

    current = current->next;
  }

  return 0;
}
