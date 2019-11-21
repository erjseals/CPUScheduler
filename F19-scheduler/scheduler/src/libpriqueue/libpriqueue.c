/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumptions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->size = 0;
	q->head	= NULL;
	q->comparer = comparer;
}


/**
  Insert the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	//create a new node with the value to be inserted into the queueueueue
	Node* newNode = malloc(sizeof(Node));
	newNode->data = ptr;
	newNode->next = NULL;

	//Insert into the linked list

	//Case 1:
	//check if the list is empty
	if(q->size == 0){
		q->head = newNode;
		q->size++;
		return 0;
	}

	//Case 2:
	//Find where this new node should go
	//This means finding a Node with an inner value that has a lower priority
	else{
		//Case 2a:
		//The linked list is not empty, however this new value has a higher Priority
		//than everything else in the linked list

		//comparer returns negative when the first argument is less than the second.
		//So this if tests whether the new node has higher priority (smaller value)
		//than the current head
		if(q->comparer(newNode->data, q->head->data) < 0){
			newNode->next = q->head;
			q->head = newNode;
			q->size++;
			return 0;
		}
		//
		//comparer returns 0 if the two arguments have the same priority.
		//if this is the case right now, we need to put this at the end of the list
		//of tasks with this priority.
		//So, really, we just need to find the moment when this new value is absolutely
		//greater than the value in the list
		else{
			//need some temp pointers
			//if either of these are null, the for loop size will take care of those
			//edge cases
			Node* temp = q->head;
			Node* tempNext = temp->next;
			int i;
			// we have size of the list
			for(i = 0 ; i < q->size-1 ; i++){
				if(q->comparer(newNode->data, tempNext->data) < 0){
					temp->next = newNode;
					newNode->next = tempNext;
					q->size++;
					return i+1;
				}
				else{
					temp = tempNext;
					tempNext = tempNext->next;
				}
			}
			//if the entire for loop was executed, then this is going on the back
			//also note, temp will point to the last element in the list
			temp->next = newNode;
			q->size++;
			return q->size - 1;
		}
	}

	//if this return gets called, an error occurred
	return -1;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	return q->head->data;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	if(q->size == 0){
		return NULL;
	}
	else{
		Node* temp = q->head->next;
		void* ret = q->head->data;
		free(q->head);
		q->head = temp;
		q->size--;
		return ret;
	}
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	return NULL;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	return 0;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	return 0;
}


/**
  Return the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{

}
