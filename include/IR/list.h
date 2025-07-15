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
            tail->prev = head;
            head->next = tail;
        }

        bool isEmpty() const {
            return head->getNext() == tail;
        }
        ListNode* getHead() const {
            return head->getNext();
        }
        ListNode* getTail() const {
            return tail->getPrev();
        }
        void insertAfter(ListNode* newNode, T node) {
            node->insertAfter(newNode);
            sz++;
        }
        void insertBefore(ListNode* newNode, T node) {
            node->insertBefore(newNode);
            sz++;
        }
        void PushBack(T node) {
            if (node == nullptr || node->id == 0) return;
            insertBefore(tail, node);
        }
        void PushFront(T node) {
            if (node == nullptr || node->id == 0) return;
            insertAfter(head, node);
        }
        void remove(T node) {
            if (node == nullptr || node->id == 0) return;
            node->remove();
            sz--;
        }
        int getSize() const {
            return sz;
        }
    private:
        ListNode* head;
        ListNode* tail;
    };
}
