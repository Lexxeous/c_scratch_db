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

	/**************************************** READ TABLE DESCR *************************************/

	const std::string table_name = "#columns";
	Table::table_descriptor_t table_desc;
	Table::read_table_descriptor(pfile, table_name, table_desc);

	/*************************************** CREATE PERSON TABLE ***********************************/

	const std::string table_name_0 = "person";
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

	Table::create_table(pfile, table_name_0, columns);

	/*************************************** ADD TO PERSON TABLE ***********************************/

	std::vector<Table::colval_t> vals_A;

	Table::colval_t val_1;
	val_1.first = "first_name";
	val_1.second = "Joe";
	vals_A.push_back(val_1);

	Table::colval_t val_2;
	val_2.first = "last_name";
	val_2.second = "Smith";
	vals_A.push_back(val_2);

	Table::colval_t val_3;
	val_3.first = "age";
	val_3.second = "37";
	vals_A.push_back(val_3);

	Table::insert_into(pfile, table_name_0, vals_A);

	/*************************************** ADD TO PERSON TABLE ***********************************/

	std::vector<Table::colval_t> vals_B;

	Table::colval_t val_4;
	val_4.first = "first_name";
	val_4.second = "Alex";
	vals_B.push_back(val_4);

	Table::colval_t val_5;
	val_5.first = "last_name";
	val_5.second = "Gibson";
	vals_B.push_back(val_5);

	Table::colval_t val_6;
	val_6.first = "age";
	val_6.second = "23";
	vals_B.push_back(val_6);

	Table::insert_into(pfile, table_name_0, vals_B);

	/*************************************** ADD TO PERSON TABLE ***********************************/

	std::vector<Table::colval_t> vals_C;

	Table::colval_t val_7;
	val_7.first = "first_name";
	val_7.second = "Another";
	vals_C.push_back(val_7);

	Table::colval_t val_8;
	val_8.first = "last_name";
	val_8.second = "One";
	vals_C.push_back(val_8);

	Table::colval_t val_9;
	val_9.first = "age";
	val_9.second = "999";
	vals_C.push_back(val_9);

	Table::insert_into(pfile, table_name_0, vals_C);

	/******************************************** FINALIZE *****************************************/

	Buffer_mgr::shutdown(pfile);
	Page_file::print(pfile);
	pfile.close();
	return 0;
}
