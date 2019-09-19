/**************************************************************************************************
* Filename:   paging_manager.h
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB paging implementation.
**************************************************************************************************/

/********************************************* GUARD *********************************************/

#ifndef PAGING_MANAGER
#define PAGING_MANAGER

/***************************************** HEADER FILES ******************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

/****************************************** DATA TYPES *******************************************/

typedef uint8_t BYTE; // 8-bits is one Byte, one char is one Byte
typedef uint16_t rec_offset_t; // simple typedef for a directory entry
typedef std::fstream file_descriptor_t;

/****************************************** DEFINITIONS ******************************************/

#define F_NAME_LEN 200 // maximum file name length
#define PG_RECORD_UNUSED 65535

/******************************************* CONSTANTS *******************************************/

const uint16_t PAGE_SIZE = 16384; // page size is 16kB ; 1kB = 1024 Bytes
const uint16_t SIG_SIZE = 5;

/************************************** FUNCTION PROTOTYPES **************************************/

namespace Page_file
{
	// Formats a page file. Should create the page file with the file name fname, give it the appropriate page size of fsize, initialize the page header, and zero the pages.
	void pgf_format(char fname[200], uint16_t &fsize);

	// Write a page with page number page_id that is buffered at page_buf to the formatted open page file pfile.
	void pgf_write(file_descriptor_t &pfile, int page_id, void *page_buf);

	// Read a page with page number page_id from the open file pfile into the the memory at page_buf.
	void pgf_read(file_descriptor_t &pfile, int page_id, void *page_buf);

	// Print the contents of the DB file to ensure the existance of appropriate records.
	void print(char fname[F_NAME_LEN]);
}


namespace Page
{
	// Move the data beginning at start and ending at the beginning of the record directory backwards num_bytes bytes. The parameter page_buf is passed for error checking.
	void pg_compact(void *page_buf, uint16_t num_bytes, BYTE *start);

	// Move the bytes in the page beginning at start forwards num_bytes bytes.
	void pg_expand(void *page_buf, uint16_t num_bytes, BYTE *start);

	// Add a pre-formatted record at record into the page whose buffer address is at page.
	void pg_add_record(void *page, void *record);

	// Delete the record with the record id rec_id from the page buffered at page. Note that this function may have to compact the page.
	void pg_del_record(void *page, uint16_t rec_id); 

	// Replace the record with record id rec_id with the record at record in the page buffered at page. Note that if the record is shorter, then compaction is required, but is the record is longer, expansion is required.
	int pg_modify_record(void *page, void *record, BYTE rec_id); 

	// Pack the integer in val at the end of the buffer at buf. Note that the format is to pack the size of the integer in bytes (4) followed by the value.
	void rec_packint(std::string &buf, int val);

	// Pack the string in str to the end of the buffer at buf. Note that the format is to pack the size of the string plus 9 followed by the bytes of the string.
	void rec_packstr(std::string &buf, const std::string &str);

	// Unpack the next integer in buf and return its value. This function should advance next to the index of the next field in the buffer for subsequent calls (as does an iterator).
	int rec_upackint(void *buf, uint16_t &next);

	// Unpacks the next string in buf and put its value in val. It returns the size of the string. The next parameter advance to index to the next field.
	int rec_upackstr(void *buf, uint16_t &next, std::string val);

	// Stores the total length of buf into the first two bytes as a 16-bit unsigned integer.
	void rec_finish(std::string &buf);

	// Resize buf so that the size can be placed at the first two bytes. Initializes next to the index of where the first field should go.
	void rec_begin(std::string &buf);
}

#endif // PAGING_MANAGER
