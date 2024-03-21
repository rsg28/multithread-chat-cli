// Name: Aarham Haider
// Computing ID: aah13
// Student ID: 301462422

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// **** GLOBAL VARIABLES TO HANDLE STATIC ARRAYS **** //
Node NodeArray[LIST_MAX_NUM_NODES];
List ListArray[LIST_MAX_NUM_HEADS];

int ActiveLists = 0;
int ActiveNodes = 0;

bool IsInitialized = false;

List *FreeList;
Node *FreeNode;

static pthread_mutex_t QueueMutex = PTHREAD_MUTEX_INITIALIZER;

// **** HELPER FUNCTIONS TO COMMUNICATE DETAILS ABOUT LINKED LISTS **** //
bool IsEmpty(List *pList) { return pList->Count == 0; }

bool BeforeList(List *pList) { return (pList->Current == NULL && pList->Bound == LIST_OOB_START); }

bool AfterList(List *pList) { return (pList->Current == NULL && pList->Bound == LIST_OOB_END); }

bool NodesFull() { return ActiveNodes == LIST_MAX_NUM_NODES; }

bool ListsFull() { return ActiveLists == LIST_MAX_NUM_HEADS; }

void *ReturnNodeItem(Node *pNode) { return pNode->Item; }


// **** HELPER FUNCTIONS TO UPDATE LISTS/NODES AT THE ARRAY LEVEL **** //

void ResetNode(Node* pNode)
{
    pNode->Item = NULL;
    pNode->Prev = NULL;
}

void ResetList(List* pList)
{
    pList->First = NULL;
    pList->Current = NULL;
    pList->Last = NULL;
    pList->Count = 0;
    pList->Bound = LIST_OOB_START;
}

Node *OccupyNode()
{
    if (NodesFull()) { return NULL; }

    Node *SetNode = FreeNode;
    FreeNode = FreeNode->Next;
    ResetNode(SetNode);
    ActiveNodes++;

    return SetNode;
}

void ReleaseNode(Node *Node)
{
    if (NodesFull()) { return; }

    Node->Next = FreeNode;
    FreeNode = Node;
    ResetNode(Node);
    ActiveNodes--;
}

List *OccupyList()
{
    if (ListsFull()){ return NULL; }
    List *SetList = FreeList;
    FreeList = FreeList->Next;
    ResetList(SetList);
    ActiveLists++;

    return SetList;
}

void ReleaseList(List *List)
{
    if (ListsFull()){ return; }

    List->Next = FreeList;
    FreeList = List;
    ResetList(List);
    ActiveLists--;
}


// **** MAIN LIST FUNCTIONS START HERE **** //
List *List_create()
{
    // First call to List_create() sets up all of the arrays with next pointers
    if (IsInitialized == false)
    {
        IsInitialized = true;
        for (int i = 0; i < LIST_MAX_NUM_HEADS - 1; i++)
        {
            ListArray[i].Next = ListArray + i + 1;
        }
        ListArray[LIST_MAX_NUM_HEADS].Next = NULL;

        for (int j = 0; j < LIST_MAX_NUM_NODES - 1; j++)
        {
            NodeArray[j].Next = NodeArray + j + 1;
            NodeArray[j].Item = NULL;
        }
        NodeArray[LIST_MAX_NUM_NODES].Next = NULL;

        FreeList = &(ListArray[0]);
        FreeNode = &(NodeArray[0]);
    }

    if (ListsFull()) { return NULL; }

    // A new list is reserved on to the array and returned
    List *NewList = OccupyList();

    return NewList;
}

int List_count(List *pList) { return pList->Count; }

void *List_first(List *pList)
{
    if (IsEmpty(pList)) { return NULL; }

    // Setting current node to the first and returning item 
    pList->Current = pList->First;
    return ReturnNodeItem(pList->Current);
}

void *List_last(List *pList)
{
    if (IsEmpty(pList)) { return NULL; }

    // Setting current node to the last and returning item 
    pList->Current = pList->Last;
    return ReturnNodeItem(pList->Current);
}

