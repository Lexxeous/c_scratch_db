# Set defualt parameters
db_file ?= "test_db.dat"
buf_file ?= "page_buf.dat"
db_exec ?= "driver"
test_exec ?= "test"

run: comp
	./$(db_exec) $(db_file) # run "driver" executable

comp:
	g++ -o $(db_exec) driver.cpp paging_manager.cpp # compile and link into a "driver" executable

clean:
	rm $(db_file) $(buf_file) $(db_exec) $(test_exec)

del:
	rm -r a.out*

# debug: comp
# 	lldb $(db_exec) $(db_file)

test: test_db
	./$(test_exec)

test_db:
	g++ -o $(test_exec) test.cpp paging_manager.cpp
