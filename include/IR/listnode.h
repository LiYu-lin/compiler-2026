#pragma once

namespace IR {
    template<typename T>
    class List;
    struct Instruction;
    struct Use;
    struct BasicBlock;
    struct ListNode {
        const int id =0;
        ListNode* next;
        ListNode* prev;
        ListNode(int val) : id(val), next(nullptr) {}
        ~ListNode() {
            delete next;  // 递归删除后续节点
        }
        ListNode* getNext() const {
            return next;
        }
        ListNode* getPrev() const {
            return prev;
        }
        int getID() const {
            return id;
        }
        bool isEmpty() const {
            return next == nullptr;
        }
        void clear() {
            delete next;  // 删除后续节点
            next = nullptr;  // 清空指针
        }
        friend struct List<IR::Instruction *>;
        friend struct List<IR::Use *>;
        friend struct List<IR::BasicBlock *>;
        private:
        void insertAfter(ListNode* node) {
            if (node == nullptr) return;
            node->next = next;
            if (next != nullptr) {
                next->prev = node;
            }
            next = node;
            node->prev = this;
        }
        void insertBefore(ListNode* node) {
            if (node == nullptr) return;
            node->prev = prev;
            if (prev != nullptr) {
                prev->next = node;
            }
            prev = node;
            node->next = this;
        }
        virtual void remove() {
            if (prev != nullptr) {
                prev->next = next;
            }
            if (next != nullptr) {
                next->prev = prev;
            }
            prev = nullptr;
            next = nullptr;
        }
    };
}