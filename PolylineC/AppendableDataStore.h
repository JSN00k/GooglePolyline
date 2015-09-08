#ifndef googlePolylineTest_AppendableDataStore_h
#define googlePolylineTest_AppendableDataStore_h

#include <stdbool.h>

struct AppendableDataStore;
typedef struct AppendableDataStore AppendableDataStore;

/* Creates the linked list manager. You probably shouldn't be calling 
   LinkedList functions directly but instead be using these functions.
   count: The number of elements of dataType that the list can hold.
   dataTypeSize: The size of each element that you're going to add to the data. 
*/
AppendableDataStore *AppendableDataStoreCreate (unsigned count,
                                                size_t dataTypeSize);

/* Add data to the linked list. Things will be bad if the data type isn't
   the same size as you gave the AppendableDataStore when you created it. */
void AppendableDataStoreAddData (AppendableDataStore *store,
                                 void *data, unsigned count);

/* returns the total size of all of the data contained in the linked list. */
size_t AppendableDataStoreDataSize (AppendableDataStore *store);

/* Copies all of the data from the linked list into result. result must
   have at least enough room for all of the data in the linked list. */
void AppendableDataStoreCollapseDataIntoResult (AppendableDataStore *store,
                                                void *result);

/* Returns all of the data contained in the linked list. You own the data
   returned by this function. */
void *AppendableDataStoreGetData (AppendableDataStore *store);

/* Get's the next value from the dataStore and puts it in value. Returns
   true if there is a value. Returns false when at the end of the store.
   This allows you enumerate the data store with code something like:

   MyType value;
   while (AppendableDataStoreNext (store, &value)) {
     // Use value is some way.
     ...
   }
*/
bool AppendableDataStoreNext (AppendableDataStore *store, void *value);

/* Resets the enumeration. i.e. AppendableDataStoreNext will now return
   the first value stored in the dataStore again. */
void AppendableDataStoreResetEnumeration (AppendableDataStore *store);
  
void AppendableDataStoreFree (AppendableDataStore *store);

#endif
