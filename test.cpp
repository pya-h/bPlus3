//===----------------------------------------------------------------------===//
//
//
// Identification:   test/b_plus_tree_delete_test.cpp
//
//
//===----------------------------------------------------------------------===/

#include "bp3.h"

#include <iostream>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

int main() {
    cout << "B+ Tree Test..." << endl;
    BPlusTree tree;

    bool testResult = true;
    for (int i = 100; i < 500; i++) {
        RecordPointer one_record(i, i);
        if (!tree.Insert(i, one_record)) {
            testResult = false;
        }
    }

    cout << "Insert Test: " << (!testResult || tree.IsEmpty() ? "FAILURE!" : "SUCCESS!") << endl;
    testResult = true;
    for (int i = 100; i < 500; i++) {
        RecordPointer one_record;
        tree.GetValue(i, one_record);
        if (one_record.page_id != i) {
            testResult = false;
        }
    }
    
    cout << "Search Test: " << (!testResult || tree.IsEmpty() ? "FAILURE!" : "SUCCESS!") << endl;

    testResult = true;
    for (int i = 100; i < 500; i += 4)
        tree.Remove(i);

    RecordPointer temp;
    cout << "Remove Test: " << (tree.GetValue(100, temp) || tree.GetValue(204, temp) ? "FAILURE!" : "SUCCESS!") << endl;

    vector<RecordPointer> records;
    tree.RangeScan(100, 112, records);
    cout << "RangeScan Test: " << (records.size() != 9 ? "FAILURE!" : "SUCCESS!") << endl;

    return 0;
}
