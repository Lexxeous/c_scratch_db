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

		std::cout << "Page number should be 1: " << *(uint16_t*)page << std::endl;
		std::cout << "Record Length should be 48: " << rec->length << std::endl;
	}


	void pg_del_record(void* page, unsigned short rec_id)
	{
		
	}


	int pg_modify_record(void* page, void* record, BYTE rec_id)
	{
		return 0;
	}


	void rec_packint(std::string &buf, int val)
	{
		uint16_t old_size = buf.size();
		buf.resize(old_size + sizeof(uint16_t) + sizeof(int)); // 2 for length 'L', 2 for type 't', & 4 for int data 'd'

		BYTE* t_ptr = ((BYTE*)buf.data()) + sizeof(uint16_t); // move a pointer to the beginning of the type field 't'
		*t_ptr = 4;

		BYTE* d_ptr = ((BYTE*)buf.data() + sizeof(uint16_t) + sizeof(uint16_t)); // move a pointer to the beginning of the data field 'd'
		BYTE* arr = (BYTE*)&val; // assign each of the 4 bytes of <val> to an array of bytes
		for(int i = 0 ; i < 4; i++) {
			d_ptr[i] = arr[i]; // place the array of bytes one by ones
		}

		/* Check the integer value that was passed in */
		// std::cout << *(int*)arr << std::endl;

		/* Check the current contents of the string buffer */
		// for(int i = 0 ; i < buf.size(); i++) {
		// 	std::cout << (int)buf[i] << std::endl;
		// }
	}


	void rec_packstr(std::string &buf, const std::string &str)
	{
		uint16_t old_size = buf.size();
		buf.resize(old_size + sizeof(uint16_t) + str.size() + 9); // 2 for length 'L', 2 for type 't', & (original string length + 9) for data 'd'

		BYTE* t_ptr = ((BYTE*)buf.data()) + sizeof(uint16_t); // move a pointer to the beginning of the type field 't'
		*t_ptr = (str.size() + 9);

		BYTE* d_ptr = ((BYTE*)buf.data() + sizeof(uint16_t) + sizeof(uint16_t)); // move a pointer to the beginning of the data field 'd'
		memcpy(d_ptr, str.data(), str.size());

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