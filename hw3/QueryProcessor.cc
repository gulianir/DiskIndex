/*
 * Copyright ©2018 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Autumn Quarter 2018 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <iostream>
#include <algorithm>

#include "./QueryProcessor.h"

extern "C" {
  #include "./libhw1/CSE333.h"
}

namespace hw3 {

QueryProcessor::QueryProcessor(list<string> indexlist, bool validate) {
  // Stash away a copy of the index list.
  indexlist_ = indexlist;
  arraylen_ = indexlist_.size();
  Verify333(arraylen_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[arraylen_];
  itr_array_ = new IndexTableReader *[arraylen_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::iterator idx_iterator = indexlist_.begin();
  for (HWSize_t i = 0; i < arraylen_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = new DocTableReader(fir.GetDocTableReader());
    itr_array_[i] = new IndexTableReader(fir.GetIndexTableReader());
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (HWSize_t i = 0; i < arraylen_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) {
  Verify333(query.size() > 0);
  vector<QueryResult> finalresult;

  // MISSING:
  // set up initial results in finalresult
  GetResults(query[0], finalresult);

  // for every other word, get results and
  // remove results not in the intersection from
  // finalresult. Update ranks of results in intersection.
  for (HWSize_t i = 1; i < query.size(); i++) {
    vector<QueryResult> curr;
    GetResults(query[i], curr);
    SubsetFromFinal(finalresult, curr);
  }

  // Sort the final results.
  std::sort(finalresult.begin(), finalresult.end());
  return finalresult;
}

void QueryProcessor::SubsetFromFinal(vector<QueryResult>& finalresult,
                                     vector<QueryResult>& curr) {
  vector<QueryResult>::iterator it;
  vector<QueryResult>::iterator it2;
  it = finalresult.begin();
  while (it != finalresult.end()) {
    bool found = false;
    for (it2 = curr.begin(); it2 != curr.end(); it2++) {
      if (it->document_name.compare(it2->document_name) == 0) {
        found = true;
        it->rank += it2->rank;
        break;
      }
    }
    if (!found) {
      finalresult.erase(it);
    } else {
      it++;
    }
  }
}

void QueryProcessor::GetResults(const string& word,
                                vector<QueryResult>& results) {
  for (HWSize_t i = 0; i < arraylen_; i++) {
    IndexTableReader* memIndex = itr_array_[i];
    DocTableReader* docTable = dtr_array_[i];

    DocIDTableReader* docIDTable = memIndex->LookupWord(word);
    if (docIDTable == nullptr) {
      continue;
    }
    list<docid_element_header> docids;
    docids = docIDTable->GetDocIDList();

    for (docid_element_header currFile : docids) {
      string docname;
      Verify333(docTable->LookupDocID(currFile.docid, &docname));

      QueryResult qr;
      qr.document_name = docname;
      qr.rank = currFile.num_positions;
      results.push_back(qr);
    }
    delete docIDTable;
  }
}

}  // namespace hw3
