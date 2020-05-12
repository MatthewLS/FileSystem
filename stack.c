/**************************************************************
* Class: CSC-415-0# Spring 2020
* Group Name: Cabinet
* Name: File System

* Student ID: 918297424
* Name: Matthew Stephens
* Student ID: 915173797
* Name: Benjamin Lewis
* Student ID: 917871245
* Name: Tejas Vajjhala
* Student ID: 918383679
* Name: Chris Manaoat
*
* Assignment 3 – File System *
*
*
*  File: stack.c
*
* Description: The world's greatest file cabinet
*
* **************************************************************/

/// C program for linked list implementation of stack
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// A structure to represent a stack
struct StackNode {
    int data;
    struct StackNode* next;
};

struct StackNode* newNode(int data)
{
    struct StackNode* stackNode = (struct StackNode*)malloc(sizeof(struct StackNode));
    stackNode->data = data;
    stackNode->next = NULL;
    return stackNode;
}

int isEmpty(struct StackNode* root)
{
    return !root;
}

void push(struct StackNode** root, int data)
{
    struct StackNode* stackNode = newNode(data);
    stackNode->next = *root;
    *root = stackNode;
   // printf("%d pushed to stack\n", data);
}

int pop(struct StackNode** root)
{
    if (isEmpty(*root))
        return INT_MIN;
    struct StackNode* temp = *root;
    *root = (*root)->next;
    int popped = temp->data;
    free(temp);

    return popped;
}

int peek(struct StackNode* root)
{
    if (isEmpty(root))
        return INT_MIN;
    return root->data;
}
/*
int main()
{
    struct StackNode* freeBlockStack = NULL;

    push(&freeBlockStack, 10);
    push(&freeBlockStack, 20);
    push(&freeBlockStack, 30);

    printf("%d popped from stack\n", pop(&root));

    printf("Top element is %d\n", peek(root));

    return 0;
}
*/