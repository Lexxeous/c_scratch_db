/**************************************************************************************************
* Filename:   paging_manager.cpp
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB paging implementation.
**************************************************************************************************/

/****************************************** HEADER FILES *****************************************/

#include "paging_manager.h"

/************************************** IMPLEMENT STRUCTURES *************************************/

// Struct for a single record.
// Provides easy access to the record's length and the address of the beginning of its data.
struct record_t {
  uint16_t length; // L
  BYTE data[1]; // record does not necessarily have at least one byte of data
};

// Struct for a single page.
// Provides easy access to the page data, directory size, and free bytes.
struct page_t {
  BYTE data[PAGE_SIZE - 2*sizeof(uint16_t)];  // includes data + directory
  uint16_t dir_size; // E
  uint16_t free_bytes; // F
}__attribute__((packed));

// Struct for the header page (page 0).
// Provides easy access to the signature, number of pages in the file, and pads for the rest of the page.
struct header_page_t {
  char sig[SIG_SIZE]; // "EAGL0"
  uint16_t num_pages; // NN
  BYTE rest[PAGE_SIZE - SIG_SIZE - sizeof(uint16_t)];
}__attribute__((packed));

/********************************************* MACROS ********************************************/

#define START_PAGE_DIR(page) ((BYTE*)&page->dir_size) - (page->dir_size)
#define END_LAST_REC(page) ((BYTE*)&page->dir_size) - (page->dir_size) - (page->free_bytes)

/************************************ FUNCTION IMPLEMENTATIONS ***********************************/

namespace Page_file
{
	void pgf_format(char fname[F_NAME_LEN], uint16_t &fsize)
	{
		if(strlen(fname) > F_NAME_LEN) // filename error checking
		{
			printf("ERROR: Filename too long. Limit to 200 characters.\n");
			exit(EXIT_FAILURE);
		}

		header_page_t h_page; // define the header page datatype
		strncpy(h_page.sig, "EAGL9", SIG_SIZE);
		h_page.num_pages = fsize;
		bzero(h_page.rest, PAGE_SIZE - SIG_SIZE - sizeof(uint16_t));

		std::fstream init_pages_file; // define the pages initialization file
		init_pages_file.open(fname, std::ios::out | std::ios::binary); // open the pages initialization file
		init_pages_file.write((char*) &h_page, sizeof(h_page));
		
		/* Initialize the data page(s) */
		for(uint16_t data_pages = 1; data_pages < fsize; data_pages++)
		{
			page_t data_page;
			bzero(data_page.data, PAGE_SIZE - (2*sizeof(uint16_t)));
			data_page.dir_size = 0;
			data_page.free_bytes = PAGE_SIZE - (2*sizeof(uint16_t));
			init_pages_file.write((char*) &data_page, sizeof(data_page));
		}

		init_pages_file.close(); // close the pages initialization file
	}


	void pgf_write(file_descriptor_t &pfile, int page_id, void *page_buf)
	{
		int16_t db_pg_start = ((PAGE_SIZE * page_id));
		pfile.seekg(db_pg_start, std::ios_base::beg);
		pfile.write(reinterpret_cast<char*>(page_buf), PAGE_SIZE);
	}


	void pgf_read(file_descriptor_t &pfile, int page_id, void* page_buf)
	{
		/* Do error checking */
		// pfile.fail()
		// pfile.bad()

		uint16_t db_pg_start = ((PAGE_SIZE * page_id));
		pfile.seekg(db_pg_start, std::ios_base::beg);
		pfile.read(reinterpret_cast<char*>(page_buf), PAGE_SIZE);

		/* Check the number of free bytes (F) and the directory size (E) */
		// page_t* r_page = (page_t*)page_buf;
		// std::cout << "Free Bytes: " << r_page->free_bytes << std::endl;
		// std::cout << "Dir Size: " << r_page->dir_size << std::endl;

		/* Write the entire page to a separate file for data checking */
		// std::fstream buf_file;
		// buf_file.open("page_buf.dat", std::ios::out | std::ios::binary);
		// buf_file.write(reinterpret_cast<char*>(page_buf), PAGE_SIZE);
		// buf_file.close();
	}


	void print(char fname[F_NAME_LEN])
	{
		
	}
} // End of "Page_file" namespace


namespace Page
{
	void pg_compact(void* page_buf, uint16_t num_bytes, BYTE* start)
	{
		
	}


	void pg_expand(void* page_buf, uint16_t num_bytes, BYTE* start)
	{
		
	}


