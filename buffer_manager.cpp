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
	std::map<uint16_t, page_descriptor_t> buf_pool;  // map a uint16_t <page_id> to a page_descriptor_t containing the page ID, pointer to page, and dirty bit
}

/************************************ FUNCTION IMPLEMENTATIONS ***********************************/

namespace Buffer
{
	void initialize(uint16_t pool_sz)
	{
		std::cout << std::endl << "Initializing buffer manager..." << std::endl;

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


	void shutdown(file_descriptor_t &pfile)
	{
		std::cout << std::endl << "Shutting down buffer manager..." << std::endl;

		flush_all(pfile); // flush all pages to disk (file)

		/* Clear all the values in <lru_list> */
		for(int i = 0; i < buf_pool_size; i++)
		{
			lru_list[i] = 0;
		}

		buf_pool.clear(); // delete all pages from the pool

		buf_initialized = false; // uninitialize the buffer manager
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
		if(!buf_pool.at(page_id).dirty) // if the page is not dirty
		{
			std::cout << "Page " << page_id << " is not dirty." << std::endl;
			return;
		}
		else // page is dirty
		{
			std::cout << std::endl << "Flushing page " << page_id << "..." << std::endl;
			buf_pool.at(page_id).dirty = 0; // clear the dirty bit for the page
			num_dirty_pages--; // decrement the total number of dirty pages
			Page_file::pgf_write(pfile, page_id, buf_pool.at(page_id).page); // write the page to disk (file)
		}
	}


	void flush_all(file_descriptor_t &pfile)
	{
		std::cout << std::endl << "Flushing all pages from LRU cache..." << std::endl;
		for(int i = 0; i < buf_pool_size; i++)
		{
			if(lru_list[i] > 0) // if page is in LRU cache
			{
				if(buf_pool.at(lru_list[i]).dirty) // if page is dirty and needs to be written back to disk (file)
				{
					std::cout << "Flushing page " << lru_list[i] << "..." << std::endl;
					buf_pool.at(lru_list[i]).dirty = 0; // clear the dirty bit for the page
					num_dirty_pages--; // decrement the total number of dirty pages
					Page_file::pgf_write(pfile, lru_list[i], buf_pool.at(lru_list[i]).page); // write the page to disk (file)
				}
				else
				{
					std::cout << "Page " << lru_list[i] << " is not dirty." << std::endl;
					continue; // go to the next page in the buffer pool
				}
			}
			else // <lru_list> slot is free
			{
				break; // break out of the loop because rest of LRU slots will also be 0
			}
		}
	}


	void buf_write(file_descriptor_t &pfile, uint16_t page_id) // SHOULD ONLY SET THE DIRTY BIT
	{
		/* Check if page already exists in the buffer pool or not */
		if(buf_pool.count(page_id) > 0) // key of <page_id> exists in the buffer pool
		{
			validate_page_id(page_id); // make sure that key <page_id> is unique
			if(buf_pool.at(page_id).dirty) // if the page is already dirty
				throw buffer_error("Tried to set dirty bit for page that is already dirty.");
			else 
			{
				num_dirty_pages++; // increment the total number of dirty pages
				buf_pool.at(page_id).dirty = 1; // set the dirty bit for the page
				LRU_update(page_id); // update <lru_list>
			}
		}
		else // key of <page_id> does NOT exist in the buffer pool
			throw buffer_error("Page does not exist in the LRU cache buffer pool and therefore cannot be dirty.");
	}

	// BUF_READ MAY ALSO HAVE TO REPLACE A PAGE IN THE LRU LIST IF IT IS FULL AND FLUSH THAT INDIVIDUAL PAGE TO DISK (FILE)
	void* buf_read(file_descriptor_t &pfile, uint16_t page_id) // THIS FUNCTION SHOULD BE THE ONE THAT INSERTS AND BUFFERS THE PAGE IN THE BUFFER POOL
	{
		/* Check if page already exists in the buffer pool or not */
		if(buf_pool.count(page_id) > 0) // key <page_id> exists in the buffer pool
		{
			validate_page_id(page_id); // make sure that key <page_id> is unique
			LRU_update(page_id); // update the LRU cache before returning the pointer to the page
			return buf_pool.at(page_id).page; // return pointer to the page in LRU cache
		}
		else // key <page_id> does NOT exist in the buffer pool
		{
			void* read_buf; // define the page buffer
			read_buf = calloc(PAGE_SIZE, sizeof(char)); // allocate 16384 bytes
			Page_file::pgf_read(pfile, page_id, read_buf); // read the empty page from disk (file)
			page_descriptor_t page_desc = page_descriptor_t(page_id, read_buf, DEF_NOT_DTY); // create a new page descriptor
			buf_pool.insert(std::pair<uint16_t, page_descriptor_t>(page_id, page_desc)); // add page descriptor to LRU cache
			LRU_update(page_id); // update the LRU cache before returning the pointer to the page
			return read_buf; // return pointer to the empty page read from disk (file)
		}
	}


	uint16_t replace(file_descriptor_t &pfile, void* &page)
	{
		return 0;
	}


	bool full()
	{
		bool has_zero = true; // assume that LRU cache is not full by default
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
		std::cout << std::endl << "LRU list: " << '[';
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

	void validate_page_id(uint16_t page_id)
	{
		if(buf_pool.count(page_id) > 1)
			throw buffer_error("There should not be more than one page descriptor with the same page ID in the buffer pool.");
	}

	void print_buf_pool()
	{
		std::cout << std::endl << "Current state of the buffer pool..." << std::endl;
		for(std::map<uint16_t, page_descriptor_t>::iterator itr = buf_pool.begin(); itr != buf_pool.end(); itr++)
		{
		  std::cout << "Entry " << itr->first \
		  					<< " = {Key: " << itr->first \
		  					<< " => [Page ID: " << (itr->second).page_id \
		  					<< ", Page Address: " << (itr->second).page \
		  					<< ", Dirty: " << (itr->second).dirty \
		  					<< "]}" << std::endl;
		}
		std::cout << std::endl;
	}
}