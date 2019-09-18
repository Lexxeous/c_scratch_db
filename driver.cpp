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
	uint16_t ref_rum = 2;
	uint16_t &num_pages = ref_rum;
	Page_file::pgf_format(test_db_name, num_pages);

	/*********************************** TEST THE pgf_read FUNCTION ***********************************/
	void* pg_buf;
	pg_buf = calloc(PAGE_SIZE, sizeof(char));
	std::fstream pfile; // define the pages file for reading
	pfile.open(test_db_name); // open the pages file
	Page_file::pgf_read(pfile, 1, pg_buf);
	pfile.close(); // close the pages file

	/************************************* SET UP RECORD #1 TO ADD ************************************/

	std::string ref_s;
	std::string &s = ref_s;
	int val = 4123456;

	Page::rec_begin(s);
	Page::rec_packint(s, val);
	// Page::rec_packstr();
	// Page::rec_finish();

	// Page::pg_add_record();

	/************************************* SET UP RECORD #1 TO ADD ************************************/

	return 0;
}