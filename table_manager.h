/**************************************************************************************************
* Filename:   table_manager.h
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:  Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB buffer implementation.
**************************************************************************************************/

/*************************************************************************************************
// A table is a linked list of pages (with head and tail).  Each page has a
// collection of records for that specific table. Each table has only one linked
// list of pages that contains only that table records, and no other table's
// records.
*************************************************************************************************/

/*************************************************************************************************
  The catalog is made up of a header page (0), "#master" page (1), a "#columns" page (2), and a free page list (3)

  A record/row in the "#master" page follows the format: |type|name|fp|lp|def|
    Every unique table only has one record in the "#master" page
  
  A record/row in the "#columns" page follows the format: |tname|colname|ord|type|size|
    Every unique table has many records in the "#columns" page because a table can have many columns in it
*************************************************************************************************/

/********************************************* GUARD *********************************************/

#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

/***************************************** HEADER FILES ******************************************/

#include "paging/paging.h"

#include <climits>
#include <cstdint>
#include <string>
#include <vector>

/************************************** IMPLEMENT STRUCTURES *************************************/

namespace Table
{
  /* Structure that formats a data page with the linked list functionality added via <next_page> */
  struct table_page_t
  {
    uint16_t next_page;
    BYTE data[PAGE_SIZE - 3 * sizeof(uint16_t)];
    uint16_t dir_size;
    uint16_t free_bytes;
  };

  /* Structure that uniquely identifies a record in the database based on its <page_id> and <rec_id> */
  struct RID
  {
    uint16_t page_id;
    uint16_t rec_id;
  };

  /* Structure that collects the all but the table name for a "#columns" page record/row */
  struct column_type_t
  {
    std::string name;
    uint16_t ord;
    uint16_t type;
    uint16_t max_size;
  };

  /* Structure that collects the first 3 variables from a record in the "#master" page */
  struct pg_locations_t
  {
    std::string name;
    uint16_t first_page;
    uint16_t last_page;
  };

  /* Structure that collects the last 2 variables from a record in the "#master" page ; also inherits from <pg_locations_t> */
  struct master_table_row_t : public pg_locations_t
  {
    // needs RID
    uint16_t type;
    std::string def;
  };

  /* Structure for housing a vector that will contain all <column_type_t> values that match a given table name ; also inherits from <pg_locations_t>*/
  struct table_descriptor_t : public pg_locations_t
  {
    std::vector<column_type_t> col_types;
  };

  /* ??? */
  struct col_def_t
  {
    std::string name;
    uint16_t type;
    uint16_t size;
  };

  // /* Struct defining a vector of "#master" table rows */
  // struct master_table_t
  // {  
  //   using Page;
  //   Page_t m_page;
  // };

  // /* Struct defining a vector of "#columns" table rows */
  // struct columns_table_t
  // {
  //   using Page;
  //   Page_t c_page;
  // };
}

/******************************************* CONSTANTS *******************************************/

namespace Table
{
  const uint16_t TBL_TYPE_VCHAR = 9;
  const uint16_t TBL_TYPE_SHORT = 2;

  const char TBL_MASTER_NAME[] = "#master";
  const uint16_t TBL_MASTER_PAGE = 1;

  const char TBL_COLUMN_NAME[] = "#columns";
  const uint16_t TBL_COLUMNS_PAGE = 2;

  const uint16_t DB_TYPE_TABLE = 1;

  //const BYTE TBL_MOD_MASTER = 0x1;
  //const BYTE TBL_MOD_TYPE = 0x2;
}

/******************************************* DATATYPES *******************************************/

namespace Table
{
  typedef std::pair<std::string, std::string> colval_t;
}

namespace Table
{

  /************************************** DEFINE TABLE ERROR *************************************/

  class table_error : public std::runtime_error
  {
    public:
      table_error(std::string what) : std::runtime_error(what) {}
      table_error(const char *what) : std::runtime_error(what) {}
  };

  /************************************* FUNCTION PROTOTYPES *************************************/

  void insert_into(file_descriptor_t &dbfile, const std::string &table_name, std::vector<colval_t> values);

  void print_master(file_descriptor_t &dbfile); 
  
  void print_table(file_descriptor_t &dbfile, const std::string &table_name);

  /* Bootstrapper for tables. Creates #master and #columns */
  void tbl_format(const char fname[], uint16_t npages);

  void create_table(file_descriptor_t &dbfile, const std::string &tname, const std::vector<col_def_t> &cols);

  /* Iterate through allocated pages for a table and find the first page that has enough free bytes to contain a record of given size */
  uint16_t rec_find_free(file_descriptor_t &dbfile, pg_locations_t &location, uint16_t size);

  /* Allocate a free page by removing it from the free pages list and adding it to the end of this table's linked list */
  uint16_t extend(table_descriptor_t td);

  master_table_row_t master_find_table(std::string tname, void* page, RID &rid);

  // Get page of #master from buffer manager
  // convert to master_table_t
  // find entry for table_name,
  // Get page of #columns from buffer manager
  // create table_descriptor_t td
  // put name, first_page, last_page from #master into td
  // find all entries of name in #columns and put then in col_types
  // (may need to read multiple pages)
  void read_table_descriptor(file_descriptor_t &dbfile, const std::string &table_name, table_descriptor_t &table_descr);

  void extend_table(file_descriptor_t &pfile, pg_locations_t &location);

  /* Create master record and append to "#master" and for each in <col_types>, create column record and append to "#columns" */
  // get the #master page
  // create empty void page buffer
  // create empty string buffer
  // pack the master record data into a string
  // convert the td into a <master_table_row_t>
  // insert the record for the <master_table_row_t>
  // write the master page to the buffer pool

  // get the #columns page
  // loop through all the columns in <td>
    // check that the #columns page has enough room, call extend table if needed
    // write a record with a single column
  // write the #columns page to the buffer pool
  void write_new_table_descriptor(file_descriptor_t &dbfile, const table_descriptor_t &td);

  void write_updated_master_row(file_descriptor_t &dbfile, const master_table_row_t &td, RID rid);

  void tbl_pack_master_row(std::string &rec, const master_table_row_t &row);

  void tbl_pack_col_type(std::string &rec, const std::string &tname, const column_type_t &colt);
}; // namespace Table

#endif // TABLE_MANAGER_H