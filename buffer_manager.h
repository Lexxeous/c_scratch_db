/**************************************************************************************************
* Filename:   buffer_manager.h
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB paging implementation.
**************************************************************************************************/

/********************************************* GUARD *********************************************/

#ifndef BUFFER_MANAGER
#define BUFFER_MANAGER

/***************************************** HEADER FILES ******************************************/

#include "paging_manager.h"

/****************************************** DATA TYPES *******************************************/



/****************************************** DEFINITIONS ******************************************/



/******************************************* CONSTANTS *******************************************/



/************************************** FUNCTION PROTOTYPES **************************************/

// Set the buffer pool size to sz. Note that you will want to throw an error if the number of dirty pages is not zero.
void initialize(uint16_t pool_sz);

// Flush all the pages to pfile and delete them from the pool. Also clear the LRU list and indicate that the buffer manager is not initialized.
void shutdown(file_descriptor_t &pfile);

// If the page is in the LRU list, then remove it and add it to the front. Otherwise just add it to the front.
void LRU_update(uint16_t page_id);

// Write the page with is page_id to the file pfile if it is dirty. Indicate that the page is no longer dirty.
void flush(file_descriptor_t &pfile, uint16_t page_id);

// Write all the pages that are buffered and dirty to the file pfile. Indicate that the flushed pages are no longer dirty.
void flush_all(file_descriptor_t &pfile);

// Do not actually write the page, but indicate that it is dirty.
void buf_write(file_descriptor_t &pfile, int page_id);

// If the page with id page_id is already in the buffer, then update its LRU list and return a pointer to its buffer. Otherwise, read it from pfile, update its LRU list, and return a pointer to its buffer. You may have to do page replacement.
void* buf_read(file_descriptor_t &pfile, int page_id);

// Find a page to replace in the LRU list. Flush it if necessary and remove it from the buffer pool and the LRU list. Put its address in page, and return its page_id.
uint16_t replace(void* &page);

// Helper function that returns true if the buffer pool is full.
bool full(); 

#endif // BUFFER_MANAGER
