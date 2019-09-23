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
  uint16_t dir_size; // E ; the absolute number of entries/records
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

#define START_PAGE_DIR(page) (BYTE*)(((BYTE*)&page->dir_size) - ((page->dir_size)*2))
#define END_LAST_REC(page) (BYTE*)(((BYTE*)&page->dir_size) - ((page->dir_size)*2) - (page->free_bytes))

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
		strncpy(h_page.sig, "EAGL0", SIG_SIZE);
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
		uint16_t db_pg_start = ((PAGE_SIZE * page_id));
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


	void print_records(file_descriptor_t &pfile)
	{
		void* page_buf; // define the page buffer
		page_buf = calloc(PAGE_SIZE, sizeof(char)); // allocate 16384 bytes

		/* Get the number of pages from the header page */
		Page_file::pgf_read(pfile, 0, page_buf);
		header_page_t* h_page = (header_page_t*)page_buf;
		uint16_t page_count = (uint16_t)h_page->num_pages;
		std::cout << "\nDB pages: " << page_count << std::endl << std::endl;
		std::cout << "Page 0 (Header Page):" << std::endl << "  ..." << std::endl;

		/* Print the contents of each data page */
		for(int i = 1; i < page_count; i++)
		{
			/* Get the array of record offsets */
			Page_file::pgf_read(pfile, i, page_buf); // read the current page
			page_t* d_page = (page_t*)page_buf; // cast as a header page
			uint16_t num_entries = d_page->dir_size;
			rec_offset_t* dir_ptr = (rec_offset_t*)START_PAGE_DIR(d_page);
			rec_offset_t* dir_arr = new rec_offset_t[num_entries];
			memcpy(dir_arr, dir_ptr, num_entries*2); // multiplied by 2 to account for moving only bytes and not uint16_ts

			/* Print the record offsets for each page */
			std::cout << "Page " << i << ": " << std::endl;

			if(d_page->dir_size == 0) // if there are no records to print
			{
				std::cout << "  ..." << std::endl;
			}
			else // if there are records to print
			{
				std::cout << "  Offsets: ";
				for(int j = 0; j < num_entries; j++)
				{
					std::cout << (uint16_t)dir_arr[j] << " | ";
				}
				std::cout << std::endl;

				/* Loop through all records in page <i> */
				for(int k = 0; k < num_entries; k++) // for all offsets in the page
				{
					if(dir_arr[k] != RECORD_UNUSED) // if the record exists
					{
						uint16_t ref_next = 2;
						uint16_t &next = ref_next;
						std::string val;

						BYTE* cur_off = (BYTE*)((BYTE*)d_page + dir_arr[k]);
						record_t* cur_rec = (record_t*)cur_off;

						int val_1 = Page::rec_upackint((void*)cur_rec, next);
						int str_len = Page::rec_upackstr((void*)cur_rec, next, val);
						int val_2 = Page::rec_upackint((void*)cur_rec, next);

						std::cout << "  Record " << k << ":  int: " << val_1 << "  string: " << val << "  int: " << val_2 << std::endl;
					}
				}
			}
			delete[] dir_arr;
		}
		std::cout << "END" << std::endl;
	}
} // End of "Page_file" namespace


namespace Page
{
	void pg_compact(void* page_buf, uint16_t num_bytes, BYTE* start)
	{
		record_t* compact_record = (record_t*)start; // destination (dest)
		BYTE* compact_record_end = (BYTE*)((BYTE*)compact_record + compact_record->length); // source (src)
		memmove((BYTE*)compact_record, compact_record_end, num_bytes);
	}


	void pg_expand(void* page_buf, uint16_t num_bytes, BYTE* start)
	{
		// record_t* expand_record = (record_t*)start; // destination (dest)
		// BYTE* expand_record_end = (BYTE*)((BYTE*)expand_record + expand_record->length); // source (src)
		// memmove((BYTE*)expand_record, expand_record_end, num_bytes);
	}


