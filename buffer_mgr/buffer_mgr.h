#ifndef BUFFER_MGR_H
#define BUFFER_MGR_H

#include "../paging/paging.h"

#include <climits>
#include <map>
#include <string>
#include <list>

namespace Buffer_mgr {
  // Note: all locking is handles by the lock manager, not here.

  struct buffer_descriptor_t {
    //buffer_descriptor_t(uint16_t pid, void *pg, bool dty) : page(pg), dirty(dty), page_id(pid) {}
    buffer_descriptor_t(uint16_t pgid, void *pg, bool dty) : page(pg), dirty(dty), page_id(pgid) {}
    buffer_descriptor_t() : page(0), dirty(false) {}
    void *page;
    bool dirty;
    uint16_t page_id;
    // need pinned
  };

  class buffering_error : public std::runtime_error {
  public:
    buffering_error(std::string what) : std::runtime_error(what) {}
    buffering_error(const char *what) : std::runtime_error(what) {}
  };

  std::list<buffer_descriptor_t>::iterator find(uint16_t page_id);
  
  void initialize(uint16_t pool_sz);
  void shutdown(file_descriptor_t &pfile);

  void LRU_update(uint16_t page_id);
  void LRU_Remove(uint16_t page_id);

  //void *fetch(uint16_t page_id);
  void flush(file_descriptor_t &pfile, uint16_t page_id);
  void flush_all(file_descriptor_t &pfile);

  // Write a page in memory to disk.
  void buf_write(file_descriptor_t &pfile, int page_id);
  // Read a page from disk to memory
  void *buf_read(file_descriptor_t &pfile, int page_id);

  uint16_t replace(file_descriptor_t &pfile);
  bool full(); 

}; // namespace Buffer_mgr
#endif // BUFFER_MGR_H