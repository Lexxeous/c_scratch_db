/**************************************************************************************************
* Filename:   buffer_manager.cpp
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:  Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB buffer implementation.
**************************************************************************************************/

/****************************************** HEADER FILES *****************************************/

#include "table_manager.h"
#include "buffer_mgr/buffer_mgr.h"

/************************************ FUNCTION IMPLEMENTATIONS ***********************************/

namespace Table
{
  void insert_into(file_descriptor_t &dbfile, const std::string &table_name, std::vector<colval_t> values)
  {

  }


  void print_master(file_descriptor_t &dbfile)
  {

  }


  void print_table(file_descriptor_t &dbfile, const std::string &table_name)
  {

  }


  void tbl_format(const char fname[], uint16_t npages)
  {
    Page_file::pgf_format(fname, npages); // first format the page file with the paging layer

    std::fstream dbfile(fname, std::ios::in | std::ios::out | std::ios::binary);
    if (!dbfile)
      throw Page::paging_error("Cannot open page file for writing.");

    /* Create "#master" table page with one entry for "#columns" table */
    table_page_t* mt_page = static_cast<table_page_t*>(Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE));
    mt_page->next_page = 0;
    mt_page->free_bytes -= sizeof(uint16_t); // so that page directory will never point to the next_page
    Buffer_mgr::buf_write(dbfile, TBL_MASTER_PAGE);

