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

/*********************************** NAMESPACED/GLOBAL VARIABLES *********************************/

namespace Buffer
{
	/* Define "global" namespaced variables ; parameters regarding the current state of the buffer manager layer */
	uint16_t buf_pool_size;
	uint16_t num_dirty_pages;
	uint16_t* lru_list;
	bool buf_initialized;
	bool buf_full;
	std::map<uint16_t, page_descriptor_t> buf_pool;
}

/************************************ FUNCTION IMPLEMENTATIONS ***********************************/

namespace Buffer
{
	void initialize(uint16_t pool_sz)
	{
		/* Initialize the buffer manager layer's parameters */
		buf_pool_size = pool_sz; // initialize the global buffer pool size variable
		// num_dirty_pages = 0; // DONT INITIALIZE <num_dirty_pages> TO 0, OR THE FUNCTION WILL NOT BE ABLE THROW AN ERROR
		lru_list = (uint16_t*)calloc(buf_pool_size, sizeof(uint16_t)); 
		buf_initialized = false;
		buf_full = false;

		/* Make sure that there are no garbage values in the LRU list */
		for(int i = 0; i < buf_pool_size; i++)
			lru_list[i] = 0;

		/* Throw an error if there are dirty pages that have not been written back to disk */
		if(num_dirty_pages != 0)
			throw buffer_error("Cannot initialize record buffer pool. There are still dirty pages that have not been written back to disk.");

		buf_initialized = true;
	}


	void shutdown(/*file_descriptor_t &pfile*/)
	{
		buf_initialized = false;

		/* clear all the values in <lru_list> */
		for(int i = 0; i < buf_pool_size; i++)
		{
			lru_list[i] = 0;
		}
	}


	void LRU_update(uint16_t page_id)
	{
		bool in_list = false; // assume that <page_id> wont be found in <lru_list> by default
		int pg_id_idx; // index of <page_id> if it already exists in <lru_list>

		for(int i = 0; i < buf_pool_size; i++)
		{
			if(lru_list[i] == page_id) // <page_id> currently exists in <lru_list>
			{
				in_list = true;
				pg_id_idx = i; // get the index where the <page_id> was "found"
				break;
			}
		}

		if(in_list) // <page_id> was found in <lru_list>
		{
			for(int j = pg_id_idx; j > 0; j--) // loop throught <lru_list> backwards from the "found" index
				lru_list[j] = lru_list[j-1]; // copy all the elements towards the back but leave index [0] alone
		}
		else // <page_id> was NOT found in <lru_list>
		{
			for(uint16_t k = (buf_pool_size - 1); k > 0; k--) // loop through <lru_list> backwards
				lru_list[k] = lru_list[k-1]; // copy all the elements towards the back but leave index [0] alone
		}
		lru_list[0] = page_id; // set the first element in <lru_list> to <page_id>
	}


	void flush(file_descriptor_t &pfile, uint16_t page_id)
	{

	}


	void flush_all(file_descriptor_t &pfile)
	{

	}


	void buf_write(file_descriptor_t &pfile, uint16_t page_id)
	{	
		page_descriptor_t write_page_d = buf_pool[page_id]; // get the page descriptor from the buffer pool

		if(write_page_d.dirty) // if the page is already dirty
		{
			throw buffer_error("Tried to set dirty bit for page that is already dirty.");
		}
		else
		{
			write_page_d.dirty = 1; // set the dirty bit for the page
			num_dirty_pages++; // increment the total number of dirty pages
			LRU_update(page_id); // update <lru_list>
		}
	}


	// void* buf_read(file_descriptor_t &pfile, uint16_t page_id)
	// {

	// }


	uint16_t replace(file_descriptor_t &pfile, void* &page)
	{
		return 0;
	}


	bool full()
	{
		bool has_zero = true;
		for(int i = 0; i < buf_pool_size; i++)
		{
			if(lru_list[i] == 0) // there is at least 1 zero (empty slot) in <lru list>
				return false;
			else // there are NO zeros (empty slots) in <lru_list>
				has_zero = false;
		}

		return !has_zero;
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

	uint16_t get_buf_pool_size()
	{
		return buf_pool_size;
	}

	uint16_t get_num_dirty_pages()
	{
		return num_dirty_pages;
	}

	bool get_buf_initialized()
	{
		return buf_initialized;
	}

	bool get_buf_full()
	{
		buf_full = full();
		return buf_full;
	}
}