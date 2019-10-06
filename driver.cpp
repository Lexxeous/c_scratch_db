/****************************************** HEADER FILES *****************************************/

#include "paging/paging.h"
#include "buffer_manager.h"

/**************************************** GLOBAL VARIABLES ***************************************/

// std::map<uint16_t, Buffer::page_descriptor_t> buf_pool;

/************************************** DRIVER IMPLEMENTATION ************************************/

int main(int argc, char* argv[])
{

	/**************************************** ERROR CHECKING ***************************************/

	if(argc != 2)
	{
		printf("ERROR: Wrong number of command line arguments. Use ./<executable> <db_file> as format.");
		exit(EXIT_FAILURE);
	}

	/******************************************** SETUP ********************************************/

	char* test_db_name = argv[1]; // get the DB file name
	uint16_t lru_size = 3; // set the size of the LRU cache
	Buffer::initialize(lru_size);
	Buffer::print_lru_list();

	/********************************* FORMAT THE DB FILE IN BINARY ********************************/

	uint16_t ref_num = 6;
	uint16_t &num_pages = ref_num;
	Page_file::pgf_format(test_db_name, num_pages);

	/********************************* PAGE BUFFER AND OPEN DB FILE ********************************/

	void* page_buf; // define the page buffer
	page_buf = calloc(PAGE_SIZE, sizeof(char)); // allocate 16384 bytes
	std::fstream pfile; // define the DB file for reading and writing
	pfile.open(test_db_name); // open the pages file

	/************************************ SET UP RECORD #0 TO ADD **********************************/

	/* Create the string for the whole of record 0 */
	std::string s_0;

	/* Create integer data for record 0 */
	int addr_num_0 = 150;
	int zip_code_0 = 38501;
	uint16_t rec_0_page_num = 1;

	/* Create reference string(s) for string data for record 0 */
	std::string const_ref_s_0 = "abcdefghijklmnopqrstuvwxyz777"; // 29 characters long
	std::string &addr_str_0 = const_ref_s_0;

	/* Setup and add record 0 */
	// Page_file::pgf_read(pfile, rec_0_page_num, page_buf);
	page_buf = Buffer::buf_read(pfile, rec_0_page_num);
	Page::rec_begin(s_0);
	Page::rec_packint(s_0, addr_num_0);
	Page::rec_packstr(s_0, addr_str_0);
	Page::rec_packint(s_0, zip_code_0);
	Page::rec_finish(s_0);
	uint16_t rec_id_0 = Page::pg_add_record(page_buf, (void*)s_0.data(), s_0.size()); // reclen = |2|2|4|2|29|2|4| ; L = 45
	std::cout << "Added record " << rec_id_0 << " to page " << rec_0_page_num << std::endl;
	// Page_file::pgf_write(pfile, rec_0_page_num, page_buf);

	/* Buffering for record 0 on page 1 */
	Buffer::buf_write(page_buf, rec_0_page_num); // write the page to LRU cache and set its dirty bit
	Buffer::print_lru_list();
	Page_file::print(pfile);
	// Buffer::flush(pfile, rec_0_page_num);

	/*********************************** SET UP RECORD #1 TO ADD *********************************/

	/* Create the string for the whole of record 1 */
	std::string s_1;

	/* Create integer data for record 1 */
	int addr_num_1 = 42;
	int zip_code_1 = 20470;
	int rec_1_page_num = 1;

	/* Create reference string(s) for string data for record 1 */
	std::string const_ref_s_1 = "P Sherman Wallaby Way, Sydney"; // 29 characters long
	std::string &addr_str_1 = const_ref_s_1;

	/* Setup and add record 1 */
	// Page_file::pgf_read(pfile, rec_1_page_num, page_buf);
	page_buf = Buffer::buf_read(pfile, rec_1_page_num);
	Page::rec_begin(s_1);
	Page::rec_packint(s_1, addr_num_1);
	Page::rec_packstr(s_1, addr_str_1);
	Page::rec_packint(s_1, zip_code_1);
	Page::rec_finish(s_1);
	uint16_t rec_id_1 = Page::pg_add_record(page_buf, (void*)s_1.data(), s_1.size()); // reclen = |2|2|4|2|29|2|4| ; L = 45
	std::cout << "Added record " << rec_id_1 << " to page " << rec_1_page_num << std::endl;
	// Page_file::pgf_write(pfile, rec_1_page_num, page_buf);

	/* Buffering for record 1 on page 1 */
	Buffer::buf_write(page_buf, rec_1_page_num); // write the page to LRU cache and set its dirty bit
	Buffer::print_lru_list();
	Page_file::print(pfile);
	Buffer::flush(pfile, rec_1_page_num);
	
	/*********************************** SET UP RECORD #2 TO ADD *********************************/

	// /* Create the string for the whole of record 2 */
	// std::string s_2;

	// /* Create integer data for record 2 */
	// int addr_num_2 = 666;
	// int zip_code_2 = 66666;
	// int rec_2_page_num = 2;

	// /* Create reference string(s) for string data for record 2 */
	// std::string const_ref_s_2 = "Highway to Hell"; // 15 characters long
	// std::string &addr_str_2 = const_ref_s_2;

	// /* Setup and add record 2 */
	// Page_file::pgf_read(pfile, rec_2_page_num, page_buf);
	// Page::rec_begin(s_2);
	// Page::rec_packint(s_2, addr_num_2);
	// Page::rec_packstr(s_2, addr_str_2);
	// Page::rec_packint(s_2, zip_code_2);
	// Page::rec_finish(s_2);
	// uint16_t rec_id_2 = Page::pg_add_record(page_buf, (void*)s_2.data(), 31); // reclen = |2|2|4|2|15|2|4| ; L = 31
	// Page_file::pgf_write(pfile, rec_2_page_num, page_buf);
	// std::cout << "Added record " << rec_id_2 << " to page " << rec_2_page_num << std::endl;

	/*********************************** SET UP RECORD #3 TO ADD *********************************/

	// /* Create the string for the whole of record 3 */
	// std::string s_3;

	// /* Create integer data for record 3 */
	// int addr_num_3 = 752;
	// int zip_code_3 = 38502;
	// int rec_3_page_num = 2;

	// /* Create reference string(s) for string data for record 3 */
	// std::string const_ref_s_3 = "North Dixie Ave"; // 15 characters long
	// std::string &addr_str_3 = const_ref_s_3;

	// /* Setup and add record 3 */
	// Page_file::pgf_read(pfile, rec_3_page_num, page_buf);
	// Page::rec_begin(s_3);
	// Page::rec_packint(s_3, addr_num_3);
	// Page::rec_packstr(s_3, addr_str_3);
	// Page::rec_packint(s_3, zip_code_3);
	// Page::rec_finish(s_3);
	// uint16_t rec_id_3 = Page::pg_add_record(page_buf, (void*)s_3.data(), 31); // reclen = |2|2|4|2|15|2|4| ; L = 31
	// Page_file::pgf_write(pfile, rec_3_page_num, page_buf);
	// std::cout << "Added record " << rec_id_3 << " to page " << rec_3_page_num << std::endl;

	/*********************************** SET UP RECORD #4 TO ADD *********************************/

	// /* Create the string for the whole of record 4 */
	// std::string s_4;

	// /* Create integer data for record 4 */
	// int addr_num_4 = 123;
	// int zip_code_4 = 12345;
	// int rec_4_page_num = 3;

	// /* Create reference string(s) for string data for record 4 */
	// std::string const_ref_s_4 = "This is an address line"; // 23 characters long
	// std::string &addr_str_4 = const_ref_s_4;

	// /* Setup and add record 4 */
	// Page_file::pgf_read(pfile, rec_4_page_num, page_buf);
	// Page::rec_begin(s_4);
	// Page::rec_packint(s_4, addr_num_4);
	// Page::rec_packstr(s_4, addr_str_4);
	// Page::rec_packint(s_4, zip_code_4);
	// Page::rec_finish(s_4);
	// uint16_t rec_id_4 = Page::pg_add_record(page_buf, (void*)s_4.data(), 39); // reclen = |2|2|4|2|23|2|4| ; L = 39
	// Page_file::pgf_write(pfile, rec_4_page_num, page_buf);
	// std::cout << "Added record " << rec_id_4 << " to page " << rec_4_page_num << std::endl;

	/************************************** DELETE SOME RECORDS ************************************/

	// int del_page_num_2 = 2;
	// int del_page_num_3 = 3;

	// Page_file::pgf_read(pfile, del_page_num_2, page_buf); // read page 2
	// Page::pg_del_record(page_buf, 1); // delete record at index 1
	// Page_file::pgf_write(pfile, del_page_num_2, page_buf); // write page 2

	// Page_file::pgf_read(pfile, del_page_num_3, page_buf); // read page 3
	// Page::pg_del_record(page_buf, 0); // delete record at index 0
	// Page_file::pgf_write(pfile, del_page_num_3, page_buf); // write page 3

	/*************************************** PRINT THE RECORDS *************************************/

	Page_file::print(pfile);

	pfile.close();
	return 0;
}