void *List_next(List *pList)
{
    if (IsEmpty(pList)) { return NULL; }

    // Set current node to beyond list if next doesn't exist or is already beyond list
    if (!pList->Current->Next || AfterList(pList))
    {
        pList->Bound = LIST_OOB_END;
        pList->Current = pList->Current->Next;
        return NULL;
    }

    pList->Current = pList->Current->Next;
    return ReturnNodeItem(pList->Current);
}

void *List_prev(List *pList)
{
    if (IsEmpty(pList)) { return NULL; }

    // Set node at the back of the list if back doesn't exist or is already before list
    if (pList->Current == pList->First || BeforeList(pList))
    {
        pList->Bound = LIST_OOB_START;
        pList->Current = NULL;

        return NULL;
    }

    pList->Current = pList->Current->Prev;
    return ReturnNodeItem(pList->Current);
}

void *List_curr(List *pList)
{
    if (IsEmpty(pList)) { return NULL; }
    return ReturnNodeItem(pList->Current);
}

int List_insert_after(List *pList, void *pItem)
{
    if (NodesFull()) { return LIST_FAIL; }

    // Initialize a new node on the static node array
    Node *NewNode = OccupyNode();
    NewNode->Item = pItem;
    pList->Count++;

    // Set all of the markers in the list to the new node if it's the first entry
    if (pList->Count == 1)
    {
        pList->Current = pList->First = pList->Last = NewNode;
        NewNode->Next = NewNode->Prev = NULL;
        return LIST_SUCCESS;
    }

    // Unique operations for out of bounds conditions
    if (BeforeList(pList))
    {
        NewNode->Next = pList->First;
        NewNode->Prev = NULL;
        pList->First->Prev = NewNode;
        pList->First = NewNode;

        pList->Current = NewNode;
        return LIST_SUCCESS;
    }

    if (AfterList(pList))
    {
        NewNode->Next = NULL;
        NewNode->Prev = pList->Last;
        pList->Last->Next = NewNode;
        pList->Last = NewNode;

        pList->Current = NewNode;
        return LIST_SUCCESS;
    }

    // Set new node to last if insert at the tail
    if (pList->Current == pList->Last) { pList->Last = NewNode; }

    // General Case
    NewNode->Next = pList->Current->Next;
    NewNode->Prev = pList->Current;
    pList->Current->Next = NewNode;
    if (NewNode->Next) { NewNode->Next->Prev = NewNode; }

    // Set current node to newly inserted node
    pList->Current = NewNode;
    return LIST_SUCCESS;
}

int List_insert_before(List *pList, void *pItem)
{
    if (NodesFull()) { return LIST_FAIL; }

    // Go back a node and conduct the insert node
    List_prev(pList);
    List_insert_after(pList, pItem);
    return LIST_SUCCESS;
}

int List_append(List *pList, void *pItem)
{
    if (NodesFull()) { return LIST_FAIL; }

    // Go last and insert node
    List_last(pList);
    List_insert_after(pList, pItem);
    return LIST_SUCCESS;
}

int List_prepend(List *pList, void *pItem)
{
    if (NodesFull()) { return LIST_FAIL; }

    // Go before list and insert node
    List_first(pList);
    List_prev(pList);
    List_insert_after(pList, pItem);
    return LIST_SUCCESS;
}

