#include "bp3.h"

void BPlusTree::PlaceParent(const KeyType &key, InternalNode* cursor, Node* child)
{
    if (cursor->key_num < MAX_FANOUT - 1)
    {
        int i = 0;
        for (; key > cursor->keys[i] && i < cursor->key_num; i++);
        for (int j = cursor->key_num; j > i; cursor->keys[j] = cursor->keys[j], j--);
        for (int j = cursor->key_num + 1; j > i + 1; cursor->children[j] = cursor->children[j], j--);
        cursor->keys[i] = key;
        cursor->key_num++;
        cursor->children[i + 1] = child;
    }
    else
    {
        InternalNode* newInternal = new InternalNode;
        TemporaryInternalNode temp(cursor);
        int i = 0;
        for (; key > temp.keys[i] && i < MAX_FANOUT - 1; i++);
        temp.ShiftRight(i);
        temp.keys[i] = key;
        temp.children[i + 1] = child;
        cursor->key_num = MAX_FANOUT / 2;
        newInternal->key_num = MAX_FANOUT - 1 - cursor->key_num;
        for (int i = 0, j = cursor->key_num + 1; i < newInternal->key_num; newInternal->keys[i] = temp.keys[j], i++, j++);
        for (int i = 0, j = cursor->key_num + 1; i < newInternal->key_num + 1; newInternal->children[i] = temp.children[j], i++, j++);

        if (cursor == root)
        {
            InternalNode* newRoot = new InternalNode;
            newRoot->keys[0] = cursor->keys[cursor->key_num];
            newRoot->children[0] = cursor;
            newRoot->children[1] = newInternal;
            newRoot->key_num = 1;
            root = newRoot;
        }
        else PlaceParent(cursor->keys[cursor->key_num], FindParent(root, cursor), newInternal);

    }
}

InternalNode* BPlusTree::FindParent(Node* cursor, Node* child)
{
    InternalNode* parent = NULL;
    if (cursor->is_leaf) return NULL;
    InternalNode* intCursor = (InternalNode*)cursor;
    if (intCursor->children[0]->is_leaf) return NULL;
    for (int i = 0; i < intCursor->key_num + 1; i++)
    {
        if (intCursor->children[i] == child)
        {
            parent = intCursor;
            return parent;
        }
        else
        {
            parent = FindParent(intCursor->children[i], child);
            if (parent != NULL)
                return parent;
        }
    }
    return parent;
}

Node* BPlusTree::Traverse(const KeyType& key, Node* node)
{
    for (int i = 0; i < node->key_num; i++)
    {
        InternalNode* cr = (InternalNode*)node;
        if (key < cr->keys[i])
        {
            node = cr->children[i];
            break;
        }
        if (i == cr->key_num - 1)
        {
            node = cr->children[i + 1];
            break;
        }
    }
    return node;
}

LeafNode* BPlusTree::Find(const KeyType& key, int& keyIndex)
{
    if (!IsEmpty())
    {
        Node* cursor = root;
        while (!cursor->is_leaf)
            cursor = Traverse(key, cursor);

        LeafNode* leaf = (LeafNode*)cursor;
        for (int i = 0; i < leaf->key_num; i++)
            if (leaf->keys[i] == key)
            {
                keyIndex = i;
                return leaf;
            }
    }
    keyIndex = -1;
    return NULL;
}

void BPlusTree::DeleteLeaf(LeafNode* leaf)
{
    if (leaf != NULL)
    {
        delete[] leaf->keys;
        delete[] leaf->pointers;
        leaf->next_leaf = leaf->prev_leaf = NULL;
        delete leaf;
    }
}

void BPlusTree::DeleteInternalNode(InternalNode* intNode)
{
    delete[] intNode->keys;
    delete[] intNode->children;
    delete intNode;
}

