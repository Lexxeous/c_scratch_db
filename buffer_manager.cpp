/**************************************************************************************************
* Filename:   paging_manager.cpp
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB paging implementation.
**************************************************************************************************/

/****************************************** HEADER FILES *****************************************/

#include "paging/paging.h"
#include "buffer_manager.h"

/************************************** IMPLEMENT STRUCTURES *************************************/



/********************************************* MACROS ********************************************/



/************************************ FUNCTION IMPLEMENTATIONS ***********************************/

namespace Buf
{
	void initialize(uint16_t pool_sz)
	{
		buf_pool_size = pool_sz; // initialize the global buffer pool size variable

		/* Make sure that there are no garbage values in the LRU list */
		for(int i = 0; i < buf_pool_size; i++)
			lru_list[i] = 0;

		/* Throw an error if there are dirty pages that have not been written back to disk */
		if(num_dirty_pages != 0)
			throw buffer_error("Cannot initialize record buffer pool. There are still dirty pages that have not been written back to disk.");
	}


	void shutdown(file_descriptor_t &pfile)
	{

	}


	void LRU_update(uint16_t page_id)
	{

	}


	void flush(file_descriptor_t &pfile, uint16_t page_id)
	{

	}


	void flush_all(file_descriptor_t &pfile)
	{

	}


	void buf_write(file_descriptor_t &pfile, int page_id)
	{

	}


	// void* buf_read(file_descriptor_t &pfile, int page_id)
	// {

	// }


	uint16_t replace(void* &page)
	{
		return 0;
	}


	bool full()
	{
		return false;
	}

	void print_lru_list()
	{
		std::cout << '[';
		for(int i = 0; i < buf_pool_size; i++)
		{
			if(i < buf_pool_size - 1){std::cout << lru_list[i] << ", ";}
			else{std::cout << lru_list[i];}
		}
		std::cout << ']' << std::endl;
	}
}