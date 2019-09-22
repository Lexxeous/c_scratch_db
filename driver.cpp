/****************************************** HEADER FILES *****************************************/

#include "paging_manager.h"

/************************************** DRIVER IMPLEMENTATION ************************************/

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("ERROR: Wrong number of command line arguments. Use ./<executable> <db_file> as format.");
		exit(EXIT_FAILURE);
	}

	char* test_db_name = argv[1]; // get the DB file name

	/******************************** FORMAT THE DB FILE IN BINARY *******************************/

	uint16_t ref_num = 3;
	uint16_t &num_pages = ref_num;
	Page_file::pgf_format(test_db_name, num_pages);

	/******************************** PAGE BUFFER AND OPEN DB FILE *******************************/

	void* page_buf; // define the page buffer
	page_buf = calloc(PAGE_SIZE, sizeof(char)); // allocate 16384 bytes
	std::fstream pfile; // define the DB file for reading and writing
	pfile.open(test_db_name); // open the pages file

	/*********************************** SET UP RECORD #2 TO ADD *********************************/

	/* Create the string for the whole of record 1 */
	std::string s_0;

	/* Create integer data for record 1 */
	int addr_num_0 = 150;
	int zip_code_0 = 38501;
	int page_num_0 = 1;

	/* Create reference string(s) for string data for record 1 */
	std::string const_ref_s_0 = "abcdefghijklmnopqrstuvwxyz777"; // 29 characters without null terminator '\0';
	std::string &addr_str_0 = const_ref_s_0;

	/* Setup and add record 1 */
	Page_file::pgf_read(pfile, page_num_0, page_buf);
	Page::rec_begin(s_0);
	Page::rec_packint(s_0, addr_num_0);
	Page::rec_packstr(s_0, addr_str_0);
	Page::rec_packint(s_0, zip_code_0);
	Page::rec_finish(s_0);
	uint16_t rec_id_0 = Page::pg_add_record(page_buf, (void*)s_0.data());
	Page_file::pgf_write(pfile, page_num_0, page_buf);
	std::cout << "Added record " << rec_id_0 << " to page " << page_num_0 << std::endl;

	/*********************************** SET UP RECORD #1 TO ADD *********************************/

	/* Create the string for the whole of record 1 */
	std::string s_1;

	/* Create integer data for record 1 */
	int addr_num_1 = 140;
	int zip_code_1 = 37066;
	int page_num_1 = 1;

	/* Create reference string(s) for string data for record 1 */
	std::string const_ref_s_1 = "Upper Station Camp Creek Road"; // 29 characters without null terminator '\0';
	std::string &addr_str_1 = const_ref_s_1;

	/* Setup and add record 1 */
	Page_file::pgf_read(pfile, page_num_1, page_buf);
	Page::rec_begin(s_1);
	Page::rec_packint(s_1, addr_num_1);
	Page::rec_packstr(s_1, addr_str_1);
	Page::rec_packint(s_1, zip_code_1);
	Page::rec_finish(s_1);
	uint16_t rec_id_1 = Page::pg_add_record(page_buf, (void*)s_1.data());
	Page_file::pgf_write(pfile, page_num_1, page_buf);
	std::cout << "Added record " << rec_id_1 << " to page " << page_num_1 << std::endl;

	/*********************************** SET UP RECORD #2 TO ADD *********************************/

	/* Create the string for the whole of record 2 */
	std::string s_2;

	/* Create integer data for record 2 */
	int addr_num_2 = 999;
	int zip_code_2 = 69696;
	int page_num_2 = 2;

	/* Create reference string(s) for string data for record 2 */
	std::string const_ref_s_2 = "123456789ABCDEF123"; // 18 characters without null terminator '\0';
	std::string &addr_str_2 = const_ref_s_2;

	/* Setup and add record 2 */
	Page_file::pgf_read(pfile, page_num_2, page_buf);
	Page::rec_begin(s_2);
	Page::rec_packint(s_2, addr_num_2);
	Page::rec_packstr(s_2, addr_str_2);
	Page::rec_packint(s_2, zip_code_2);
	Page::rec_finish(s_2);
	uint16_t rec_id_2 = Page::pg_add_record(page_buf, (void*)s_2.data());
	Page_file::pgf_write(pfile, page_num_2, page_buf);
	std::cout << "Added record " << rec_id_2 << " to page " << page_num_2 << std::endl;

	/*********************************** SET UP RECORD #3 TO ADD *********************************/

	/* Create the string for the whole of record 3 */
	std::string s_3;

	/* Create integer data for record 3 */
	int addr_num_3 = 666;
	int zip_code_3 = 42020;
	int page_num_3 = 2;

	/* Create reference string(s) for string data for record 3 */
	std::string const_ref_s_3 = "amzldofkaheyfhgjahdbeee"; // 23 characters without null terminator '\0';
	std::string &addr_str_3 = const_ref_s_3;

	/* Setup and add record 3 */
	Page_file::pgf_read(pfile, page_num_3, page_buf);
	Page::rec_begin(s_3);
	Page::rec_packint(s_3, addr_num_3);
	Page::rec_packstr(s_3, addr_str_3);
	Page::rec_packint(s_3, zip_code_3);
	Page::rec_finish(s_3);
	uint16_t rec_id_3 = Page::pg_add_record(page_buf, (void*)s_3.data());
	Page_file::pgf_write(pfile, page_num_3, page_buf);
	std::cout << "Added record " << rec_id_3 << " to page " << page_num_3 << std::endl;


	pfile.close();
	return 0;
}