void BPlusTree::Merge(const KeyType& key, InternalNode* cursor, Node* child)
{
    if (cursor == root) {
        if (cursor->key_num == 1) {
            if (cursor->children[1] == child) {
                if (child->is_leaf)
                    DeleteLeaf((LeafNode*)child);
                else
                    DeleteInternalNode((InternalNode*)child);
                root = cursor->children[0];
                DeleteInternalNode(cursor);
                return;
            }
            else if (cursor->children[0] == child) {
                if (child->is_leaf)
                    DeleteLeaf((LeafNode*)child);
                else
                    DeleteInternalNode((InternalNode*)child);
                root = cursor->children[1];
                DeleteInternalNode(cursor);
                return;
            }
        }
    }
    int index;
    for (index = 0; index < cursor->key_num; index++)
        if (cursor->keys[index] == key)
            break;

    for (int i = index; i < cursor->key_num; i++)
        cursor->keys[i] = cursor->keys[i + 1];

    for (index = 0; index < cursor->key_num + 1; index++)
        if (cursor->children[index] == child)
            break;

    for (int i = index; i < cursor->key_num + 1; i++)
        cursor->children[i] = cursor->children[i + 1];

    cursor->key_num--;
    if (cursor->key_num >= MAX_FANOUT / 2 - 1 || cursor == root)
        return;
    InternalNode* parent = FindParent(root, cursor);
    int leftIndex, rightIndex;
    for (index = 0; index < parent->key_num + 1; index++) {
        if (parent->children[index] == cursor) {
            leftIndex = index - 1;
            rightIndex = index + 1;
            break;
        }
    }
    if (leftIndex >= 0) {
        InternalNode* leftNode = (InternalNode*)parent->children[leftIndex];
        if (leftNode->key_num >= MAX_FANOUT / 2) {
            for (int i = cursor->key_num; i > 0; i--) {
                cursor->keys[i] = cursor->keys[i - 1];
            }
            cursor->keys[0] = parent->keys[leftIndex];
            parent->keys[leftIndex] = leftNode->keys[leftNode->key_num - 1];
            for (int i = cursor->key_num + 1; i > 0; i--) {
                cursor->children[i] = cursor->children[i - 1];
            }
            cursor->children[0] = leftNode->children[leftNode->key_num];
            cursor->key_num++;
            leftNode->key_num--;
            return;
        }
    }
    if (rightIndex <= parent->key_num) {
        InternalNode* rightNode = (InternalNode*)parent->children[rightIndex];
        if (rightNode->key_num >= MAX_FANOUT / 2) {
            cursor->keys[cursor->key_num] = parent->keys[index];
            parent->keys[index] = rightNode->keys[0];
            for (int i = 0; i < rightNode->key_num - 1; i++) {
                rightNode->keys[i] = rightNode->keys[i + 1];
            }
            cursor->children[cursor->key_num + 1] = rightNode->children[0];
            for (int i = 0; i < rightNode->key_num; ++i) {
                rightNode->children[i] = rightNode->children[i + 1];
            }
            cursor->key_num++;
            rightNode->key_num--;
            return;
        }
    }
    if (leftIndex >= 0) {
        InternalNode* leftNode = (InternalNode*)parent->children[leftIndex];
        leftNode->keys[leftNode->key_num] = parent->keys[leftIndex];
        for (int i = leftNode->key_num + 1, j = 0; j < cursor->key_num; j++)
            leftNode->keys[i] = cursor->keys[j];

        for (int i = leftNode->key_num + 1, j = 0; j < cursor->key_num + 1; j++) {
            leftNode->children[i] = cursor->children[j];
            cursor->children[j] = NULL;
        }
        leftNode->key_num += cursor->key_num + 1;
        cursor->key_num = 0;
        Merge(parent->keys[leftIndex], parent, cursor);
    }
    else if (rightIndex <= parent->key_num) {
        InternalNode* rightNode = (InternalNode*)parent->children[rightIndex];
        cursor->keys[cursor->key_num] = parent->keys[rightIndex - 1];
        for (int i = cursor->key_num + 1, j = 0; j < rightNode->key_num; j++) {
            cursor->keys[i] = rightNode->keys[j];
        }
        for (int i = cursor->key_num + 1, j = 0; j < rightNode->key_num + 1; j++) {
            cursor->children[i] = rightNode->children[j];
            rightNode->children[j] = NULL;
        }
        cursor->key_num += rightNode->key_num + 1;
        rightNode->key_num = 0;
        Merge(parent->keys[rightIndex - 1], parent, rightNode);
    }
}

/*
 * Helper function to decide whether current b+tree is empty
 */
bool BPlusTree::IsEmpty() const { return root == NULL; }

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
 /*
  * Return the only value that associated with input key
  * This method is used for point query
  * @return : true means key exists
  */
