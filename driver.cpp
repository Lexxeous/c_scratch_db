#include "paging_manager.h"


int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("ERROR: Wrong number of command line arguments. Use ./<executable> <db_file> as format.");
		exit(EXIT_FAILURE);
	}

	char* test_db_name = argv[1]; // get the DB file name

	/********************************** FORMAT THE DB FILE IN BINARY **********************************/
	uint16_t ref_num = 2;
	uint16_t &num_pages = ref_num;
	Page_file::pgf_format(test_db_name, num_pages);

	/*********************************** TEST THE pgf_read FUNCTION ***********************************/
	// void* pg_buf;
	// pg_buf = calloc(PAGE_SIZE, sizeof(char));
	// std::fstream pfile; // define the pages file for reading
	// pfile.open(test_db_name); // open the pages file
	// Page_file::pgf_read(pfile, 1, pg_buf);
	// pfile.close(); // close the pages file

	/************************************* SET UP RECORD #1 TO ADD ************************************/

	/* Create the string for the whole of record 1 */
	std::string s_1;

	/* Create integer data for record 1 */
	int addr_1_num = 150;
	int zip_code_1 = 38501;

	/* Create reference string(s) for string data for record 1 */
	std::string const_ref_s_1 = "abcdefghijklmnopqrstuvwxyz777"; // 29 characters without null terminator '\0';
	std::string &addr_str_1 = const_ref_s_1;

	std::string const_ref_s_2 = "abcdefgh"; // 8 characters without null terminator '\0';
	std::string &addr_str_2 = const_ref_s_2;

	std::string const_ref_s_3 = "ab"; // 2 characters without null terminator '\0';
	std::string &addr_str_3 = const_ref_s_3;

	/* Create the reference value for the record's number, for which to be added */
	uint16_t ref_page_num_1 = 1;
	void* page_num_1 = &ref_page_num_1;

	/* Setup and add record 1 */
	Page::rec_begin(s_1);
	Page::rec_packint(s_1, addr_1_num);
	Page::rec_packstr(s_1, addr_str_1);
	Page::rec_packstr(s_1, addr_str_2);
	Page::rec_packstr(s_1, addr_str_3);
	Page::rec_packint(s_1, zip_code_1);
	Page::rec_finish(s_1);
	Page::pg_add_record(page_num_1, (void*)s_1.data());

	/************************************* SET UP RECORD #2 TO ADD ************************************/

	return 0;
}
