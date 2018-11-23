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

#include <stdio.h>   // for fprintf()
#include <string.h>  // for strcmp()

#include "./fileindexutil.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw3 {

// Initialize the "CRC32::table_is_initialized" static member variable.
bool CRC32::table_is_initialized_ = false;

// We need this declaration here so we can refer to the table_ below.
uint32_t CRC32::table_[256];

static uint32_t CRC32Reflect(uint32_t reflectme, const char c) {
  uint32_t val = 0;
  int position;

  // Swap bit 0 for bit 7, bit 1 For bit 6, etc....
  for (position = 1; position < (c + 1); position++) {
    if (reflectme & 1) {
      val |= (1 << (c - position));
    }
    reflectme >>= 1;
  }
  return val;
}

CRC32::CRC32(void) {
  if (!table_is_initialized_) {
    // Yes, there is a potential race condition if multiple threads
    // construct the very first CRC32() objects simultaneously.  We'll
    // live with it. ;)
    Initialize();
    table_is_initialized_ = true;
  }

  // Prep the CRC32 state.
  finalized_ = false;
  crc_state_ = 0xFFFFFFFF;
}

void CRC32::FoldByteIntoCRC(const uint8_t nextbyte) {
  Verify333(finalized_ != true);
  crc_state_ =
    (crc_state_ >> 8) ^ this->table_[(crc_state_ & 0xFF) ^ nextbyte];
}

uint32_t CRC32::GetFinalCRC(void) {
  if (!finalized_) {
    finalized_ = true;
    crc_state_ ^= 0xffffffff;
  }
  return crc_state_;
}

void CRC32::Initialize(void) {
  uint32_t polynomial = 0x04C11DB7;
  uint32_t codes, position;

  // Initialize the CRC32 lookup table; the table's 256 values
  // represent ASCII character codes.
  for (codes = 0; codes <= 0xFF; codes++) {
    CRC32::table_[codes] = CRC32Reflect(codes, 8) << 24;
    for (position = 0; position < 8; position++) {
      table_[codes] =
        (table_[codes] << 1) ^
        ((table_[codes] & (1 << 31)) ? polynomial : 0);
    }
    this->table_[codes] = CRC32Reflect(table_[codes], 32);
  }
}

FILE *FileDup(FILE *f) {
  // Duplicate the underlying file descriptor using dup().
  int newfd = dup(fileno(f));
  Verify333(newfd != -1);

  // Use fdopen() to open a new (FILE *) for the dup()'ed descriptor.
  FILE *retfile = fdopen(newfd, "rb");
  Verify333(retfile != nullptr);

  // Return the new (FILE *).
  return retfile;
}

}  // namespace hw3