bool BPlusTree::GetValue(const KeyType& key, RecordPointer& result) 
{
    int keyIndex = -1;
    LeafNode* resultLeaf = Find(key, keyIndex);
    if (resultLeaf != NULL && keyIndex >= 0)
        result = resultLeaf->pointers[keyIndex];
    return false;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
 /*
  * Insert constant key & value pair into b+ tree
  * If current tree is empty, start new tree, otherwise insert into leaf Node.
  * @return: since we only support unique key, if user try to insert duplicate
  * keys return false, otherwise return true.
  */
bool BPlusTree::Insert(const KeyType& key, const RecordPointer& value) 
{ 
    if (IsEmpty())
    {
        LeafNode *newNode = new LeafNode;
        newNode->keys[0] = key;
        newNode->pointers[0] = value;
        newNode->key_num = 1;
        root = newNode;
    }
    else
    {
        Node *cursor = root;
        InternalNode *parent = NULL;
        while (!cursor->is_leaf)
        {
            parent = (InternalNode*)cursor;
            cursor = Traverse(key, cursor);
        }
        LeafNode *leaf = (LeafNode*)cursor;
        if (leaf->key_num < MAX_FANOUT - 1)
        {
            int i = 0;
            for (; key > leaf->keys[i] && i < leaf->key_num; i++);
            if (leaf->keys[i] != key)
            {
                for (int j = leaf->key_num; j > i; j--) {
                    leaf->keys[j] = leaf->keys[j - 1];
                    leaf->pointers[j] = leaf->pointers[j - 1];
                }
                leaf->keys[i] = key;
                leaf->pointers[i] = value;
                leaf->key_num++;
            }
            else return false;
        }
        else
        {
            LeafNode* newLeaf = new LeafNode;
            TemporaryLeafNode temp(leaf);
            int i = 0;
            for (; key > temp.keys[i] && i < MAX_FANOUT - 1; i++);
            if (key != temp.keys[i])
            {
                temp.ShiftRight(i);
                temp.keys[i] = key;
                temp.recordPointers[i] = value;
                leaf->key_num = MAX_FANOUT / 2;
                newLeaf->key_num = MAX_FANOUT - leaf->key_num;
                leaf->next_leaf = newLeaf;
                newLeaf->prev_leaf = leaf;
                for (i = 0; i < leaf->key_num; leaf->keys[i] = temp.keys[i],
                    leaf->pointers[i] = temp.recordPointers[i], i++);
                for (int i = 0, j = leaf->key_num; i < newLeaf->key_num; newLeaf->keys[i] = temp.keys[j],
                    newLeaf->pointers[i] = temp.recordPointers[j], i++, j++);
                if (cursor == root)
                {
                    InternalNode* newRoot = new InternalNode;
                    newRoot->keys[0] = newLeaf->keys[0];
                    newRoot->children[0] = leaf;
                    newRoot->children[1] = newLeaf;
                    newRoot->key_num = 1;
                    root = newRoot;
                }
                else PlaceParent(newLeaf->keys[0], parent, newLeaf);
            }
            else return false;
        }
    }
    return true;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
 /*
  * Delete key & value pair associated with input key
  * If current tree is empty, return immdiately.
  * If not, User needs to first find the right leaf node as deletion target, then
  * delete entry from leaf node. 
  */
void BPlusTree::Remove(const KeyType& key) 
{
    if (IsEmpty()) return;
    Node* cursor = root;
    InternalNode* parent = NULL;
    int leftNeighbourKeyIndex, rightNeighbourKeyIndex;
    while (!cursor->is_leaf)
    {
        for (int i = 0; i < cursor->key_num; i++)
        {
            parent = (InternalNode*)cursor;
            leftNeighbourKeyIndex = i - 1;
            rightNeighbourKeyIndex = i + 1;
            if (key < cursor->keys[i])
            {
                cursor = parent->children[i];
                break;
            }
            if (i == cursor->key_num - 1)
            {
                leftNeighbourKeyIndex = i;
                rightNeighbourKeyIndex = i + 2;
                cursor = parent->children[i + 1];
                break;
            }
        }
    }
    bool found = false;
    int itemIndex = 0;
    for (; itemIndex < cursor->key_num; itemIndex++)
    {
        if (cursor->keys[itemIndex] == key)
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        LeafNode* leaf = (LeafNode*)cursor;
        for (int i = itemIndex; i < leaf->key_num; i++)
        {
            leaf->keys[i] = leaf->keys[i + 1];
            leaf->pointers[i] = leaf->pointers[i + 1];
        }
        leaf->key_num--;


        if (cursor == root)
        {
            if (!cursor->key_num)
            {
                // tree removed completely
                DeleteLeaf(leaf);
                root = NULL;
            }
        }
        else if(cursor->key_num < MAX_FANOUT / 2)
        {
            if (leftNeighbourKeyIndex >= 0)
            {
                LeafNode* left = (LeafNode*)parent->children[leftNeighbourKeyIndex];// or leaf->prev_leaf
                if (left->key_num >= 1 + MAX_FANOUT / 2)
                {
                    for (int i = leaf->key_num; i > 0; leaf->keys[i] = leaf->keys[i - 1],
                        leaf->pointers[i] = leaf->pointers[i - 1], i--);
                    leaf->key_num++;
                    leaf->keys[0] = left->keys[left->key_num - 1];
                    left->pointers[0] = left->pointers[left->key_num - 1];
                    left->key_num--;
                    parent->keys[leftNeighbourKeyIndex] = leaf->keys[0];
                    return;
                }
            }
            if (rightNeighbourKeyIndex <= parent->key_num)
            {
                LeafNode* right = (LeafNode*)parent->children[rightNeighbourKeyIndex];//or leaf->next_leaf
                if (right->key_num >= 1 + MAX_FANOUT / 2)
                {
                    leaf->keys[leaf->key_num] = right->keys[0];
                    leaf->pointers[leaf->key_num++] = right->pointers[0];
                    right->key_num--;
                    for (int i = 0; i < right->key_num; right->keys[i] = right->keys[i + 1],
                        right->pointers[i] = right->pointers[i + 1], i++);
                    parent->keys[rightNeighbourKeyIndex - 1] = right->keys[0];
                    return;
                }
            }
            // if not:
            if (leftNeighbourKeyIndex >= 0)
            {
                LeafNode* left = (LeafNode*)parent->children[leftNeighbourKeyIndex];// or leaf->prev_leaf
                for (int i = left->key_num, j = 0; j < leaf->key_num; left->keys[i] = leaf->keys[j],
                    left->pointers[i] = leaf->pointers[j], i++, j++);

                left->key_num += leaf->key_num;
                if(leaf->next_leaf)
                    leaf->next_leaf->prev_leaf = left;
                left->next_leaf = leaf->next_leaf;
                Merge(parent->keys[leftNeighbourKeyIndex], parent, leaf);
                //DeleteLeaf(leaf);
            }
            else if (rightNeighbourKeyIndex <= parent->key_num)
            {
                LeafNode* right = (LeafNode*)parent->children[rightNeighbourKeyIndex];//or leaf->next_leaf
                for (int i = leaf->key_num, j = 0; j < right->key_num; leaf->keys[i] = right->keys[j],
                    leaf->pointers[i] = right->pointers[j], i++, j++);
                leaf->key_num += right->key_num;
                if(right->next_leaf != NULL)
                    right->next_leaf->prev_leaf = leaf;
                leaf->next_leaf = right->next_leaf;
                Merge(parent->keys[rightNeighbourKeyIndex - 1], parent, right);
                //DeleteLeaf(right);
            }
        }
    }
}

/*****************************************************************************
 * RANGE_SCAN
 *****************************************************************************/
 /*
  * Return the values that within the given key range
  * First find the node large or equal to the key_start, then traverse the leaf
  * nodes until meet the key_end position, fetch all the records.
  */
void BPlusTree::RangeScan(const KeyType& key_start, const KeyType& key_end,
    std::vector<RecordPointer>& result) 
{
    int keyIndex = -1;
    LeafNode* leafCursor = NULL;
    for (int i = 0; i <= key_end && (leafCursor == NULL || keyIndex < 0); i++)
    leafCursor = Find(i, keyIndex);
    if (leafCursor != NULL && keyIndex >= 0)
    {
        for(int i = keyIndex; i < leafCursor->key_num; i++)
            result.push_back(leafCursor->pointers[keyIndex]);
        leafCursor = leafCursor->next_leaf;
        while (leafCursor != NULL)
        {
            for (int i = 0; i < leafCursor->key_num && leafCursor->keys[i] <= key_end; i++)
                result.push_back(leafCursor->pointers[keyIndex]);
            leafCursor = leafCursor->next_leaf;
        }
    }
}
