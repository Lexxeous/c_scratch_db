/**************************************************************************************************
* Filename:   c_scratch_db.h
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB implementation.
**************************************************************************************************/

/****************************************** HEADER FILES *****************************************/

#include "c_scratch_db.h"

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
		
		// Initialize the data page(s)
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


	// void pgf_write(file_descriptor_t pfile, int page_id, void *page_buf)
	// {
		
	// }


	void pgf_read(file_descriptor_t &pfile, int page_id, void* page_buf)
	{
		// uint16_t db_pg_start = ((PAGE_SIZE * page_id));
		// pfile.seekg(db_pg_start, std::ios_base::beg);
		
		// pfile.fail()
		// pfile.bad()
		// pfile.read(reinterpret_cast<char*>(page_buf), PAGE_SIZE);
	}
} // End of "Page_file" namespace


// namespace Page
// {
// 	void pg_compact(void *page_buf, uint16_t num_bytes, BYTE *start)
// 	{
		
// 	}


	// void pg_expand(void *page_buf, uint16_t num_bytes, BYTE *start)
	// {
		
	// }


	// void pg_add_record(void *page, void *record)
	// {
		
	// }


	// void pg_del_record(void *page, unsigned short rec_id)
	// {
		
	// }


	// int pg_modify_record(void *page, void *record, BYTE rec_id)
	// {
		
	// }


	// void rec_packint(std::string &buf, int val)
	// {
		
	// }


	// void rec_packstr(std::string &buf, const std::string &str)
	// {
		
	// }


	// int rec_upackint(void *buf, unsigned short &next)
	// {
		
	// }


	// int rec_upackstr(void *buf, unsigned short &next, std::string val)
	// {
		
	// }


	// void rec_finish(std::string &buf)
	// {
		
	// }


	// void rec_begin(std::string &buf, unsigned short &next)
	// {
		
	// }
// }