    /* Create empty "#columns" table page */
    table_page_t* colpage = static_cast<table_page_t*>(Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE));
    colpage->free_bytes -= sizeof(uint16_t); // skip the next_page bytes
    colpage->next_page = 0;
    Buffer_mgr::buf_write(dbfile, TBL_COLUMNS_PAGE);

    /* Create all the entries that will go in "#columns" (so "#columns" will have an entry for itself) */
    struct triple {
      std::string name;
      uint16_t type;
      uint16_t size;
    };

    triple all[] = {{"tname", TBL_TYPE_VCHAR, 40},
                   {"colname", TBL_TYPE_VCHAR, 40},
                   {"ord", TBL_TYPE_SHORT, 1},
                   {"type", TBL_TYPE_SHORT, 1},
                   {"size", TBL_TYPE_SHORT, 1}};

    table_descriptor_t td;
    td.name = "#columns";
    td.first_page = TBL_COLUMNS_PAGE;
    td.last_page = TBL_COLUMNS_PAGE;
    int count = 0;
    for (auto each : all)
    {
      column_type_t ct;
      ct.name = each.name;
      ct.ord = count;
      ct.type = each.type;
      ct.max_size = each.size;
      td.col_types.push_back(ct);
    }

    write_new_table_descriptor(dbfile, td);
  }


  void create_table(file_descriptor_t &dbfile, const std::string &tname, const std::vector<col_def_t> &cols)
  {

  }


  uint16_t rec_find_free(file_descriptor_t &dbfile, pg_locations_t &location, uint16_t size)
  {
    if (location.last_page > 0) // if at least one page has been allocated...
    {
      table_page_t* last_page = static_cast<table_page_t*>(Buffer_mgr::buf_read(dbfile, location.last_page)); // read last page allocated
      if (last_page->free_bytes >= sizeof(uint16_t) + size) // if enough space in the last page for the record
      {
        return location.last_page; // return the last page
      }
    }

    /* Either the table has no pages allocated yet, or the last page does not have enough space, so we need to extend */
    extend_table(dbfile, location);

    /* The <location> is changed in the "extend_table()" function */
    return location.last_page; // return last page after extension
  }


  uint16_t extend(table_descriptor_t td)
  {
    std::cout << "I am inside extend() function" << std::endl;
    return 0;
  }


  master_table_row_t master_find_table(std::string tname, void* page, RID &rid)
  {
    std::cout << "I am inside master_find_table() function" << std::endl;
    master_table_row_t mtr;
    mtr.type = 99;
    mtr.def = "DEF";
    return mtr;
  }


  void read_table_descriptor(file_descriptor_t &dbfile, const std::string &table_name, table_descriptor_t &table_descr)
  {

  }


  void extend_table(file_descriptor_t &pfile, pg_locations_t &location)
  {
    /* Get free page list with format: |numpgs|pg|pg|pg|pg| */
    Page_file::page_free_t* pgfree = static_cast<Page_file::page_free_t*>(Buffer_mgr::buf_read(pfile, Page_file::PGF_PAGES_FREE_ID));
    
    if (pgfree->size == 0) // if the free pages list is empty
      throw table_error("No free pages available");

    /* Get the last page_id in the list of free pages, and then delete it from the list */
    uint16_t free_page_id = pgfree->free[pgfree->size - 1];
    pgfree->size--;

    // Buffer the free pages page to maintain the buffer pool
    Buffer_mgr::buf_write(pfile, Page_file::PGF_PAGES_FREE_ID);

    /* If the table had at least one table page already allocated, add the new page to the end of the table linked list */
    if (location.last_page != 0)
    {
      table_page_t* old_last = static_cast<table_page_t*>(Buffer_mgr::buf_read(pfile, location.last_page));
      old_last->next_page = free_page_id;
      Buffer_mgr::buf_write(pfile, location.last_page);
    }

    /* Set the table's last page <next_page> to 0 (because it's now the last page in the table) */
    table_page_t* new_page = static_cast<table_page_t*>(Buffer_mgr::buf_read(pfile, free_page_id));
    new_page->next_page = 0;

    /* Correctly format the directory for the new page. This is breaking the layering and should be handled in the "paging" layer */
    uint16_t* dir = PG_DIRECTORY(new_page);
    dir[0] = 0;
    new_page->dir_size = 1;
    new_page->free_bytes -= sizeof(new_page->dir_size) + sizeof(uint16_t);

    /* Update the master table to reflect that the table has a new last page */
    if (location.last_page == 0) // if no pages have yet been allocated for the table
      location.first_page = location.last_page = free_page_id; 
    else // if one or more pages have already been allocated to the table
      location.last_page = free_page_id;

    /* Buffer the new page that was added as the table's last page to maintain the buffer pool */
    Buffer_mgr::buf_write(pfile, free_page_id);

    /* Must change record in master table that records last page */
    void* page = Buffer_mgr::buf_read(pfile, TBL_MASTER_PAGE); // read the master page
    RID rid;
    master_table_row_t mtr = master_find_table(location.name, page, rid);
    mtr.first_page = location.first_page;
    mtr.last_page = location.first_page;
    write_updated_master_row(pfile, mtr, rid);
  }


  void write_new_table_descriptor(file_descriptor_t &dbfile, const table_descriptor_t &td)
  {
    std::cout << "I am inside write_new_table_descriptor() function" << std::endl;
  }


  void write_updated_master_row(file_descriptor_t &dbfile, const master_table_row_t &td, RID rid)
  {

  }


  void tbl_pack_master_row(std::string &rec, const master_table_row_t &row)
  {
    /* The "#master" page format: |name|fp|lp|type|def| */
    Page::rec_begin(rec);
    Page::rec_packstr(rec, row.name);
    Page::rec_packshort(rec, row.first_page);
    Page::rec_packshort(rec, row.last_page);
    Page::rec_packshort(rec, row.type);
    Page::rec_packstr(rec, row.def);
    Page::rec_finish(rec);
  }


  void tbl_pack_col_type(std::string &rec, const std::string &tname, const column_type_t &colt)
  {
    /* The "#columns" page format: |tname|colname|ord|type|size| */
    Page::rec_begin(rec);
    Page::rec_packstr(rec, tname);
    Page::rec_packstr(rec, colt.name);
    Page::rec_packshort(rec, colt.ord);
    Page::rec_packshort(rec, colt.type);
    Page::rec_packshort(rec, colt.max_size);
    Page::rec_finish(rec);
  }
}

