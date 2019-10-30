void Table_pages::tbl_format(const char fname[], uint16_t npages) // Bootstrapper for tables. Creates #master and #columns.
{
  Page_file::pgf_format(fname, npages);
  // now, create master table page with one entry for #columns table
  std::fstream dbfile(fname, std::ios::in | std::ios::out | std::ios::binary);
  if (!dbfile) {
    throw Page::paging_error("cannot open page file for writing");
  }
  table_page_t *mt_page = static_cast<table_page_t *>(
      Buffer_mgr::buf_read(dbfile, TBL_MASTER_PAGE));
  mt_page->next_page = 0;
  mt_page->free_bytes -= sizeof(
      uint16_t); // so that page directory will never point to the next_page
  Buffer_mgr::buf_write(dbfile, TBL_MASTER_PAGE);

  // create empty columns table page
  table_page_t *colpage = static_cast<table_page_t *>(
      Buffer_mgr::buf_read(dbfile, TBL_COLUMNS_PAGE));
  colpage->free_bytes -= sizeof(uint16_t); // skip the next_page bytes
  colpage->next_page = 0;
  Buffer_mgr::buf_write(dbfile, TBL_COLUMNS_PAGE);

  // def = "create table #columns (tname varchar(40), colname varchar(40), ord
  // short, type short, size short)";

  // Now, create all the entries that will go in #columns (so #columns will have
  // an entry for itself)
  struct three {
    std::string name;
    uint16_t type;
    uint16_t size;
  };

  three all[] = {{"tname", TBL_TYPE_VCHAR, 40},
                 {"colname", TBL_TYPE_VCHAR, 40},
                 {"ord", TBL_TYPE_SHORT, 1},
                 {"type", TBL_TYPE_SHORT, 1},
                 {"size", TBL_TYPE_SHORT, 1}};

  table_descriptor_t td;
  td.name = "#columns";
  td.first_page = TBL_COLUMNS_PAGE;
  td.last_page = TBL_COLUMNS_PAGE;
  int count = 0;
  for (auto each : all) {
    column_type_t ct;
    ct.name = each.name;
    ct.ord = count;
    ct.type = each.type;
    ct.max_size = each.size;
    td.col_types.push_back(ct);
  }

  write_new_table_descriptor(dbfile, td);
}


uint16_t Table_pages::rec_find_free(file_descriptor_t &dbfile, pg_locations_t &location, uint16_t size)
{
  // if at least one page has been allocated...
  if (location.last_page > 0) {
    //    read last page allocated
    table_page_t *last_page = static_cast<table_page_t *>(
        Buffer_mgr::buf_read(dbfile, location.last_page));
    //    test to see if free_bytes + sizeof(uint16_6) >= size
    if (last_page->free_bytes >= sizeof(uint16_t) + size) {
      //    if so, return last_page
      return location.last_page;
    }
  }
  // At this point, either the table has no pages allocated, or the last
  //   page does not have enough space for the record, so we need to extend it.
  // extend table       could throw error if no pages
  extend_table(dbfile, location);
  // re-read locations page  <- no need, location variable should be changed in
  // extend_table return last_page
  return location.last_page;
}

void Table_pages::extend_table(file_descriptor_t &pfile, pg_locations_t &location)
{
  // get free page list
  Page_file::page_free_t *pgfree = static_cast<Page_file::page_free_t *>(
      Buffer_mgr::buf_read(pfile, Page_file::PGF_PAGES_FREE_ID));
  //  |numpgs|pg|pg|pg|pg|
  if (pgfree->size == 0) {
    throw table_error("No free pages available");
  }
  // get the first page_id in the list of free pages, and then
  //   delete it from the list
  /*
  std::vector<uint16_t> pgvec(pgfree->free, pgfree->free + pgfree->size);
  uint16_t free_page_id = pgvec[pgvec.size()];
  pgvec.pop_back();
  pgfree->size = pgvec.size();
  memmove(pgfree->free, pgvec.data(), pgvec.size());
  */
  uint16_t free_page_id = pgfree->free[pgfree->size - 1];
  pgfree->size--;

  // Now, write the free page back to update the persistent list
  Buffer_mgr::buf_write(pfile, Page_file::PGF_PAGES_FREE_ID);

  // Add this new page to the end of our list of pages for the table
  //   but only if the table had, at least, one table page already allocated.
  if (location.last_page != 0) {
    table_page_t *old_last = static_cast<table_page_t *>(
        Buffer_mgr::buf_read(pfile, location.last_page));
    old_last->next_page = free_page_id;
    Buffer_mgr::buf_write(pfile, location.last_page);
  }
  // set the free pages next_page to 0 (its now the last page)
  table_page_t *new_page =
      static_cast<table_page_t *>(Buffer_mgr::buf_read(pfile, free_page_id));
  new_page->next_page = 0;

  //  I am breaking the layering here.  This should be handled in the Paging layer
  uint16_t *dir = PG_DIRECTORY(new_page);
  dir[0] = 0;
  new_page->dir_size = 1;
  new_page->free_bytes -= sizeof(new_page->dir_size) + sizeof(uint16_t); // sizeof dir entry plus next_page field

  // Update the master table to reflect that the table has a
  //   new last page
  if (location.last_page == 0) {
    location.first_page = location.last_page = free_page_id;
  } else {
    location.last_page = free_page_id;
  }

  Buffer_mgr::buf_write(pfile, free_page_id);

  // Note: must change record in master table that records last page
  // Get page of #master from buffer manager
  void *page = Buffer_mgr::buf_read(pfile, TBL_MASTER_PAGE);
  RID rid;
  master_table_row_t mtr = master_find_table(location.name, page, rid);
  mtr.first_page = location.first_page;
  mtr.last_page = location.first_page;
  write_updated_master_row(pfile, mtr, rid);
}

void Table_pages::tbl_pack_master_row(std::string &rec, const master_table_row_t &row)
{
  Page::rec_begin(rec);
  //|type|name|fp|lp|def|
  Page::rec_packshort(rec, row.type);
  Page::rec_packstr(rec, row.name);
  Page::rec_packshort(rec, row.first_page);
  Page::rec_packshort(rec, row.last_page);
  Page::rec_packstr(rec, row.def);
  Page::rec_finish(rec);
}

void Table_pages::tbl_pack_col_type(std::string &rec, const std::string &tname, const column_type_t &colt)
{
  //|tname|colname|ord|type|size|
  Page::rec_begin(rec);
  Page::rec_packstr(rec, tname);
  Page::rec_packstr(rec, colt.name);
  Page::rec_packshort(rec, colt.ord);
  Page::rec_packshort(rec, colt.type);
  Page::rec_packshort(rec, colt.max_size);
  Page::rec_finish(rec);
}