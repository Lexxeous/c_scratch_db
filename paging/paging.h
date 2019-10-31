#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <cmath>
#include <fstream>
#include <iostream>

#include <climits>
#include <stdexcept>
#include <exception>
#include <string>

typedef uint8_t BYTE;

#define PAGE_SIZE (16 * 1024)
//#define PG_NUM_RECORDS_PTR(page_offset) ((unsigned short *) (((BYTE *)
//page_offset + PAGE_SIZE) - 2 * sizeof(unsigned short)))
#define PG_NUM_RECORDS_PTR(page_offset)                                        \
  ((unsigned short *)&(((::Page::Page_t *)page_offset)->dir_size))
//#define PG_FREE_BYTES_PTR(page_offset) ((unsigned short *) (((BYTE *)
//page_offset + PAGE_SIZE) - sizeof(unsigned short)))
#define PG_FREE_BYTES_PTR(page_offset)                                         \
  ((unsigned short *)&(((Page_t *)page_offset)->free_bytes))

#define PG_DIRECTORY(page_buf)                                                 \
  ((uint16_t *)(reinterpret_cast<BYTE *>(                                      \
                    &((Page::Page_t *)page_buf)->dir_size) -                   \
                (((Page::Page_t *)page_buf)->dir_size * sizeof(uint16_t))))
#define PG_DIRECTORY_BYTES(page_offset)                                        \
  (((BYTE *)page_offset + PAGE_SIZE) - PG_DIRECTORY(page_offset))

// for now:

namespace Page {
  class paging_error : public std::runtime_error {
  public:
    paging_error(std::string what) : std::runtime_error(what) {}
    paging_error(const char *what) : std::runtime_error(what) {}
  };
  class record_error : public std::runtime_error {
  public:
    record_error(std::string what) : std::runtime_error(what) {}
    record_error(const char *what) : std::runtime_error(what) {}
  };

  //  TODO: add page header to each function below.

  // page:  |rec1|rec2|rec3}...}|rec_directory|rd_size|bytes_free|
  // A record is: |size|rest of record|
  //
  // The record directory is at the end of the page.
  //   The very last unsigned short is the number of bytes free in the page
  //   The next to last unsigned short is the number of entries in the record
  //   directory. The very "first" record (not by offset, but ordinally) in the
  //   page directory is
  //       rec_offset_t *rec_directory = ((BYTE *) page + PAGE_SIZE) -
  //                                         (num_entries *
  //                                         sizeof(rec_offset_t)) - 2 *
  //                                         sizeof(unsigned short));
  //       int first_page = rec_directory[0].offset;
  // record directory: |rec_des|rec_des|...|num_records|
  // num_records is an unsigned short.

  struct record_t {
    unsigned short size;
    // rest is data
    BYTE data[1]; // does a record have at least one byte of data?  no
                  // necessarily
  };

  struct Page_t {
    BYTE data[PAGE_SIZE -
              2 * sizeof(unsigned short)]; // includes data + directory
    unsigned short dir_size;
    unsigned short free_bytes;
  }__attribute__((packed));

  struct Page_header_t {
    char sig[4];
    unsigned short num_pages;
    BYTE rest[PAGE_SIZE - 4 - sizeof(unsigned short)];
  };

  const uint16_t PG_REC_UNUSED = USHRT_MAX;
  const unsigned short PG_INITIAL_BYTES =
      PAGE_SIZE - 2 * sizeof(unsigned short);
  typedef unsigned short rec_offset_t;

  bool pg_is_empty(void *page);
  void pg_compact(void *page_buf, uint16_t num_bytes, BYTE *start);
  void pg_pushdown(void *page_buf, uint16_t num_bytes, BYTE *start);
  void *rec_get_ref(void *page, uint16_t rec_id, uint16_t &size);
  BYTE *rec_make_copy(void *page, BYTE rec_id, void *rec_buffer,
                      uint16_t &size);
  uint16_t pg_add_record(void *page, void *record, uint16_t reclen);
  void pg_del_record(void *page, unsigned short rec_id);
  void pg_fixup_directory(void *page, uint16_t rec_id, uint16_t offset);
  int pg_modify_record(void *page, void *record, uint16_t rec_id);

  BYTE *next_record(void *page, uint16_t &next);

  const BYTE RTYPE_NULL = 0;
  const BYTE RTYPE_SHORT = 2;
  const BYTE RTYPE_INT = 4;
  const BYTE RTYPE_DOUBLE = 8;
  const BYTE RTYPE_STRING = 9;

  void rec_packint(std::string &buf, int val);
  void rec_packshort(std::string &buf, int16_t val);
  void rec_packstr(std::string &buf, const std::string &str);
  int rec_upackint(void *buf, unsigned short &next);
  int16_t rec_upackshort(void *buf, unsigned short &next);
  int rec_upackstr(void *buf, unsigned short &next, std::string &val);
  void rec_finish(std::string &buf);
  void rec_begin(std::string &buf);
}; // namespace Page

std::ostream &operator<<(std::ostream &os, const Page::record_t &rec);
std::ostream &operator<<(std::ostream &os, const Page::Page_t &page);

typedef std::fstream file_descriptor_t;
namespace Page_file {
  // Format page file.
  // |page file header|page|page|...
  //
  //                       Page 0
  // page file header:  |sig+first_free+reserved|  ...    |
  //

  struct page_free_t {
    uint16_t size;
    uint16_t free[PAGE_SIZE/sizeof(uint16_t) - 1];
  };
  
  const uint16_t PGF_PAGES_FREE_ID = 3;

  void print(const char fname[]);
  void print(file_descriptor_t &pfile);

  void pgf_format(const char fname[200], uint16_t fsize);

  // Write a page in memory to disk.
  void pgf_write(file_descriptor_t &pfile, int page_id, void *page_buf);
  // Read a page from disk to memory
  void pgf_read(file_descriptor_t &pfile, int page_id, void *page_buf);

  // return the page number of a page of size 0
  int pgf_find_free(file_descriptor_t &pfile);

}; // namespace Page_file

#endif // PAGING_H
