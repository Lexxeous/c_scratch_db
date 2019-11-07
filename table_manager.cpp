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

    struct triple {
      std::string name;
      uint16_t type;
      uint16_t size;
    };

    
    /* Create all the entries that will go in "#master" (so "#master" will have an entry for itself) */
    triple mstr[] = {{"name", TBL_TYPE_VCHAR, 40},
                   {"fp", TBL_TYPE_SHORT, 1},
                   {"lp", TBL_TYPE_SHORT, 1},
                   {"type", TBL_TYPE_SHORT, 1},
                   {"def", TBL_TYPE_VCHAR, 40}};

    table_descriptor_t mstr_td;
    mstr_td.name = "#master"; // this is the table name for the "#master" page
    mstr_td.first_page = TBL_MASTER_PAGE;
    mstr_td.last_page = TBL_MASTER_PAGE;
    int mstr_count = 0;
    for (auto each : mstr)
    {
      // |"name"|"fp"|"lp"|"type"|"def"|
      column_type_t mstr_ct;
      mstr_ct.name = each.name; // column name
      mstr_ct.ord = mstr_count;
      mstr_ct.type = each.type;
      mstr_ct.max_size = each.size;
      mstr_td.col_types.push_back(mstr_ct);
      mstr_count++;
    }
    write_new_table_descriptor(dbfile, mstr_td);


    /* Create all the entries that will go in "#columns" (so "#columns" will have an entry for itself) */
    triple cols[] = {{"tname", TBL_TYPE_VCHAR, 40},
                   {"colname", TBL_TYPE_VCHAR, 40},
                   {"ord", TBL_TYPE_SHORT, 1},
                   {"type", TBL_TYPE_SHORT, 1},
                   {"size", TBL_TYPE_SHORT, 1}};

    table_descriptor_t cols_td;
    cols_td.name = "#columns"; // this is the table name for the "#columns" page
    cols_td.first_page = TBL_COLUMNS_PAGE;
    cols_td.last_page = TBL_COLUMNS_PAGE;
    int cols_count = 0;
    for (auto each : cols)
    {
      // |"tname"|"colname"|"ord"|"type"|"size"|
      column_type_t cols_ct;
      cols_ct.name = each.name; // column name
      cols_ct.ord = cols_count;
      cols_ct.type = each.type;
      cols_ct.max_size = each.size;
      cols_td.col_types.push_back(cols_ct);
      cols_count++;
    }
    write_new_table_descriptor(dbfile, cols_td);
  }


  void create_table(file_descriptor_t &dbfile, const std::string &tname, const std::vector<col_def_t> &cols)
  {
    /* Get free page list with format: |numpgs|pg|pg|pg|pg| */
    Page_file::page_free_t* pgfree = static_cast<Page_file::page_free_t*>(Buffer_mgr::buf_read(dbfile, Page_file::PGF_PAGES_FREE_ID));
    
    if(pgfree->size == 0) // if the free pages list is empty
      throw table_error("No free pages available.");

    /* Get the last page_id in the list of free pages, and then delete it from the list */
    uint16_t free_page_id = pgfree->free[pgfree->size - 1];
    pgfree->size--;

    Buffer_mgr::buf_write(dbfile, Page_file::PGF_PAGES_FREE_ID); // Buffer the free pages page to maintain the buffer pool

    /* Create a new master table row for the new table */
    master_table_row_t new_mtr;
    new_mtr.name = tname;
    new_mtr.first_page = free_page_id;
    new_mtr.last_page = 0;

    /* Pack all of the new table data and write to "#master" and "#columns" */
    table_descriptor_t create_td;
    create_td.name = new_mtr.name;
    create_td.first_page = new_mtr.first_page;
    create_td.last_page = new_mtr.last_page;
    int cols_count = 0;
    for (auto each : cols)
    {
      // |"tname"|"colname"|"ord"|"type"|"size"|
      column_type_t cols_ct;
      cols_ct.name = each.name; // column name
      cols_ct.ord = cols_count;
      cols_ct.type = each.type;
      cols_ct.max_size = each.size;
      create_td.col_types.push_back(cols_ct);
      cols_count++;
    }
    write_new_table_descriptor(dbfile, create_td);
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
    uint16_t* mstr_nxt_page = (uint16_t*)page; // next page in the master table linked list

    master_table_row_t mtr;
    for(int i = 0; i < mstr_page->dir_size; i++)
    {
      bool found = false;
      std::string cur_tname = ""; // empty sting to store each table name
      std::string cur_def = ""; // empty string to store each definition
      int16_t u = sizeof(uint16_t); // u = 2 ; for easy traversal of the current master record

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
      {
        continue; // not the table we were looking for, try the next one
      }

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

      if(found)
      {
        mtr.name = cur_tname;
        mtr.first_page = *cur_first_page;
        mtr.last_page = *cur_last_page;
        mtr.type = *cur_type;
        mtr.def = cur_def;
        rid.page_id = 1; // I AM NOT LOOPING THROUGH ALL OF THE PAGES OF "#master" YET
        rid.rec_id = i;
        break; // stop searching
      }
      else
      {
        throw table_error("Table name not found in #master table.");
      }
    }

    return mtr;
  }


  void read_table_descriptor(file_descriptor_t &dbfile, const std::string &table_name, table_descriptor_t &table_descr)
  {
    int16_t u = sizeof(uint16_t); // u = 2 ; for easy traversal of the current record
    RID rid;

    void* mstr_page = (void*)Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE); // read the first page in the "#master" table
    master_table_row_t mstr_row = master_find_table(table_name, mstr_page, rid); // assumes that you find the master row, would throw error otherwise

    table_descr.name = mstr_row.name;
    table_descr.first_page = mstr_row.first_page;
    table_descr.last_page = mstr_row.last_page;

    void* cols_page = (void*)Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE); // read the first "#columns" page
    while(true)
    {
      Page::Page_t* alt_cols_page = (Page::Page_t*)cols_page;
      uint16_t* cols_nxt_page = (uint16_t*)cols_page; // next page in the master table linked list

      for(int i = 0; i < alt_cols_page->dir_size; i++) //loop through all records in the current columns page
      {
        std::string cur_tname = "";
        std::string cur_cname = "";

        uint16_t* offset_arr = PG_DIRECTORY(cols_page); // get the list of offsets for the current "#columns" page
        uint16_t* tname_val = (uint16_t*)((BYTE*)cols_page + offset_arr[i] + u); // next page in the master table linked list
        BYTE* tname_str = (BYTE*)((BYTE*)cols_page + offset_arr[i] + (2*u)); // pointer to the beginning of the table name

        int tname_len = (*tname_val - Page::RTYPE_STRING);
        for(int j = 0; j < tname_len; j++) // loop through every character in the table name
        {
          cur_tname += *(tname_str + j); // append every character in the table name
        }

        if(cur_tname == table_name) // if the table name matches
        {
          /* Then unpack all of the data: name, ord, type, and max_size */
          uint16_t* cname_val = (uint16_t*)((BYTE*)cols_page + offset_arr[i] + (2*u) + tname_len);
          BYTE* cname_str = (BYTE*)((BYTE*)cols_page + offset_arr[i] + (2*u) + tname_len + u); // pointer to the beginning of <cname>

          int cname_len = (*cname_val - Page::RTYPE_STRING);
          for(int k = 0; k < cname_len; k++) // loop through every character in <cname>
          {
            cur_cname += *(cname_str + k); // append every character in the <cname>
          }

          uint16_t* cur_ord = (uint16_t*)((BYTE*)cols_page + offset_arr[i] + (2*u) + tname_len + u + cname_len + u); // get the current record <ord>
          uint16_t* cur_type = (uint16_t*)((BYTE*)cols_page + offset_arr[i] + (2*u) + tname_len + u + cname_len + (3*u)); // get the current record <type>
          uint16_t* cur_mxsz = (uint16_t*)((BYTE*)cols_page + offset_arr[i] + (2*u) + tname_len + u + cname_len + (5*u)); // get the current record <max_size>

          /* Populate <table_descr> with all of the unpacked values */
          column_type_t cur_ct;
          cur_ct.name = cur_cname;
          cur_ct.ord = *cur_ord;
          cur_ct.type = *cur_type;
          cur_ct.max_size = *cur_mxsz;
          table_descr.col_types.push_back(cur_ct);
        }
        else
          continue; // go to the next record in "#columns" 
      }
      if(*cols_nxt_page == 0) // if last page in the "#columns" table
      {
        break; // break out of the while loop because we have already read the last page in the table
      }
      else
      {
        void* cols_page = (void*)Buffer_mgr::buf_read(dbfile, (int)(*cols_nxt_page)); // read the next "#columns" page
      }
    }
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

    // I AM NOT LOOPING THROUGH THE LINKED LIST YET TO FIND THE LAST PAGE BEFORE READING AND CHECKING THE FREE BYTES
    /* Create record in "#master" for the new table and write to buffer */
    Page::Page_t* mstr_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE)); // get "#master |rec1|...|recN|E|F|"
    tbl_pack_master_row(mstr_str, mtr);

    // CANNOT EXTEND THE MASTER TABLE IF THERE ARE NO RECORDS FOR IT IN "#master" or "#columns"
    if(mstr_str.size() > mstr_page->free_bytes) // if record cannot fit in original "#master" page (page 1)
    {
      pg_locations_t mstr_pgl; // create new empty <page_locations_t> for master
      mstr_pgl.name = td.name; // the name for the new table (<tname>)
      mstr_pgl.first_page = td.first_page;
      mstr_pgl.last_page = td.last_page;
      extend_table(dbfile, mstr_pgl); // extend the master table
      Page::Page_t* ext_mstr_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, mstr_pgl.last_page)); // read the last page in "#master"
      Page::pg_add_record((void*)ext_mstr_page, (void*)mstr_str.data(), mstr_str.size()); // add record to "#master" last page
      Buffer_mgr::buf_write(dbfile, mstr_pgl.last_page); // write "#master" last page to buffer
    }
    else // new record can fit in the original "#master" page
    {
      Page::pg_add_record((void*)mstr_page, (void*)mstr_str.data(), mstr_str.size());
      Buffer_mgr::buf_write(dbfile, TBL_MASTER_PAGE);
    }

    // I AM NOT LOOPING THROUGH THE LINKED LIST YET TO FIND THE LAST PAGE BEFORE READING AND CHECKING THE FREE BYTES
    /* Create records in "#columns" for all of the unique columns in the new table and write to buffer */
    Page::Page_t* cols_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE)); // get "#columns |rec1|...|recN|E|F|"
    for(int i = 0; i < td.col_types.size(); i++) // loop through all of the column types in the vector
    {
      tbl_pack_col_type(cols_str, mtr.name, td.col_types[i]);

      if(cols_str.size() > cols_page->free_bytes) // if record cannot fit in "#columns"
      {
        /* Create a new <page_locations_t> and assign the appropriate values from <td> */
        pg_locations_t cols_pgl;
        cols_pgl.name = td.name; // the name for the new table (<tname>)
        cols_pgl.first_page = td.first_page;
        cols_pgl.last_page = td.last_page;
        extend_table(dbfile, cols_pgl); 
        Page::Page_t* ext_cols_page = static_cast<Page::Page_t*>(Buffer_mgr::buf_read(dbfile, cols_pgl.last_page));
        Page::pg_add_record((void*)ext_cols_page, (void*)cols_str.data(), cols_str.size());
        Buffer_mgr::buf_write(dbfile, cols_pgl.last_page);
      }
      else
      {
        Page::pg_add_record((void*)cols_page, (void*)cols_str.data(), cols_str.size());
      } 
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
    // NOT UPDATING THE DEFINITION YET

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

