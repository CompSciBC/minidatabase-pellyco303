#ifndef ENGINE_H
#define ENGINE_H
#include<string>
#include <vector>  
#include <iostream>  
#include "Record.h"
#include "BST.h"     

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

struct Engine {
    vector<Record> heap;               // the main data store 
    BST<int, int> idIndex;                // index by student ID
    BST<string, vector<int>> lastIndex;   // index by last name 

    int theIndex = 0;
    int newestRecord; // return most recently added record

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record& recIn) {
        heap.push_back(recIn);

        // update the idIndex bst
        idIndex.insert(recIn.id, theIndex);

        //search lastIndex to see if a node already exists with that last name
        if (lastIndex.find(toLower(recIn.last)) != nullptr) { //someone else has the same last name
            vector<int>* namesIndices = lastIndex.find(toLower(recIn.last));
            namesIndices->push_back(theIndex); // update the vector/value of the node
            int lenOf = lastIndex.find(toLower(recIn.last))->size();
        }else{ //add a new node to lastIndex bst
            vector<int> namesIndices;
            namesIndices.push_back(theIndex);
            lastIndex.insert(toLower(recIn.last), namesIndices);
        }
        newestRecord = recIn.id; //update newestRecord
        theIndex++;
        return recIn.id;
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id) {
        if (!idIndex.find(id)) { // id not in the database
            return false;
        } else {
            int idx = *(idIndex.find(id));
            string theLastName = heap[idx].last; // last name of the student
            heap [idx].deleted = true; // mark student record as 'deleted'
            // update idIndex
            idIndex.erase(id); 
            // update lastIndex
            vector<int>* lastNames = lastIndex.find(toLower(theLastName)); 
            // if the length of the vector is greater than 1, the last name is not unique
            if (lastNames->size() > 1) {
                for (int i = 0; i < lastNames->size(); i++) {
                    if (lastNames->at(i) == idx) {
                        lastNames->erase(lastNames->begin() + i); // delete the heap's index number from the vector
                    }
                }
            } else {
                lastIndex.erase(toLower(theLastName)); // delete the node from lastIndex bst
            }
            return true;
        }
    }

    // Finds a record by student ID. Returns a pointer to the record, or nullptr if not found. Outputs the number of comparisons made in the search.
    const Record* findById(int id, int& cmpOut) {
        idIndex.resetMetrics();
        // id is not in the database
        if (idIndex.find(id) == nullptr) {
            cmpOut = idIndex.comparisons;
            return nullptr;
        } else {
            idIndex.resetMetrics();
            int idx = *(idIndex.find(id));
            cmpOut = idIndex.comparisons;
            Record* recordPtr = &heap[idx];
            return recordPtr;
        }
    }

    //Returns records with an ID within range
    vector<const Record*> rangeById(int lo, int hi, int& cmpOut) {
        vector<const Record*> v1;
        idIndex.resetMetrics();
        idIndex.rangeApply(lo, hi, [&](const int& id, int& ind) {
            Record* ptr = &heap[ind];
            v1.push_back(ptr);
        });
        cmpOut = idIndex.comparisons;
        return v1;
    }

    // helper for prefixByLast
    bool matches(const string& thePrefix, const string& str) {
        if (str.size() < thePrefix.size()) {
            return false;
        }
        for (int i = 0; i < thePrefix.size(); i++) {
            if (thePrefix[i] != str[i]) {
                return false;
            }
        }
        return true;
    }

    // Returns all records whose last name begins with a given prefix.
   // Case-insensitive using lowercase comparison.
    vector<const Record*> prefixByLast(const string& prefix, int& cmpOut) {
        vector<const Record*> v1;
        string prefixLowerCase = toLower(prefix);
        lastIndex.resetMetrics();

        lastIndex.rangeApply(prefixLowerCase, string(1, prefixLowerCase[0] + 1), [&](const string& k, vector<int>& v) {
            if (matches(prefixLowerCase, k)) {
                for (int i = 0; i < v.size(); i++) {
                    v1.push_back(&heap[v[i]]);
                }
            }
         });
        cmpOut = lastIndex.comparisons;
        return v1;
    }
};
#endif

