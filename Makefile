#----------------------------------------- DEFAULT PARAMS ----------------------------------------#
db_file ?= "test_db.dat"
buf_file ?= "page_buf.dat"
db_exec ?= "driver"
test_exec ?= "test"

#----------------------------------------- FOR EXECUTION -----------------------------------------#

run: comp
	./$(db_exec) $(db_file) # run "driver" executable

comp:
# 	g++ -o $(db_exec) driver.cpp paging_manager.cpp buffer_manager.cpp # compile and link into a "driver" executable
	g++ -o $(db_exec) driver.cpp "paging/paging.o" buffer_manager.cpp

clean:
	rm $(db_file) $(buf_file) $(db_exec) $(test_exec)

#----------------------------------------- FOR DEBUGGING -----------------------------------------#

debug: debug_comp
	lldb $(db_exec)

debug_comp:
# 	g++ -g -o $(db_exec) driver.cpp paging_manager.cpp buffer_manager.cpp # compile and link into a "driver" executable
	g++ -g -o $(db_exec) driver.cpp "paging/paging.o" buffer_manager.cpp

debug_clean:
	rm -r $(db_exec).dSYM

#------------------------------------------ FOR TESTING ------------------------------------------#

test: test_db
	./$(test_exec)

test_db:
# 	g++ -o $(test_exec) test.cpp paging_manager.cpp buffer_manager.cpp
	g++ -o $(test_exec) test.cpp "paging/paging.o" buffer_manager.cpp

#-------------------------------------------------------------------------------------------------#