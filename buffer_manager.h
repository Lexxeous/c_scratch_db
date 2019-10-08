/**************************************************************************************************
* Filename:   buffer_manager.h
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB buffer implementation.
**************************************************************************************************/

/********************************************* GUARD *********************************************/

#ifndef BUFFER_MANAGER
#define BUFFER_MANAGER

/***************************************** HEADER FILES ******************************************/

#include "paging/paging.h"

/****************************************** DEFINITIONS ******************************************/

#define DEF_NOT_DTY false // default clear dirty bit for <page_descriptor_t> constructor

/************************************** IMPLEMENT STRUCTURES *************************************/

namespace Buffer
{
	struct page_descriptor_t // additional data about a DB page
	{
		page_descriptor_t(uint16_t id, void* pg, bool dty) : page_id(id), page(pg), dirty(dty){}; // initial constructor
		// page_descriptor_t() : page(0), dirty(false){}; // default constructor
		uint16_t page_id; // page ID (0..N)
		void* page; // pointer to the starting address of the page
		bool dirty; // dirty bit ; if dirty == true, contents in RAM differ from disk and must be written back, else contents are the same
	};
}

/************************************** FUNCTION PROTOTYPES **************************************/

namespace Buffer
{
	class buffer_error : public std::runtime_error // runtime error function(s) to indicate a failure at the buffering layer
	{
  	public:
	    buffer_error(std::string what) : std::runtime_error(what) {}
	    buffer_error(const char* what) : std::runtime_error(what) {}
  };

	/* Set the buffer pool size to pool_sz. Note that you will want to throw an error if the number of dirty pages is not zero. */
	void initialize(uint16_t pool_sz);

	/* Flush all the pages to pfile and delete them from the pool. Also clear the LRU list and indicate that the buffer manager is not initialized. */
	void shutdown(file_descriptor_t &pfile);

	/* If the page is in the LRU list, then remove it and add it to the front. Otherwise just add it to the front. */
	void LRU_update(uint16_t page_id);

	/* Write the page with its <page_id> to the file <pfile> if it is dirty. Indicate that the page is no longer dirty. */
	void flush(file_descriptor_t &pfile, uint16_t page_id);

	/* Write all the pages that are buffered and dirty to the file pfile. Indicate that the flushed pages are no longer dirty. */
	void flush_all(file_descriptor_t &pfile);

	/* Do not actually write the page, but indicate that it is dirty. */
	void buf_write(file_descriptor_t &pfilebuffer, uint16_t page_id);

	/* If the page with id page_id is already in the buffer, then update its LRU list and return a pointer to its buffer. Otherwise, read it from pfile, update its LRU list, and return a pointer to its buffer. You may have to do page replacement. */
	void* buf_read(file_descriptor_t &pfile, uint16_t page_id);

	/* Find a page to replace in the LRU list. Flush it if necessary and remove it from the buffer pool and the LRU list. Put its address in page, and return its page_id. */
	uint16_t replace(file_descriptor_t &pfile, void* &page);

	/* Helper function that returns true if the buffer pool is full. */
	bool full();

	/* Helper function that prints the current state of <lru_list> for debugging within "driver.cpp". */
	void print_lru_list();

	/* Helper function for the driver to get the current value of <buf_pool_size> for debugging within "driver.cpp". */
	uint16_t get_buf_pool_size();

	/* Helper function for the driver to get the current value of <num_dirty_pages> for debugging within "driver.cpp". */
	uint16_t get_num_dirty_pages();

	/* Helper function for the driver to get the current value of <buf_initialized> for debugging within "driver.cpp". */
	bool get_buf_initialized();

	/* Helper function for the driver to get the current value of "full()" for debugging within "driver.cpp". */
	bool get_buf_full();

	/* Helper function that throws an error if a page ID exists more than once in the buffer pool. */
	void validate_page_id(uint16_t page_id);

	/* Helper function that prints the current state of <buf_pool> */
	void print_buf_pool();
}

#endif // BUFFER_MANAGER
