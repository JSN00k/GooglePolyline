/* This code works using a circular linked list. We keep a reference
   to the last node in the linked list (which is turn points to the
   first node). This makes it easy to append a new node when the data
   store in the current last node is filled. */
#include <stdlib.h>
#include <string.h>

#include "AppendableDataStore.h"

typedef struct LinkedList LinkedList;

/* A circularly linked list because you can append to the beginning and 
   end easily. */
struct LinkedList {
  void *data;
  unsigned dataCount;
  LinkedList *next;
};

struct AppendableDataStore {
  LinkedList *lastNode;
  /* This size of the dataType to be stored in the Linked List. */
  size_t dataTypeSize;
  /* The number of elements of dataType that each node can store. */
  unsigned capacity;
  unsigned nodeCount;

  /* Used by the AppendableDataStoreNext function to get the next
     piece of data. */
  LinkedList *enumerationNode;
  unsigned enumerationLocation;
  /* The number of elements currently contained in the linked list. */
  unsigned dataCount;
};


/* Initializes the first, or next node. For use by the AppendableDataStore. */
LinkedList *LinkedListCreate (unsigned count, size_t typeSize);

/* Frees all of the data in the linked list. For use by the AppendableDataStore. */
void LinkedListFree ();

AppendableDataStore *AppendableDataStoreCreate (unsigned count, size_t typeSize) {
  AppendableDataStore *result = malloc (sizeof (AppendableDataStore));
  result->dataTypeSize = typeSize;
  result->capacity = count;
  result->nodeCount = 0;
  result->lastNode = NULL;
  return result;
}

void AppendableDataStoreAddData (AppendableDataStore *manager,
                         void *data, unsigned count) {
  manager->dataCount += count;
  if (!manager->lastNode) {
    manager->lastNode = LinkedListCreate (manager->capacity,
                                          manager->dataTypeSize);
    ++manager->nodeCount;
  }
  
  LinkedList *list = manager->lastNode;
  if (list->dataCount + count <= manager->capacity) {
    memcpy (list->data + list->dataCount * manager->dataTypeSize,
            data, count * manager->dataTypeSize);
    list->dataCount += count;
    return;
  }

  LinkedList *currentNode = list;
  unsigned remainingDataToCopy = count;

  while (currentNode->dataCount + remainingDataToCopy > manager->capacity) {
    unsigned dataToCopyCount = manager->capacity - currentNode->dataCount;
    unsigned dataToCopySize = (unsigned)(dataToCopyCount * manager->dataTypeSize);
    memcpy (currentNode->data + currentNode->dataCount * manager->dataTypeSize,
            data, dataToCopySize);
    
    data += dataToCopySize;
    list->dataCount = manager->capacity;
    LinkedList *newNode = LinkedListCreate (manager->capacity, manager->dataTypeSize);
    newNode->next = list->next;
    list->next = newNode;
    ++manager->nodeCount;
    currentNode = newNode;
  }

  memcpy (currentNode->data, data, remainingDataToCopy * manager->dataTypeSize);
  currentNode->dataCount += remainingDataToCopy;
  manager->lastNode = list;
}

size_t AppendableDataStoreDataSize (AppendableDataStore *manager) {
  return manager->dataCount * manager->dataTypeSize;
}

/* Collapses the data stored in the linked list into result.
   result must have at least enough capacity for all the data. */
void AppendableDataStoreCollapseDataIntoResult (AppendableDataStore *manager,
                                                void *result) {
    if (!manager->nodeCount) {
    return;
  }

  LinkedList *currentNode = manager->lastNode;
  void *cpyLocation = result;
  do {
    currentNode = currentNode->next;
    size_t appendDataSize = currentNode->dataCount * manager->dataTypeSize;
    memcpy (cpyLocation, currentNode->data, appendDataSize);
    cpyLocation += appendDataSize;
  } while (currentNode != manager->lastNode);

  return;
}

/* Collapses all of the data in the linked list into a single 
   contiguous block of data. The caller of the function owns the
   data that is returned. */
void *AppendableDataStoreGetData (AppendableDataStore *manager) {
  void *result = malloc (AppendableDataStoreDataSize (manager));
  AppendableDataStoreCollapseDataIntoResult (manager, result);
  return result;
}

bool AppendableDataStoreNext (AppendableDataStore *manager, void *value) {
  if (manager->enumerationLocation == manager->dataCount) {
    if (manager->enumerationNode == manager->lastNode)
      return false;

    manager->enumerationNode = manager->enumerationNode->next;
    manager->enumerationLocation = 0;
  }

  memcpy (value,
          manager->enumerationNode + manager->enumerationLocation * manager->dataTypeSize,
          manager->dataTypeSize);
  return true;
}

void AppendableDataStoreFree (AppendableDataStore *manager) {
  LinkedListFree (manager->lastNode);
  free (manager);
}

LinkedList *LinkedListCreate (unsigned count, size_t typeSize) {
  LinkedList *node = malloc (sizeof (LinkedList));
  node->data = malloc (count * sizeof (typeSize));
  node->dataCount = 0;
  node->next = node;
  return node;
}

void LinkedListFree (LinkedList *list) {
  LinkedList *next = list;
  do {
    LinkedList *nextNext = next->next;
    free (next);
    next = nextNext;
  } while (next != list);
}
