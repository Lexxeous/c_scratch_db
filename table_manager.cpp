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
    table_page_t* mstr_page = static_cast<table_page_t*>(Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE));
    mstr_page->next_page = 0;
    mstr_page->free_bytes -= sizeof(uint16_t); // so that page directory will never point to the next_page
    Buffer_mgr::buf_write(dbfile, TBL_MASTER_PAGE);

    /* Create empty "#columns" table page */
    table_page_t* cols_page = static_cast<table_page_t*>(Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE));
    cols_page->free_bytes -= sizeof(uint16_t); // skip the next_page bytes
    cols_page->next_page = 0;
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
    td.name = "#columns"; // this is the table name for the "#columns" page
    td.first_page = TBL_COLUMNS_PAGE;
    td.last_page = TBL_COLUMNS_PAGE;
    int count = 0;
    for (auto each : all)
    {
      column_type_t ct;
      ct.name = each.name; // column name -> |"tname"|"colname"|"ord"|"type"|"size"|
      ct.ord = count;
      ct.type = each.type;
      ct.max_size = each.size;
      td.col_types.push_back(ct);
      count++;
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
    Page::Page_t* mstr_page = (Page::Page_t*)page;
    uint16_t* offset_arr = PG_DIRECTORY(page); // pointer to the beginning of the page directory

    uint16_t* next_page = (uint16_t*)page; // next page in the master table linked list
    std::cout << "Next page for the master table: " << *next_page << std::endl;

    master_table_row_t mtr;
    for(int i = 0; i < mstr_page->dir_size; i++)
    {
      bool found = false;
      std::string cur_tname = ""; // empty sting to store each table name
      std::string cur_def = ""; // empty string to store each definition
      int16_t u = sizeof(uint16_t); // u = 2 ; for easy traversal of the current master record

      std::cout << "Byte offest of the current master record: " << offset_arr[i] << std::endl;

      uint16_t* cur_rec_len = (uint16_t*)((BYTE*)page + offset_arr[i]); // length of the current master record
      uint16_t* tname_val = (uint16_t*)((BYTE*)page + offset_arr[i] + u); // table name length + 9
      BYTE* tname_str = (BYTE*)((BYTE*)page + offset_arr[i] + (2*u)); // pointer to the beginning of the table name

      int tname_len = (*tname_val - Page::RTYPE_STRING);
      for(int j = 0; j < tname_len; j++) // loop through every character in the table name
      {
        cur_tname += *(tname_str + j); // append every character in the table name
      }

      if(tname == cur_tname) // if current table name matches
        found = true; // found the table that we were looking for
      else
        continue; // not the table we were looking for, try the next one

      uint16_t* cur_first_page = (uint16_t*)((BYTE*)page + offset_arr[i] + (2*u) + tname_len + u); // pointer to the <first_page>
      uint16_t* cur_last_page = (uint16_t*)((BYTE*)page + offset_arr[i] + (2*u) + tname_len + (3*u)); // pointer to <last_page>
      uint16_t* cur_type = (uint16_t*)((BYTE*)page + offset_arr[i] + (2*u) + tname_len + (5*u)); // pointer to <type>
      uint16_t* def_val = (uint16_t*)((BYTE*)page + offset_arr[i] + (2*u) + tname_len + (6*u)); // <def> length + 9
      BYTE* def_str = (BYTE*)((BYTE*)page + offset_arr[i] + (2*u) + tname_len + (7*u)); // pointer to <def>

      int def_len = (*def_val - Page::RTYPE_STRING);
      for(int k = 0; k < def_len; k++) // loop through every character in <def>
      {
        cur_def += *(def_str + k); // append every character in <def>
      }

      std::cout << "Length of Current Record: " << *cur_rec_len << std::endl;
      std::cout << "Length of Current Table Name String: " << tname_len << std::endl;
      std::cout << "Current Table Name: " << cur_tname << std::endl;
      std::cout << "Current First Page: " << *cur_first_page << std::endl;
      std::cout << "Current Last Page: " << *cur_last_page << std::endl;
      std::cout << "Current Type: " << *cur_type << std::endl;
      std::cout << "Length of the Current Definition String: " << def_len << std::endl;
      std::cout << "Current Definition: " << cur_def << std::endl;

      if(found)
      {
        mtr.name = cur_tname;
        mtr.first_page = *cur_first_page;
        mtr.last_page = *cur_last_page;
        mtr.type = *cur_type;
        mtr.def = cur_def;
        rid.page_id = 1; 
        rid.rec_id = i;
        break; // stop searching
      }
    }

    return mtr;
  }


  void read_table_descriptor(file_descriptor_t &dbfile, const std::string &table_name, table_descriptor_t &table_descr)
  {
    Page::Page_t* mstr_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE)); // get "#master ||[np]rec1|...|recN|E|F|"



    Page::Page_t* cols_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE)); // get "#columns |[np]|rec1|...|recN|E|F|"
  }


  void extend_table(file_descriptor_t &pfile, pg_locations_t &location)
  {
    /* Get free page list with format: |numpgs|pg|pg|pg|pg| */
    Page_file::page_free_t* pgfree = static_cast<Page_file::page_free_t*>(Buffer_mgr::buf_read(pfile, Page_file::PGF_PAGES_FREE_ID));
    
    if (pgfree->size == 0) // if the free pages list is empty
      throw table_error("No free pages available.");

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

    /* Must change record in master table that holds last page */
    void* page = Buffer_mgr::buf_read(pfile, TBL_MASTER_PAGE); // get "#master |rec1|...|recN|E|F|"
    RID rid;
    master_table_row_t mtr = master_find_table(location.name, page, rid);
    mtr.first_page = location.first_page;
    mtr.last_page = location.last_page;
    write_updated_master_row(pfile, mtr, rid);
  }


  void write_new_table_descriptor(file_descriptor_t &dbfile, const table_descriptor_t &td)
  {
    std::string mstr_str; // empty master string for packing data
    std::string cols_str; // empty columns string for packing data

    /* Create a new <master_table_row> and assign all the appropriate values from <td> */
    master_table_row_t mtr; 
    mtr.name = td.name; // the name for the new table (<tname>)
    mtr.first_page = td.first_page;
    mtr.last_page = td.last_page;
    mtr.type = 0;
    mtr.def = "0";

    /* Create record in "#master" for the new table and write to buffer */
    Page::Page_t* mstr_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE)); // get "#master |rec1|...|recN|E|F|"
    tbl_pack_master_row(mstr_str, mtr);
    Page::pg_add_record((void*)mstr_page, (void*)mstr_str.data(), mstr_str.size());
    Buffer_mgr::buf_write(dbfile, TBL_MASTER_PAGE);

    /* Create records in "#columns" for all of the unique columns in the new table and write to buffer */
    Page::Page_t* cols_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE)); // get "#columns |rec1|...|recN|E|F|"
    for(int i = 0; i < td.col_types.size(); i++) // loop through all of the column types in the vector
    {
      tbl_pack_col_type(cols_str, mtr.name, td.col_types[i]);

      if(i==4)//cols_str.size() > cols_page->free_bytes) // if record cannot fit in "#columns"
      {
        /* Create a new <page_locations_t> and assign the appropriate values from <td> */
        pg_locations_t pgl;
        pgl.name = td.name; // the name for the new table (<tname>)
        pgl.first_page = td.first_page;
        pgl.last_page = td.last_page;
        std::cout << "First Page, Last Page: " << pgl.first_page << ", " << pgl.last_page << std::endl;
        extend_table(dbfile, pgl); 
        Page::Page_t* ext_cols_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, pgl.last_page));
        Page::pg_add_record((void*)ext_cols_page, (void*)cols_str.data(), cols_str.size());
        Buffer_mgr::buf_write(dbfile, pgl.last_page);
        return;
      } // TEST THIS STATEMENT TO SEE IF THE #COLUMNS TABLE ACTUALLY GETS EXTENDED PROPERLY

      Page::pg_add_record((void*)cols_page, (void*)cols_str.data(), cols_str.size());
    }

    Buffer_mgr::buf_write(dbfile, TBL_COLUMNS_PAGE);
  }

  // i have all the data to write to a master row
  // get the master table page at <rid.page_id>
  // get the dir = PAGE_DIRECTORY
  // jump to the offset of dir[rec_id]
  // traverse to <last_page>, <type>, and <def> and write the data
    // must shift the records if <def> is not the same length...
  void write_updated_master_row(file_descriptor_t &dbfile, const master_table_row_t &td, RID rid)
  {
    int16_t u = sizeof(uint16_t); // u = 2 ; for easy traversal of the current master record
    void* mstr_page = (void*)Buffer_mgr::buf_read(dbfile, rid.page_id);
    uint16_t* offset_arr = PG_DIRECTORY(mstr_page); // pointer to the beginning of the page directory

    uint16_t* update_last_page = (uint16_t*)((BYTE*)mstr_page + offset_arr[rid.rec_id] + (2*u) + (td.name).length() + (3*u)); // pointer to <last_page>
    uint16_t* update_type = (uint16_t*)((BYTE*)mstr_page + offset_arr[rid.rec_id] + (2*u) + (td.name).length() + (5*u)); // pointer to <type>

    *update_last_page = td.last_page; // assign updated value to the pointer location at <last_page>
    *update_type = td.type; // assign updated value to the pointer location at <type>

    Buffer_mgr::buf_write(dbfile, rid.page_id);
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

