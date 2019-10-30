#include "buffer_mgr.h"
#include <memory>

namespace Buffer_mgr {
  std::map<uint16_t, buffer_descriptor_t> page_pool;
  uint16_t pool_size;
  bool initialized = false;
  uint16_t num_dirty = 0;
  std::list<buffer_descriptor_t> LRU; // timestamp, page_id
  bool full () {
    return (page_pool.size() >= pool_size);
  }
};                                    // namespace Buffer_mgr

void Buffer_mgr::initialize(uint16_t pool_sz) {
  // Dirty pages should be flushed before initializing
  if (num_dirty) {
    throw buffering_error(
        "Attempted to intialize buffering, but dirty pages already exist.");
  }
  // set the max pool size (number of pages that can be buffered)
  pool_size = pool_sz;
}
void Buffer_mgr::shutdown(file_descriptor_t &pfile) {
  // Write all dirty pages to secondary storage
  flush_all(pfile);
  // reset the page pool
  for (auto pg : page_pool) {
    delete[] (char *) pg.second.page;
  }
  page_pool.clear();
  // remove all LRU history
  LRU.clear();
  // Indicate that the buffer manager is not initialized
  initialized = false;
}

std::list<Buffer_mgr::buffer_descriptor_t>::iterator
Buffer_mgr::find(uint16_t page_id) {
  // Loop throug the LRU history and fine the page with the id
  for (auto it = LRU.begin(); it != LRU.end(); it++) {
    if (it->page_id == page_id) {
      return it;
    }
  }

  // Return end() if the page was not found
  return LRU.end();
}

void Buffer_mgr::LRU_Remove(uint16_t page_id) {
  // find if the page is in the LRU queue (should be unique)
  auto lit = find(page_id);
  if (lit != LRU.end()) {
    LRU.erase(lit);
  }
}

void Buffer_mgr::LRU_update(uint16_t page_id) {

  // find if the page is in the LRU queue (should be unique)
  auto lit = find(page_id);

  // If found, update it (remove then insert at the beginning)
  buffer_descriptor_t bd;
  if (lit != LRU.end()) {
    bd = *lit;
    LRU.erase(lit);
  } else {
    // page not in LRU history, to add it to the front (as most recently used)
    auto it = page_pool.find(page_id);
    if (it == page_pool.end()) {
      throw buffering_error("LRU update of page that is not buffered");
    }
    bd = it->second;
  }
  LRU.push_front(bd);
}

void Buffer_mgr::flush(file_descriptor_t &pfile, uint16_t page_id) {
  std::map<uint16_t, buffer_descriptor_t>::iterator it =
      page_pool.find(page_id);
  if (it == page_pool.end()) {
    throw buffering_error("can not flush a page that has not been buffered.");
  }
  if (it->second.dirty) {
    Page_file::pgf_write(pfile, page_id, it->second.page);
    it->second.dirty = false;
    num_dirty--;
  }
  // LRU_Remove(page_id);  // What should we do with a flushed page?
}

void Buffer_mgr::flush_all(file_descriptor_t &pfile) {
  if (!num_dirty) {
    return;
  }
  for (std::pair<uint16_t, buffer_descriptor_t> one : page_pool) {
    if (one.second.dirty) {
      Page_file::pgf_write(pfile, one.first, one.second.page);
      one.second.dirty = false;
      num_dirty--;
    }
  }
}

void Buffer_mgr::buf_write(file_descriptor_t &pfile, int page_id) {
  auto it = page_pool.find(page_id);
  if (it != page_pool.end()) {
    it->second.dirty = true;
    num_dirty++;
  } else {
    throw buffering_error("Cannot write to page that is not buffered");
  }
}
// Read a page from disk to memory
void *Buffer_mgr::buf_read(file_descriptor_t &pfile, int page_id) {
  // see if it is already in the buffer pool
  std::map<uint16_t, buffer_descriptor_t>::iterator it =
      page_pool.find(page_id);

  void *retval = 0; 
  if (it != page_pool.end()) {
    retval = it->second.page;
  } else {
    retval = (void *) new BYTE[PAGE_SIZE];
    std::unique_ptr<char> cleanup((char *) retval);
    // it was not buffered, so read it:

    Page_file::pgf_read(pfile, page_id, retval);

    // if the buffer pool is full, get rid of an old page
    if (full()) {
      replace(pfile);
    }
    page_pool.insert(std::pair<uint16_t, buffer_descriptor_t>(
        page_id, buffer_descriptor_t(page_id, retval, false)));

    cleanup.release();
  }
  LRU_update(page_id);

  return retval;
}

uint16_t Buffer_mgr::replace(file_descriptor_t &pfile) {
  // get the id of the oldest page
  int16_t retval = LRU.back().page_id;

  // remove the page from the LRU history
  LRU.pop_back();

  // delete the page from the page pool
  std::map<uint16_t, buffer_descriptor_t>::iterator it = page_pool.find(retval);
  if (it == page_pool.end()) {
    throw buffering_error("Page in LRU does not exist in pool");
  }

  // flush the page first
  flush(pfile, retval);

  delete[] (char *) it->second.page;

  page_pool.erase(it);

  return retval;
}