	uint16_t pg_add_record(void* page, void* record)
	{
		record_t* rec = (record_t*)record;
		page_t* edit_page = (page_t*)page;
		BYTE* add_rec_ptr = END_LAST_REC(edit_page);

		if(edit_page->free_bytes >= rec->length) // if there is enough room for the record to be added
		{ 
			memcpy(add_rec_ptr, rec, rec->length); // append/copy the record to the end of the current set of page records

			edit_page->dir_size += 1; // add one to the number of entries/records ('E')
			edit_page->free_bytes = edit_page->free_bytes - rec->length - (uint16_t)sizeof(uint16_t); // update 'F'
			uint16_t old_num_dir_entries = (edit_page->dir_size - 1);
			uint16_t new_num_dir_entires = edit_page->dir_size;

			/* Update the record directory for the newly added record */
			rec_offset_t* dir_ptr = (rec_offset_t*)START_PAGE_DIR(edit_page);
			rec_offset_t* dir_arr = new rec_offset_t[new_num_dir_entires];
			memcpy(dir_arr, dir_ptr + 1, old_num_dir_entries*2); // multiplied by 2 to account for moving only bytes and not uint16_ts
			BYTE* end_of_recs = END_LAST_REC(edit_page);
			BYTE* rec_offset = (BYTE*)(end_of_recs - rec->length);
			dir_arr[old_num_dir_entries] = (rec_offset - (BYTE*)edit_page); // offset relative to beginning of <edit_page>
			memcpy((BYTE*)dir_ptr, (BYTE*)dir_arr, new_num_dir_entires*2); // multiplied by 2 to account for moving only bytes and not uint16_ts
			delete[] dir_arr;

			/* Output the currently read page to check for changes */
			// std::fstream buf_file;
			// buf_file.open("page_buf.dat", std::ios::out | std::ios::binary);
			// buf_file.write(reinterpret_cast<char*>(edit_page), PAGE_SIZE);
			// buf_file.close();

			return old_num_dir_entries;
		}
		else // not enough space to add record
		{
			std::cout << "ERROR: Not enough space for this record." << std::endl;
			exit(EXIT_FAILURE);
		}
	}


	void pg_del_record(void* page, uint16_t rec_id)
	{
		page_t* del_rec_page = (page_t*)page;
		BYTE* end_of_recs = END_LAST_REC(del_rec_page);
		uint16_t num_entries = del_rec_page->dir_size;
		bool last_rec;

		/* If no records in page */
		if(num_entries == 0)
		{
			std::cout << "ERROR: No records available to delete." << std::endl;
			exit(EXIT_FAILURE);
		}

		/* Determine if the record to be deleted is the last one or not */
		if((rec_id + 1) == num_entries) // if <rec_id> identifies the last record in the page
			last_rec = true;
		else
			last_rec = false;

		/* Get the array of record offsets */
		rec_offset_t* dir_ptr = (rec_offset_t*)START_PAGE_DIR(del_rec_page);
		rec_offset_t* dir_arr = new rec_offset_t[num_entries];
		memcpy((BYTE*)dir_arr, (BYTE*)dir_ptr, num_entries*2); // multiplied by 2 to account for moving only bytes and not uint16_ts

		/* Get the length of the current record */
		rec_offset_t cur_rec_offset = dir_arr[rec_id];
		BYTE* del_beg_rec = (BYTE*)((BYTE*)del_rec_page + cur_rec_offset); // get pointer to the beginning of the record (dest)
		record_t* del_record = (record_t*)del_beg_rec;

		/* Delete record or not? */
		if(rec_id >= num_entries || (uint16_t)cur_rec_offset == RECORD_UNUSED) // record does not exist in the directory
		{
			std::cout << "ERROR: Record " << rec_id << " does not exist." << std::endl;
			exit(EXIT_FAILURE);
		}
		else // record exists in the directory
		{
			if(!last_rec) // if we want to delete a record in the middle of other records
			{
				uint16_t rec_len_behind = 0;
				for(int r = 0; r < rec_id; r++)
				{
					if(dir_arr[r] != RECORD_UNUSED)
					{
						BYTE* cur_off = (BYTE*)((BYTE*)del_rec_page + dir_arr[r]);
						record_t* cur_rec = (record_t*)cur_off;
						rec_len_behind += cur_rec->length; // sum the lengths of the records behind the one to be deleted
					}
				}

				BYTE* del_end_rec = (BYTE*)((BYTE*)del_beg_rec + del_record->length); // get pointer to the end of the record (src)
				uint16_t num_bytes = PAGE_SIZE - 2*sizeof(uint16_t) - 2*(num_entries) - del_rec_page->free_bytes - del_record->length - rec_len_behind;
				
				Page::pg_compact(del_rec_page, num_bytes, del_beg_rec);
				dir_arr[rec_id] = (uint16_t)RECORD_UNUSED; // set current record offset to RECORD_UNUSED
				for(int d = rec_id + 1; d <= num_entries - 1; d++) // for all offsets after the current
				{
					if(dir_arr[d] != RECORD_UNUSED)
						dir_arr[d] -= del_record->length; // sutract the length of the deleted record
				}
				memmove((BYTE*)dir_ptr, (BYTE*)dir_arr, num_entries*2); // rewrite the directory with updated offsets
			}
			else // if we want to delete the last record in the page
			{
				dir_arr[rec_id] = (uint16_t)RECORD_UNUSED;
				memmove((BYTE*)dir_ptr, (BYTE*)dir_arr, num_entries*2); // rewrite the directory with the RECORD_UNUSED value
				bzero(del_beg_rec, del_record->length); // zeros out the last record
			}

			del_rec_page->free_bytes = del_rec_page->free_bytes + del_record->length; // add length of record to number of free bytes
		}
		delete[] dir_arr;
	}


