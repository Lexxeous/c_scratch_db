#include "c_scratch_db.h"


int main(int argc, char* argv[])
{
	if(argc != 2)
		{
			printf("ERROR: Wrong number of command line arguments. Use ./<executable> <db_file> as format.");
			exit(EXIT_FAILURE);
		}

	char* test_db_name = argv[1];

	uint16_t num = 2;
	uint16_t &num_pages = num;

	Page_file::pgf_format(test_db_name, num_pages);

	void* pg_buf;
	pg_buf = calloc(PAGE_SIZE, sizeof(char));

	std::fstream pfile; // define the pages file for reading
	pfile.open(test_db_name); // open the pages file

	Page_file::pgf_read(pfile, 1, pg_buf);

	pfile.close(); // close the pages file

	return 0;
}