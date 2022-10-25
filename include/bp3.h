//===----------------------------------------------------------------------===//
//
//
// Identification:   include/b_plus_tree.h
//
//
//===----------------------------------------------------------------------===//
#pragma once

#include <string>
#include <vector>

#define MAX_FANOUT 4
#define KeyType int


// Value structure we insert into BPlusTree
struct RecordPointer {
    int page_id;
    int record_id;
    RecordPointer() : page_id(0), record_id(0) {};
    RecordPointer(int page, int record) : page_id(page), record_id(record) {};
};


// BPlusTree Node
class Node {
public:
    Node(bool leaf) : key_num(0), is_leaf(leaf) {};
    bool is_leaf;
    int key_num;
    KeyType keys[MAX_FANOUT - 1];
    friend class BPlusTree;
};

// internal b+ tree node
class InternalNode : public Node {
public:
    InternalNode() : Node(false) {};
    Node* children[MAX_FANOUT];
};

class LeafNode : public Node {
public:
    LeafNode() : Node(true) {};
    RecordPointer pointers[MAX_FANOUT - 1];
    // pointer to the next/prev leaf node
    LeafNode* next_leaf = NULL;
    LeafNode* prev_leaf = NULL;
};


/**
 * Main class providing the API for the Interactive B+ Tree.
 *
 * Implementation of simple b+ tree data structure where internal pages direct
 * the search and leaf pages contain record pointers
 * (1) We only support (and test) UNIQUE key
 * (2) Support insert & remove
 * (3) Support range scan, return multiple values.
 * (4) The structure should shrink and grow dynamically
 */

class BPlusTree {
private:
    void PlaceParent(const KeyType& key, InternalNode* cursor, Node* child);
    InternalNode* FindParent(Node* cursor, Node* child);
    Node* Traverse(const KeyType& key, Node* node);
    LeafNode* Find(const KeyType& key, int& keyIndex);
    void DeleteLeaf(LeafNode* leaf);
    void DeleteInternalNode(InternalNode* intNode);
    void Merge(const KeyType& key, InternalNode* cursor, Node* child);
    // pointer to the root node.
    Node* root;
public:
    BPlusTree() { root = NULL; };

    // Returns true if this B+ tree has no keys and values
    bool IsEmpty() const;

    // Insert a key-value pair into this B+ tree.
    bool Insert(const KeyType& key, const RecordPointer& value);

    // Remove a key and its value from this B+ tree.
    void Remove(const KeyType& key);

    // return the value associated with a given key
    bool GetValue(const KeyType& key, RecordPointer& result);

    // return the values within a key range [key_start, key_end) not included key_end
    void RangeScan(const KeyType& key_start, const KeyType& key_end,
        std::vector<RecordPointer>& result);
};

struct TemporaryLeafNode {
    KeyType keys[MAX_FANOUT];
    RecordPointer recordPointers[MAX_FANOUT];
    TemporaryLeafNode(LeafNode* leaf)
    {
        for (int i = 0; i < MAX_FANOUT - 1; i++)
        {
            keys[i] = leaf->keys[i];
            recordPointers[i] = leaf->pointers[i];
        }
    }

    void ShiftRight(int from)
    {
        for (int j = MAX_FANOUT - 1; j > from; keys[j] = keys[j - 1],
            recordPointers[j] = recordPointers[j - 1], j--);
    }
};


struct TemporaryInternalNode {
    KeyType keys[MAX_FANOUT];
    Node *children[MAX_FANOUT + 1];
    TemporaryInternalNode(InternalNode* internalNode)
    {
        for (int i = 0; i < MAX_FANOUT - 1; i++)
        {
            keys[i] = internalNode->keys[i];
        }
        for (int i = 0; i < MAX_FANOUT; i++)
        {
            children[i] = internalNode->children[i];
        }

    }

    void ShiftRight(int from)
    {
        for (int j = MAX_FANOUT - 1; j > from; keys[j] = keys[j - 1], j--);
        for (int j = MAX_FANOUT; j > from + 1; children[j] = children[j - 1], j--);

    }
};
