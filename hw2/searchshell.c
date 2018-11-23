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

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600
#define BUFSIZE 1024
#define QUERYSIZE 512

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "memindex.h"
#include "filecrawler.h"

static void Usage(void);

int main(int argc, char **argv) {
  if (argc != 2)
    Usage();

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - crawl from a directory provided by argv[1] to produce and index
  //  - prompt the user for a query and read the query from stdin, in a loop
  //  - split a query into words (check out strtok_r)
  //  - process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  DocTable doct;
  MemIndex memindex;
  char* dirName = argv[1];
  printf("Indexing \'%s\'\n", dirName);
  int crawled = CrawlFileTree(dirName, &doct, &memindex);
  if (crawled == 0) {
    // error
    Usage();
  }
  DocTable* dt = &doct;
  MemIndex* mi = &memindex;

  if (*dt == NULL || *mi == NULL) {
    Usage();
  }

  char buf[BUFSIZE];
  char* savePtr;
  char* word;

  printf("enter query:\n");
  while (fgets(buf, BUFSIZE, stdin) != NULL) {
    if (fgets(buf, BUFSIZE, stdin) != NULL) {
      char** query = (char**) malloc(QUERYSIZE*sizeof(char*));
      if (query == NULL) {
        Usage();
      }
      HWSize_t numQueries = 0;
      char* str = buf;
      while (true) {
        word = strtok_r(str, " ", &savePtr);
        if (word == NULL) {
          break;
        }
        query[numQueries] = word;
        numQueries++;
        str = NULL;
      }
      char* lastIndex = (char*)(query[numQueries-1] + 
                        strlen(query[numQueries-1] -1));
      *lastIndex = '\0';

      LinkedList searchRes = MIProcessQuery(*mi, query, numQueries);
      if (searchRes != NULL) {
        LLIter it = LLMakeIterator(searchRes, 0);
        if (it == NULL) {
          Usage();
        }
        while (LLIteratorHasNext(it)) {
          SearchResult* current;
          LLIteratorGetPayload(it, (LLPayload_t*) &current);
          printf(" %s (%d)\n", DTLookupDocID(*dt, current->docid), current->rank);
        }
        LLIteratorFree(it);
      }
      FreeLinkedList(searchRes, &free);
    }
    FreeDocTable(*dt);
    FreeMemIndex(*mi);
    printf("enter query:\n");
  }


  return EXIT_SUCCESS;
}

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