	void pg_add_record(void* page, void* record)
	{
		record_t* rec = (record_t*)record;
		uint16_t page_num = *(uint16_t*)page;

		std::cout << "Page number should be 1: " << *(uint16_t*)page << std::endl;
		std::cout << "Record Length should be 59: " << rec->length << std::endl;

		void* page_buf; // define the page buffer
		page_buf = calloc(PAGE_SIZE, sizeof(char));
		std::fstream pfile; // define the DB file for reading and writing
		pfile.open("test_db.dat"); // open the pages file
		Page_file::pgf_read(pfile, page_num, page_buf);

		page_t* edit_page = (page_t*)page_buf;
		BYTE* add_rec_ptr = END_LAST_REC(edit_page);

		if(edit_page->free_bytes >= rec->length) { // if there is enough room for the record to be added
			memcpy(add_rec_ptr, rec, rec->length); // append/copy the record to the end of the current set of page records
		}
		else {
			printf("Not enough space for this record on page %hi.\n",page_num);
		}

		edit_page->dir_size += sizeof(uint16_t); // update E
		edit_page->free_bytes = edit_page->free_bytes - rec->length - (uint16_t)sizeof(uint16_t); // update F
		uint16_t old_num_dir_entries = (((edit_page->dir_size)/2) - 1);
		uint16_t new_num_dir_entires = old_num_dir_entries + 1;


		/* Update the record directory for the newly added record */
		rec_offset_t* dir_ptr = (rec_offset_t*)START_PAGE_DIR(edit_page);
		rec_offset_t* dir_arr = new rec_offset_t[new_num_dir_entires];
		std::cout << "The number of offsets currently in the directory: " << old_num_dir_entries << std::endl;
		memcpy(dir_arr, dir_ptr + 1, old_num_dir_entries);
		rec_offset_t* end_of_recs = (rec_offset_t*)END_LAST_REC(edit_page);
		rec_offset_t* rec_offset = (rec_offset_t*)end_of_recs - rec->length;
		std::cout << "Offset for the newly added record is: " << *rec_offset << std::endl;
		std::cout << "dir_arr[0]: " << dir_arr[0] << std::endl;
		dir_arr[old_num_dir_entries] = *rec_offset;
		memcpy(dir_ptr, dir_arr, new_num_dir_entires);
		delete[] dir_arr;


		/* Output the currently read page to check for changes */
		std::fstream buf_file;
		buf_file.open("page_buf.dat", std::ios::out | std::ios::binary);
		buf_file.write(reinterpret_cast<char*>(edit_page), PAGE_SIZE);
		buf_file.close();

		pfile.close(); // close the pages file
	}


	void pg_del_record(void* page, uint16_t rec_id)
	{
		
	}


	int pg_modify_record(void* page, void* record, BYTE rec_id)
	{
		return 0;
	}


	void rec_packint(std::string &buf, int val)
	{
		uint16_t type = 4; // define the DB type for a signed integer
		BYTE* t_arr = (BYTE*)&type; // convert <type> into an array of 2 bytes
		BYTE* d_arr = (BYTE*)&val; // convert <val> into an array of 4 bytes

		for(int i = 0; i < sizeof(uint16_t); i++) {
			buf += t_arr[i]; // append the 2 bytes of <type> to <buf>
		}
		for(int j = 0; j < sizeof(int); j++) {
			buf += d_arr[j]; // append the 4 bytes of <val> to <buf>
		}

		/* Check the integer value that was passed in */
		// std::cout << *(int*)d_arr << std::endl;

		/* Check the current contents of the buffer */
		// for(int i = 0 ; i < buf.size(); i++) {
		// 	std::cout << (int)buf[i] << std::endl;
		// }
	}


	void rec_packstr(std::string &buf, const std::string &str)
	{
		uint16_t type = str.size() + 9; // define the DB type for a string
		BYTE* t_arr = (BYTE*)&type; // convert <type> into an array of 2 bytes

		for(int t = 0; t < sizeof(uint16_t); t++) {
			buf += t_arr[t]; // append the 2 bytes of <type> to <buf>
		}
		for(int d = 0; d < str.size(); d++) {
			buf += str[d]; // append each character from <str> to <buf> in order
		}

		/* Check the current contents of the string buffer */
		// for(int i = 0 ; i < buf.size(); i++) {
		// 	std::cout << (char)buf[i] << std::endl;
		// }
	}


	int rec_upackint(void* buf, uint16_t &next)
	{
		return 0;
	}


	int rec_upackstr(void* buf, uint16_t &next, std::string val)
	{
		return 0;
	}


	void rec_finish(std::string &buf)
	{
		uint16_t tot_buf_len = buf.size();
		BYTE* l_ptr = (BYTE*)buf.data();
		*l_ptr = tot_buf_len;

		/* Check the current contents of the string buffer */
		// for(int i = 0 ; i < buf.size(); i++) {
		// 	std::cout << (uint16_t)buf[i] << std::endl;
		// }
	}


	void rec_begin(std::string &buf)
	{
		buf.resize(sizeof(uint16_t)); // start the record buffer by creating enough space for 'L'
	}
}