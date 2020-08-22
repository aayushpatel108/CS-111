#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {

  if (list == NULL || element == NULL)
    return;
  
  SortedList_t* ptr = list->next;
  while (ptr != list && strcmp(element->key, ptr->key) > 0) 
    ptr = ptr->next;

  if (opt_yield && INSERT_YIELD)
    sched_yield();

  element->prev = ptr->prev;
  element->next = ptr;
  ptr->prev->next = element;
  ptr->prev = element;
  
}

int SortedList_delete( SortedListElement_t *element) {

  if (opt_yield && DELETE_YIELD)
    sched_yield();
  
  if (element != NULL && element->prev->next != NULL && element->next->prev != NULL) {
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
  } else {
    return 1;
  }
  
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {

  if (list == NULL || key == NULL) 
    return NULL;

  SortedListElement_t *ptr = list->next;
  
  while (ptr != list) {
    if (!strcmp(ptr->key, key))
      return ptr;
    if (opt_yield && LOOKUP_YIELD) 
      sched_yield();
    ptr = ptr->next;
  }

  return NULL;
  
}

int SortedList_length(SortedList_t *list) {

  if (list == NULL) 
    return -1;

  int count = 0;
  SortedListElement_t *ptr = list->next;
  while (ptr != list) {
    if (opt_yield && LOOKUP_YIELD)
      sched_yield();
    count++;
    ptr = ptr->next;
  }

  return count;
  
}
