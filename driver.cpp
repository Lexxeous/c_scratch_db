/****************************************** HEADER FILES *****************************************/

#include "paging/paging.h"
#include "buffer_mgr/buffer_mgr.h"
#include "table_manager.h"

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
	Buffer_mgr::initialize(lru_size);
	// Buffer_mgr::print_lru_list();

	/********************************* FORMAT THE DB FILE IN BINARY ********************************/

	uint16_t ref_num = 8; // the number of pages for the DB file
	uint16_t &num_pages = ref_num;
	Table::tbl_format(test_db_name, num_pages);

	/********************************* PAGE BUFFER AND OPEN DB FILE ********************************/

	void* page_buf; // define a page buffer
	page_buf = calloc(PAGE_SIZE, sizeof(char)); // allocate 16384 bytes
	std::fstream pfile; // define the DB file for reading and writing
	pfile.open(test_db_name); // open the pages file

	/************************************ SET UP RECORD #0 TO ADD **********************************/

	// /* Create the string for the whole of record 0 */
	// std::string s_0;

	// /* Create integer data for record 0 */
	// int addr_num_0 = 150;
	// int zip_code_0 = 38501;
	// int rec_0_page_num = 1;

	// /* Create reference string(s) for string data for record 0 */
	// std::string const_ref_s_0 = "abcdefghijklmnopqrstuvwxyz777"; // 29 characters long
	// std::string &addr_str_0 = const_ref_s_0;

	// /* Buffer and add record 0 on page 1*/
	// page_buf = Buffer_mgr::buf_read(pfile, rec_0_page_num); // read empty page from disk (file) or cached page from <buf_pool>
	// Page::rec_begin(s_0);
	// Page::rec_packint(s_0, addr_num_0);
	// Page::rec_packstr(s_0, addr_str_0);
	// Page::rec_packint(s_0, zip_code_0);
	// Page::rec_finish(s_0);
	// uint16_t rec_id_0 = Page::pg_add_record(page_buf, (void*)s_0.data(), s_0.size()); // reclen = |2|2|4|2|29|2|4| ; L = 45
	// std::cout << "Added record " << rec_id_0 << " to page " << rec_0_page_num << std::endl;

	// Buffer_mgr::buf_write(pfile, rec_0_page_num); // set the dirty bit for page 1
	// // Buffer_mgr::print_lru_list();
	// Page_file::print(pfile);

	/**************************************** READ TABLE DESCR *************************************/

	const std::string table_name = "#columns";
	Table::table_descriptor_t table_desc;
	Table::read_table_descriptor(pfile, table_name, table_desc);

	/***************************************** CREATE TABLE 0 **************************************/

	const std::string create_table_name = "person";
	std::vector<Table::col_def_t> columns;

	Table::col_def_t col_1;
	col_1.name = "first_name";
	col_1.type = Table::TBL_TYPE_VCHAR;
	col_1.size = 40;
	columns.push_back(col_1);

	Table::col_def_t col_2;
	col_2.name = "last_name";
	col_2.type = Table::TBL_TYPE_VCHAR;
	col_2.size = 40;
	columns.push_back(col_2);

	Table::col_def_t col_3;
	col_3.name = "age";
	col_3.type = Table::TBL_TYPE_SHORT;
	col_3.size = 1;
	columns.push_back(col_3);

	Table::create_table(pfile, create_table_name, columns);

	/******************************************** FINALIZE *****************************************/

	Buffer_mgr::shutdown(pfile);
	Page_file::print(pfile);
	pfile.close();
	return 0;
}
