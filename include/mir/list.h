#pragma once
#include "listnode.h"

namespace IR {
    template <typename T>
    class List {
    public:
        int sz =0; // size of the list
        List() {
            sz = 0;
            head = new ListNode(0); 
            tail = new ListNode(0);
            head->next = tail;
            tail->prev = head;
        }
        ~List() {
            clear();
        }
        void clear() {
            while (head != nullptr) {
                ListNode* next = head->getNext();
                delete head;
                head = next;
            }
            tail = nullptr;
        }
        ListNode* getHead() const {
            return head->getNext();
        }
        ListNode* getTail() const {
            return tail->getPrev();
        }
        void insertAfter(ListNode* newNode, T* node) {
            if (node == nullptr || newNode == nullptr || node->id == 0) return;
            node->insertAfter(newNode);
            sz++;
        }
        void insertBefore(ListNode* newNode, T* node) {
            if (node == nullptr || newNode == nullptr || node->id == 0) return;
            node->insertBefore(newNode);
            sz++;
        }
        void PushBack(T* node) {
            if (node == nullptr || node->id == 0) return;
            insertBefore(tail);
        }
        void PushFront(T* node) {
            if (node == nullptr || node->id == 0) return;
            insertAfter(head);
        }
        void remove(T* node) {
            if (node == nullptr || node->id == 0) return;
            node->remove();
            sz--;
        }
    private:
        ListNode* head;
        ListNode* tail;
    };
}
