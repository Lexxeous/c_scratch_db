# Set defualt parameters
db_file ?= "test_db.dat"
db_exec ?= "db"
test_exec ?= "test"

run: db
	./$(db_exec) $(db_file) # run db executable

db:
	g++ -o $(db_exec) db.cpp c_scratch_db.cpp # compile and link into a "db" executable

clean:
	rm $(db_exec) $(test_exec) $(db_file)

del:
	rm -r a.out*

# debug: db
# 	lldb $(db_exec) $(db_file)

test: test_db
	./$(test_exec)

test_db:
	g++ -o $(test_exec) test.cpp c_scratch_db.cpp