	int pg_modify_record(void* page, void* record, uint16_t rec_id)
	{
		return 0;
	}


	void rec_packint(std::string &buf, int val)
	{
		uint16_t type = 4; // define the DB type for a signed integer
		BYTE* t_arr = (BYTE*)&type; // convert <type> into an array of 2 bytes
		BYTE* d_arr = (BYTE*)&val; // convert <val> into an array of 4 bytes

		for(int i = 0; i < sizeof(uint16_t); i++)
		{
			buf += t_arr[i]; // append the 2 bytes of <type> to <buf>
		}
		for(int j = 0; j < sizeof(int); j++)
		{
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

		for(int t = 0; t < sizeof(uint16_t); t++)
		{
			buf += t_arr[t]; // append the 2 bytes of <type> to <buf>
		}
		for(int d = 0; d < str.size(); d++)
		{
			buf += str[d]; // append each character from <str> to <buf> in order
		}

		/* Check the current contents of the string buffer */
		// for(int i = 0 ; i < buf.size(); i++) {
		// 	std::cout << (char)buf[i] << std::endl;
		// }
	}


	int rec_upackint(void* buf, uint16_t &next)
	{
		/* Define pointers */
		int* val_ptr;
		uint16_t* type;
		BYTE* arr = (BYTE*)buf;

		type = (uint16_t*)((BYTE*)arr + next); // get the type

		if(type[0] == 4) // if integer
		{
			val_ptr = (int*)((BYTE*)arr + next + sizeof(uint16_t));
			next += 6; // 2 bytes for type and 4 bytes for the integer value
		}
		else // if not integer
		{
			std::cout << "ERROR: Integer value not found to unpack." << std::endl;
		}

		return val_ptr[0];
	}


	int rec_upackstr(void* buf, uint16_t &next, std::string &val)
	{
		/* Define pointers */
		uint16_t str_len;
		uint16_t* type;

		type = (uint16_t*)((BYTE*)buf + next); // get the type

		if(type[0] >= 9) // if string
		{
			str_len = type[0] - 9;
			BYTE* val_ptr = (BYTE*)((BYTE*)buf + next + sizeof(uint16_t));
			next = next + sizeof(uint16_t) + str_len;

			for(uint16_t i = 0; i < str_len; i++)
			{
				val += val_ptr[i];
			}
		}
		else // if not string
		{
			std::cout << "ERROR: String value not found to unpack." << std::endl;
		}

		return (int)str_len;
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


	void rec_begin(std::string &buf, uint16_t &next)
	{
		buf.resize(sizeof(uint16_t)); // start the record buffer by creating enough space for 'L'
		next = 2; // always initializes next to 2 for the unpack functions
	}
}