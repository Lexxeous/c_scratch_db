#include "paging.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

/* Note: write a function that prints a page file:
   page file header (0)
      ...
   page 1
      record 0   int: 24 string: Hello world
      record 1   int: 11 string: This is a test
      ...
   page 3:
      ...
   END
*/

std::ostream &operator<<(std::ostream &os, const Page::record_t &rec) {
  const BYTE *where = rec.data;
  os << "size = " << rec.size << ", ";
  uint16_t totsize = rec.size;
  totsize -= sizeof(uint16_t); // count size
  while (totsize) {
    uint16_t type = *(uint16_t *)where;
    uint16_t size = *(uint16_t *)where;
    const BYTE *data = where + sizeof(uint16_t);
    switch (type) {
    case Page::RTYPE_NULL:
      os << "NULL";
      break;
    case Page::RTYPE_SHORT:
      os << "int16 (" << *((uint16_t *)data) << ")";
      break;
    case Page::RTYPE_INT:
      os << "int32 (" << *((uint32_t *)data) << ")";
      break;
    case Page::RTYPE_DOUBLE:
      os << "double64 (" << *((double *)data) << ")";
      break;
    default:
      if (type > Page::RTYPE_STRING) {
        size -= Page::RTYPE_STRING;
        std::string s((char *)data, size);
        os << "String" << size << "(" << s << ")";
      }
    }
    totsize -= sizeof(uint16_t);      // count type
    totsize -= size;                  // cout data
    where += sizeof(uint16_t) + size; // advance passed type spec and data
    // os << ", " << totsize << " bytes left ... ";
    if (totsize > 0) {
      os << ", ";
    }
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const Page::Page_t &page) {
  os << "Bytes free = " << page.free_bytes << ", Entries = " << page.dir_size
     << ", records: ";
  uint16_t *dirarray = PG_DIRECTORY(&page);
  std::vector<uint16_t> dir(dirarray, dirarray + page.dir_size);
  int id = 0;
  for (uint16_t entry : dir) {
    if (entry == Page::PG_REC_UNUSED) {
      os << std::endl << "   id = " << id++ << ", UNUSED";
    } else {
      Page::record_t *rec = (Page::record_t *)(((BYTE *)&page) + entry);
      os << std::endl << "   id = " << id++ << ", " << *rec;
    }
  }
  return os;
}

void Page_file::print(file_descriptor_t &pfile) {
  pfile.seekg(0, std::ios_base::beg);
  Page::Page_header_t ph;
  pgf_read(pfile, 0, &ph);

  Page::Page_t page_buf;
  bool empty_run = false;
  for (uint16_t pageid = 1; pageid < ph.num_pages; pageid++) {
    pgf_read(pfile, pageid, &page_buf);
    if (page_buf.dir_size == 0) {
      if (empty_run) {
        if (pageid == ph.num_pages - 1) {
          std::cout << pageid;
          std::cout << " *empty*";
        }
        continue;
      } else {
        std::cout << "PAGE: " << pageid << "-";
        empty_run = true;
      }
    } else {
      if (empty_run) {
        empty_run = false;
        std::cout << pageid - 1;
        std::cout << " *empty*" << std::endl;
      }
      std::cout << "PAGE: " << pageid << ": " << page_buf << " " << std::endl;
    }
  }
  std::cout << std::endl;
}

void Page_file::print(const char fname[]) {
  
  file_descriptor_t pfile(fname, std::ios::in | std::ios::binary);
  if (!pfile) {
    throw Page::paging_error("Cannot open file for printing");
  }

  print(pfile);

  pfile.close();
}

void Page_file::pgf_format(const char fname[200], uint16_t fsize) {
  // page file header:  |sig+first_free+reserved|  ...    |

  if (fsize <= 4) {
    // pages 0, 1, 2, and 3 are reserved
    throw Page::paging_error("Page file must be at least 4 pages");
  }
  std::ofstream ouf(fname, std::ios::out | std::ios::binary);
  if (!ouf) {
    throw Page::paging_error("cannot open page file for writing");
  }

  // fix up page header here
  Page::Page_header_t ph = {{'E', 'A', 'G', 'L'}, fsize};
  memset(              //(BYTE *)&ph + strlen("EAGL") + sizeof(unsigned short)
         ph.rest, 0,
         sizeof(Page::Page_header_t::rest));
  ouf.write((const char *)&ph, sizeof(ph));

  Page::Page_t init;
  memset((void *)&init, 0, sizeof(Page::Page_t));
  init.free_bytes = sizeof(init.data);
  for (int pageno = 1; pageno < fsize; pageno++) {
    ouf.write((const char *)&init, sizeof(init));
  }

  // now, write free page list  |num_pages|pgid|pgid|pgid...|
  // TODO: should do this as a bit map->   is_free(pg_no) = free_page.free[pg_no/8] & (1 << pg_no % 8)
  page_free_t free_page;
  free_page.size = fsize - 4;   // -4 because page 0 (reserved), 1(#master table), 2 (#columns), 3 (free pages)
  for (int pageno = 4, idx = free_page.size-1; idx >= 0; pageno++, idx--) {
    free_page.free[idx] = pageno;
  }
  ouf.seekp(PAGE_SIZE * PGF_PAGES_FREE_ID, std::ios_base::beg);
  ouf.write(reinterpret_cast<char *>(&free_page), sizeof(free_page));
  ouf.close();
}

// Write a page in memory to disk.
void Page_file::pgf_write(file_descriptor_t &pfile, int page_id,
                          void *page_buf) {
  pfile.seekg(PAGE_SIZE * page_id, std::ios_base::beg);
  if (pfile.fail() || pfile.bad()) {
    throw std::runtime_error("Cannot seek to page");
  }
  pfile.write(reinterpret_cast<char *>(page_buf), PAGE_SIZE);
  if (pfile.fail() || pfile.bad()) {
    throw std::runtime_error("Cannot write to page");
  }
}
// Read a page from disk to memory
void Page_file::pgf_read(file_descriptor_t &pfile, int page_id,
                         void *page_buf) {
  pfile.seekg(PAGE_SIZE * page_id, std::ios_base::beg);
  // pfile.clear();
  // std::cout << pfile.tellg() << std::endl;

  if (pfile.fail()) {
    throw std::runtime_error("SEEKFAIL: Cannot seek to page");
  }

  if (pfile.bad()) {
    throw std::runtime_error("SEEKBAD: Cannot seek to page");
  }

  pfile.read(reinterpret_cast<char *>(page_buf), PAGE_SIZE);
  if (pfile.fail() || pfile.bad()) {
    throw std::runtime_error("Cannot read page");
  }
}

// return the page number of a page of size 0
int Page_file::pgf_find_free(file_descriptor_t &pfile) { return 0; }

bool Page::pg_is_empty(void *page) { return false; }
// return (*PG_NUM_RECORDS_PTR(page) == 0)

// The functions below manipulate pages in memory:

void Page::pg_compact(void *page_buf, uint16_t num_bytes,
                      BYTE *start) { //, rec_offset_t *rec_directory);
  uint16_t *pgdirectory = PG_DIRECTORY(page_buf);
  memmove(start - num_bytes, start, ((BYTE *)pgdirectory) - start);
}

// Throws an exceptin if there is not enough room in the page to make room
void Page::pg_pushdown(void *page_buf, uint16_t num_bytes,
                       BYTE *start) { //, rec_offset_t *rec_directory);
  uint16_t bytes_free = *PG_FREE_BYTES_PTR(page_buf);
  uint16_t howmany = (((BYTE *)PG_DIRECTORY(page_buf)) - bytes_free) - start;
  if (bytes_free < num_bytes)
    throw paging_error("Not enough free space in page");
  // rec_offset_t *pgdirectory =
  //    reinterpret_cast<rec_offset_t *>(PG_DIRECTORY(page_buf));
  memmove(start + num_bytes, start, howmany);
}

// Returns a pointer to the record in memory.
void *Page::rec_get_ref(void *page, uint16_t rec_id, uint16_t &size) {
  record_t *start = (record_t *)(reinterpret_cast<rec_offset_t *>(
                                     PG_DIRECTORY(page))[rec_id] +
                                 reinterpret_cast<char *>(page));
  size = start->size;
  return start;
}

// Returns number of bytes left if whole record is not copied.  Throws exception
// if size > the record len.
BYTE *Page::rec_make_copy(void *page, BYTE rec_id, void *rec_buffer,
                          uint16_t &size) {
  BYTE *offset = reinterpret_cast<BYTE *>(rec_get_ref(page, rec_id, size));
  BYTE *buf = new BYTE[size];
  memcpy(buf, offset, size);
  return (BYTE *)rec_buffer;
}

uint16_t
Page::pg_add_record(void *page, void *record,
                    uint16_t reclen) { //, rec_offset_t *rec_directory);
  // Prereq: a page with enough space has been found, and the page variable
  // points to it.
  //        also record variable points to a properly formatted record built
  //        with the rec_pack functions.
  // Note: throws exception if record cannot be added because it is too long.
  //      Remember to remove from free list if page was previously empty.
  Page_t *pgptr = reinterpret_cast<Page_t *>(page);
  if (pgptr->free_bytes < reclen)
    throw paging_error("Not enoough free space in page to add record");
  BYTE *diraddr = (BYTE *)PG_DIRECTORY(page);
  uint16_t *dest = (uint16_t *)(diraddr - pgptr->free_bytes);
  memcpy(dest, record, ((record_t *)record)->size);
  pgptr->free_bytes -= reclen;

  // Now, update record directory
  rec_offset_t *pgdir = reinterpret_cast<rec_offset_t *>(PG_DIRECTORY(page));
  // bad, expensive cheat below :
  unsigned short *num_records_ptr = PG_NUM_RECORDS_PTR(page);

  // Copy the directory to this vector that
  //  will be used to add the new offset
  std::vector<unsigned short> dir(pgdir, pgdir + *num_records_ptr);

  // add the record offset to the end of the copied directory
  dir.push_back((BYTE *)dest - (BYTE *)page); 

  pgdir--; // Back up the pointer to the page directory because the it is larger
           // by one.

  pgptr->free_bytes -= sizeof(uint16_t); // new directory entry takes 2 bytes
  memcpy(pgdir, dir.data(),
         dir.size() *
             sizeof(uint16_t)); // Replace the old directory with the new one
  uint16_t retval = *num_records_ptr; // The added record id is the last, so we
                                      // will return this index
  (*num_records_ptr)++; // Increase the num_records count in the page by one.
  return retval;
}

void Page::pg_del_record(void *page, unsigned short rec_id) {
  // First get a reference to the record using the directory:
  rec_offset_t *pgdir = reinterpret_cast<rec_offset_t *>(PG_DIRECTORY(page));
  uint16_t size;
  record_t *rec = (record_t *)(rec_get_ref(page, rec_id, size));
  size = rec->size;

  // Then, update the record directory by fixing up the offsets
  pg_fixup_directory(page, rec_id, 0 - rec->size);

  // DO NOT DELETE the entry in the page directory!
  //  Index references will be messed up.

  // flag that the directory entry is unused
  pgdir[rec_id] = PG_REC_UNUSED;

  // Lastly, compact the page and update free bytes:
  pg_compact(page, size, ((BYTE *)rec) + rec->size);

  ((Page_t *)page)->free_bytes += size;
}

void Page::pg_fixup_directory(void *page, uint16_t rec_id, uint16_t offset) {
  uint16_t num_records = *PG_NUM_RECORDS_PTR(page);
  rec_offset_t *pgdir = reinterpret_cast<rec_offset_t *>(PG_DIRECTORY(page));
  for (uint16_t id = 0; id < num_records; id++) {
    if (pgdir[id] != PG_REC_UNUSED && pgdir[id] > pgdir[rec_id]) {
      pgdir[id] += offset;
    }
  }
}

int Page::pg_modify_record(void *page, void *record, uint16_t rec_id) {
  //  // Return > 0 if record is now too big and must to moved
  //       to another page.
  Page_t *pg = static_cast<Page_t *>(page);
  uint16_t rec_len = ((record_t *)record)->size;
  uint16_t *fbptr = &(pg->free_bytes);
  uint16_t *pgdir = PG_DIRECTORY(page);
  uint16_t offset = pgdir[rec_id];
  record_t *old_rec = reinterpret_cast<record_t *>((BYTE *)page + offset);
  if (rec_id > *PG_NUM_RECORDS_PTR(page))
    throw paging_error("page id out of range");
  if (rec_len - old_rec->size > *fbptr)
    return rec_len - old_rec->size;
  if (rec_len == old_rec->size) {
    // copy the record to the page
    memcpy(old_rec, record, rec_len);
    return 0;
  } else if (rec_len < old_rec->size) {
    unsigned short dif = old_rec->size - rec_len;
    pg_compact(page, old_rec->size - rec_len,
               ((BYTE *)old_rec) + old_rec->size);
    // copy the record to the page and compact
    memcpy(old_rec, record, rec_len);
    // Then, update the record directory by fixing up the offsets
    pg_fixup_directory(page, rec_id, 0 - dif);
  } else {
    // push_down the record after this one the required number of bytes
    unsigned short dif = rec_len - old_rec->size;
    pg_pushdown(page, dif, ((BYTE *)old_rec) + old_rec->size);
    // copy the record to the page
    memcpy(old_rec, record, rec_len);
    // Then, update the record directory by fixing up the offsets
    pg_fixup_directory(page, rec_id, dif);
  }
  return rec_len;
}

BYTE *Page::next_record(void *page, uint16_t &next) {
  if (next >= *PG_NUM_RECORDS_PTR(page)) {
    return nullptr;
  }
  uint16_t *dir = PG_DIRECTORY(page);

  BYTE *where = (BYTE *) page + dir[next];
  next++;
  return where;
}

// Note, the two pack functions throw an exception if packing the item results
//   in a record that is greater than the maximum allowable record length.

// 0 NULL
// 2 short
// 4 int
// 8 double
// 9 or more is a string of size (type_id - 9)

void Page::rec_packint(std::string &buf, int val) {
  size_t old_size = buf.size();
  buf.resize(old_size + sizeof(uint16_t) + sizeof(int));
  char *bufdata = const_cast<char *>(buf.data());
  *(uint16_t *)(bufdata + old_size) = RTYPE_INT;
  *(int *)(bufdata + old_size + sizeof(uint16_t)) = val;
}


void Page::rec_packshort(std::string &buf, int16_t val) {
  size_t old_size = buf.size();
  buf.resize(old_size + sizeof(uint16_t) + sizeof(uint16_t));
  char *bufdata = const_cast<char *>(buf.data());
  *(uint16_t *)(bufdata + old_size) = RTYPE_SHORT;
  *(int16_t *)(bufdata + old_size + sizeof(uint16_t)) = val;
}

// Throw an exception if next item if greater than max allowable string length
void Page::rec_packstr(std::string &buf, const std::string &str) {
  size_t old_size = buf.size();
  buf.resize(old_size + sizeof(uint16_t) + str.size());
  char *bufdata = const_cast<char *>(buf.data());
  *(uint16_t *)(bufdata + old_size) = RTYPE_STRING + str.size();
  memcpy(bufdata + old_size + sizeof(uint16_t), str.data(), str.size());
}

// Throws exception if next item is not an int
int Page::rec_upackint(void *buf, unsigned short &next) {
  // int val;
  BYTE *where = reinterpret_cast<BYTE *>(buf) + next;
  uint16_t type = *(uint16_t *)where;
  if (type != RTYPE_INT)
    throw record_error("cannot convert type to an integer");
  int retval = *(int *)((BYTE *)buf + next + sizeof(uint16_t));
  next += RTYPE_INT + sizeof(uint16_t);
  return retval;
}


int16_t Page::rec_upackshort(void *buf, unsigned short &next) {
  BYTE *where = reinterpret_cast<BYTE *>(buf) + next;
  uint16_t type = *(uint16_t *)where;
  if (type != RTYPE_SHORT)
    throw record_error("cannot convert type to a short");
  int16_t retval = *(short *)((BYTE *)buf + next + sizeof(uint16_t));
  next += RTYPE_SHORT + sizeof(uint16_t);
  return retval;
}

// Throws exception if next item is not a string
int Page::rec_upackstr(void *buf, unsigned short &next, std::string &val) {
  BYTE *where = reinterpret_cast<BYTE *>(buf) + next;
  uint16_t type = *(uint16_t *) where;
  if (type < RTYPE_STRING)
    throw record_error("cannot convert type to a string");
  val.assign(reinterpret_cast<char *>(where + sizeof(uint16_t)), type - RTYPE_STRING);
  next += val.size() + sizeof(uint16_t);
  return val.size();
}

void Page::rec_finish(std::string &buf) { // should set the size
  uint16_t *bufdata = (uint16_t *)const_cast<char *>(buf.data());
  *bufdata = buf.size(); // Remember, rec_begin should have
                         // alread added bytes to the front
}


void Page::rec_begin(std::string &buf) {
  // should resize to add place for size at front
  buf.resize(sizeof(uint16_t));
}
