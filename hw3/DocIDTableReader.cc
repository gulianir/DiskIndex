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

#include "./DocIDTableReader.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw3 {

// The constructor for DocIDTableReader calls the constructor
// of HashTableReader(), its superclass. The superclass takes
// care of taking ownership of f and using it to extract and
// cache the number of buckets within the table.
DocIDTableReader::DocIDTableReader(FILE *f, IndexFileOffset_t offset)
  : HashTableReader(f, offset) { }

bool DocIDTableReader::LookupDocID(const DocID_t &docid,
                                   std::list<DocPositionOffset_t> *ret_list) {
  // Use the superclass's "LookupElementPositions" function to
  // walk through the docIDtable and get back a list of offsets
  // to elements in the bucket for this docID.
  auto elements = LookupElementPositions(docid);

  // If the list of elements is empty, we're done.
  if (elements.size() == 0)
    return false;

  // Iterate through the elements, looking for our docID.
  for (auto it = elements.begin(); it != elements.end(); it++) {
    IndexFileOffset_t next_offset = *it;

    // Slurp the next docid out of the element.
    docid_element_header header;
    // MISSING:
    HWSize_t res = fseek(file_, next_offset, SEEK_SET);
    Verify333(res == 0);

    res = fread(&header, sizeof(header), 1, file_);
    Verify333(res == 1);
    header.toHostFormat();

    // Is it a match?
    if (header.docid == docid) {
      // Yes!  Extract the positions themselves, appending to
      // std::list<DocPositionOffset_t>.  Be sure to push in the right
      // order, adding to the end of the list as you extract
      // successive positions.

      // MISSING:
      docid_element_position pos;
      for (HWSize_t i = 0; i < header.num_positions; i++) {
        res = fread(&pos, sizeof(pos), 1, file_);
        Verify333(res == 1);
        pos.toHostFormat();
        ret_list->push_back(pos.position);
      }

      // Return the positions list through the output parameter,
      // and return true.

      // MISSING:
      // ????

      return true;
    }
  }

  // We failed to find a matching docID, so return false.
  return false;
}

list<docid_element_header> DocIDTableReader::GetDocIDList() {
  // This will be our returned list of docIDs within this table.
  list<docid_element_header> docidlist;

  // Go through *all* of the buckets of this hashtable, extracting
  // out the docids in each element and the number of word positions
  // for the each docid.
  for (HWSize_t i = 0; i < header_.num_buckets; i++) {
    // Seek to the next bucket_rec.  The "offset_" member
    // variable stores the offset of this docid table within
    // the index file .
    // MISSING:
    HWSize_t res;
    HWSize_t offset = offset_ + sizeof(bucket_rec)*i + sizeof(header_);
    res = fseek(file_, offset, SEEK_SET);
    Verify333(res == 0);

    // Read in the chain length and bucket position fields from
    // the bucket_rec.
    bucket_rec b_rec;
    // MISSING:
    res = fread(&b_rec, sizeof(b_rec), 1, file_);
    Verify333(res == 1);
    b_rec.toHostFormat();

    // Sweep through the next bucket, iterating through each
    // chain element in the bucket.
    for (HWSize_t j = 0; j < b_rec.chain_len; j++) {
      // Seek to chain element's position field in the bucket header.
      Verify333(fseek(file_,   b_rec.bucket_position
                             + j*sizeof(element_position_rec), SEEK_SET) == 0);

      // Read the next element position from the bucket header.
      element_position_rec  ep;
      // MISSING:
      res = fread(&ep, sizeof(ep), 1, file_);
      Verify333(res == 1);
      ep.toHostFormat();
      // Seek to the element itself.
      // MISSING:
      res = fseek(file_, ep.element_position, SEEK_SET);
      Verify333(res == 0);
      // Read in the docid and number of positions from the element.
      docid_element_header doc_el;
      // MISSING:
      res = fread(&doc_el, sizeof(doc_el), 1, file_);
      Verify333(res == 1);
      doc_el.toHostFormat();

      // Append it to our result list.
      docidlist.push_back(doc_el);
    }
  }

  // Done!  Return the result list.
  return docidlist;
}

}  // namespace hw3