void *List_remove(List *pList)
{
    if (!pList->Current) { return NULL; }

    Node *CurrentNode = pList->Current;
    void *CurrentItem = pList->Current->Item;
    pList->Count--;

    // Reset list and bring it beyond the list if remove clears the final node
    if (pList->Count == 0)
    {
        ResetList(pList);
        pList->Bound = LIST_OOB_END;
        ReleaseNode(CurrentNode);
        return CurrentItem;
    }

    if (BeforeList(pList) || AfterList(pList)) { return NULL; }

    // Send current node beyond list if removing the tail
    if (CurrentNode == pList->Last)
    {
        pList->Bound = LIST_OOB_END;
        pList->Last = CurrentNode->Prev;
        pList->Last->Next = NULL;
        pList->Current = pList->Last->Next;
        ReleaseNode(CurrentNode);

        return CurrentItem;
    }

    // Head case, set current node to the new head
    if (CurrentNode == pList->First)
    {
        pList->First = CurrentNode->Next;
        pList->Current = pList->First;
        ReleaseNode(CurrentNode);

        return CurrentItem;
    }

    // General Case; connect nodes before and after remove
    if (CurrentNode->Prev)
    {
        CurrentNode->Prev->Next = CurrentNode->Next;
        pList->Current = CurrentNode->Prev;
    }

    if (CurrentNode->Next)
    {
        CurrentNode->Next->Prev = CurrentNode->Prev;
        pList->Current = CurrentNode->Next;
    }

    ReleaseNode(CurrentNode);
    return CurrentItem;
}

void List_concat(List *pList1, List *pList2)
{
    // Just remove second list and don't do anything if second list is empty
    if (IsEmpty(pList2)) 
    { 
        ReleaseList(pList2); 
        return; 
    }

    // Copy all of list 2 onto list 1 if first list is empty
    if (IsEmpty(pList1))
    {
        *pList1 = *pList2;
        pList1->Current = pList2->Current;
        ReleaseList(pList2);
        return;
    }

    // General case, set beyond of list 1 to be first of list 2, before list 2 to be
    // last of list 1
    pList1->Last->Next = pList2->First;
    pList2->First->Prev = pList1->Last;
    pList1->Last = pList2->Last;

    int NewCount = pList1->Count + pList2->Count;
    pList1->Count = NewCount;

    // Destroy second list
    ReleaseList(pList2);
    return;
}

typedef void (*FREE_FN)(void *pItem);
void List_free(List *pList, FREE_FN pItemFreeFn)
{
    if (IsEmpty(pList)) { return; }

    // Start at the begining of the list and traverse all the way, calling remove
    // all the way and destroying the list afterwards
    List_first(pList);
    while (pList->Current != NULL)
    {
        if(pItemFreeFn) 
        {
            (*pItemFreeFn)(pList->Current->Item);
        }
        List_remove(pList);
    }
    ReleaseList(pList);
    return;
}

void *List_trim(List *pList)
{
    List_last(pList);
    void *ReturnItem = List_remove(pList);

    // Reset last again
    List_last(pList);
    return ReturnItem;
}

typedef bool (*COMPARATOR_FN)(void *pItem, void *pComparisonArg);
void *List_search(List *pList, COMPARATOR_FN pComparator, void *pComparisonArg)
{
    if (!pComparator || IsEmpty(pList)) { return NULL; }

    // Start at the begining of the list and traverse all the way, calling the 
    // comparator for every node, set the current node beyond list if not found
    List_first(pList);
    while (pList->Current)
    {
        if ((*pComparator)(pList->Current->Item, pComparisonArg))
        {
            return pList->Current->Item;
        }
        pList->Current = pList->Current->Next;
    }

    pList->Bound = LIST_OOB_END;
    return NULL;
}


int List_input(List* list, char* message)
{
    int InputValue;
    pthread_mutex_lock(&QueueMutex);
    {
        InputValue = List_prepend(list, message);
    }
    pthread_mutex_unlock(&QueueMutex);
    return InputValue;
}

char* List_output(List* list)
{
    char* Message;
    pthread_mutex_lock(&QueueMutex);
    {
        Message = List_trim(list);
    }
    pthread_mutex_unlock(&QueueMutex);

    return Message;
}

int List_lockedCount(List* list)
{
    int Count;
    pthread_mutex_lock(&QueueMutex);
    {
        Count = List_count(list);
    }
    pthread_mutex_unlock(&QueueMutex);

    return Count;
}