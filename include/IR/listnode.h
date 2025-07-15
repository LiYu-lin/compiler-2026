#pragma once
#include <cassert>
namespace IR {
    template<typename T>
    class List;
    struct Instruction;
    struct Use;
    struct BasicBlock;
    struct ListNode {
        const int id =0;
        ListNode *next = nullptr;
        ListNode *prev = nullptr;
        ListNode(int val) : id(val), next(nullptr),prev(nullptr){}
        ListNode(ListNode *ne, ListNode *pr, const int x) : id(x), next(ne), prev(pr) {}
        ListNode* getNext()  {
            return next;
        }
        ListNode* getPrev() {
            return prev;
        }
        int getID() const {
            return id;
        }
        bool isEmpty() const {
            return next == nullptr;
        }
        void clear() {
            while (next != nullptr) {
                ListNode* temp = next;
                next = next->next;
                delete temp;  // 迭代删除，避免递归
            }
        }
        friend struct List<IR::Instruction *>;
        friend struct List<IR::Use *>;
        friend struct List<IR::BasicBlock *>;
        private:
        void insertAfter(ListNode* node) {
            assert(node != nullptr && "Cannot insert after null");
            assert(node != this && "Cannot insert after self");
            // 检查是否形成循环
            ListNode* current = node;
            while (current != nullptr) {
                if (current == this) {
                    assert(false && "Error: Circular reference detected");
                    return;
                }
                current = current->next;
            }
            next = node->next;
            prev = node;
            if (node->next) {
                node->next->prev = this;
            }
            node->next = this;
        }
        void insertBefore(ListNode *node)
        {
            if (node == nullptr)
                return;
            next = node;
            prev = node->prev;
            if (node->prev != nullptr)
            {
                node->prev->next = this;
            }
            node->prev = this;
        }
        virtual void remove() {
            if (this == prev || this == next) {
                assert(false && "Error: Self-reference detected in remove()");
                return;
            }
            if (prev) prev->next = next;
            if (next) next->prev = prev;
            next = nullptr;
            prev = nullptr;
        }
    };
}