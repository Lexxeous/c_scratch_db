/**************************************************************************************************
* Filename:   paging_manager.cpp
* Author:     Jonathan Alexander Gibson (jagibson44@students.tntech.edu)
* Copyright:	Tennessee Technological University (TTU)
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Defines the API for the corresponding DB paging implementation.
**************************************************************************************************/

/****************************************** HEADER FILES *****************************************/

#include "paging/paging.h"
#include "buffer_manager.h"

/************************************** IMPLEMENT STRUCTURES *************************************/



/********************************************* MACROS ********************************************/



/************************************ FUNCTION IMPLEMENTATIONS ***********************************/


void initialize(uint16_t pool_sz)
{

}


void shutdown(file_descriptor_t &pfile)
{

}


void LRU_update(uint16_t page_id)
{

}


void flush(file_descriptor_t &pfile, uint16_t page_id)
{

}


void flush_all(file_descriptor_t &pfile)
{

}


void buf_write(file_descriptor_t &pfile, int page_id)
{

}


// void* buf_read(file_descriptor_t &pfile, int page_id)
// {

// }


uint16_t replace(void* &page)
{
	return 0;
}


bool full()
{
	return false;
}