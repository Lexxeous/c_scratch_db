// table.h
// A table is a linked list of pages (with head and tail).  Each page has a
// collection of records forthat specific table. Each table has only one linked
// list of pages that contains only that table records, and no other table's
// records.

#ifndef TABLE_H
#define TABLE_H

#include "paging.h"

#include <climits>
#include <cstdint>
#include <string>
#include <vector>

namespace Table_pages {

  const uint16_t TBL_TYPE_VCHAR = 9;
  const uint16_t TBL_TYPE_SHORT = 2;

  const char TBL_MASTER_NAME[] = "#master";
  const uint16_t TBL_MASTER_PAGE = 1;
  // const char *TBL_COLUMN_NAME = "#columns";
  const uint16_t TBL_COLUMNS_PAGE = 2;
  const uint16_t DB_TYPE_TABLE = 1;

  class table_error : public std::runtime_error {
  public:
    table_error(std::string what) : std::runtime_error(what) {}
    table_error(const char *what) : std::runtime_error(what) {}
  };

  typedef std::pair<std::string, std::string> colval_t;

  struct table_page_t {
    uint16_t next_page;
    BYTE data[PAGE_SIZE - 3 * sizeof(uint16_t)];
    uint16_t dir_size;
    uint16_t free_bytes;
  };

  struct RID {
    uint16_t page_id;
    uint16_t rec_id;
  };
  struct column_type_t {
    std::string name;
    uint16_t ord;
    uint16_t type;
    uint16_t max_size;
  };

  //const BYTE TBL_MOD_MASTER = 0x1;
  //const BYTE TBL_MOD_TYPE = 0x2;


  struct pg_locations_t {
    std::string name;
    uint16_t first_page;
    uint16_t last_page;
  };
  struct master_table_row_t : public pg_locations_t {
    // needs RID
    uint16_t type;
    std::string def;
  };
  //struct master_table_t : pg_locations_t {
    
  //  std::vector<master_table_row_t> rows;
  //};
  struct table_descriptor_t : public pg_locations_t {
    /* a catalog is made up of a table called #master
       and a table called #columns (so far)

       #master first page is 1.
       #columns first page is at 2.

       a record/row in the #master table contains, so far
        |type|name|fp|lp|def|
        and a table has only one record in #master.
       a record in the #columns table contains, so far
        |tname|colname|ord|type|size|
        a table has as many records in #columns as it
        has columns.
    */
    std::vector<column_type_t> col_types;
  };

  struct col_def_t {
    std::string name;
    uint16_t type;
    uint16_t size;
  };

  void insert_into(file_descriptor_t &dbfile, const std::string &table_name, std::vector<colval_t> values);

  void print_master(file_descriptor_t &dbfile); 
  
  void print_table(file_descriptor_t &dbfile, const std::string &table_name);

  void tbl_format(const char fname[], uint16_t npages);

  void create_table(file_descriptor_t &dbfile, const std::string &tname, const std::vector<col_def_t> &cols);

  // iterate through allocated pages for this table and find the
  //  first page that has enough free bytes to contain a record of
  //  the given size.
  uint16_t rec_find_free(file_descriptor_t &dbfile,  pg_locations_t &location, uint16_t size);
  // Allocate a free page.  To allocate a free page, remove it from the
  //  master free list and add it to the end of this table page list
  uint16_t extend(table_descriptor_t td);

  master_table_row_t master_find_table(std::string tname, void *page, RID &rid);

  // Get page of #master from buffer manager
  // convert to master_table_t
  // find entry for table_name,
  // Get page of #columns from buffer manager
  // create table_descriptor_t td
  // put name, first_page, last_page from #master into td
  // find all entries of name in #columns and put then in col_types
  //    (may need to read multiple pages)
  void read_table_descriptor(file_descriptor_t &dbfile,
                                           const std::string &table_name,
                                           table_descriptor_t &table_descr);

  void extend_table(file_descriptor_t &pfile, pg_locations_t &location);

  // create master record and append to #master
  // for each in col_types
  //    create column record and append to #columns
  void write_new_table_descriptor(file_descriptor_t &dbfile,
                                  const table_descriptor_t &td);

  void write_updated_master_row(file_descriptor_t &dbfile,
                                  const master_table_row_t &td, RID rid);

  void tbl_pack_master_row(std::string &rec, const master_table_row_t &row);
  void tbl_pack_col_type(std::string &rec, const std::string &tname, const column_type_t &colt);
}; // namespace Table_pages

#endif // TABLE_H