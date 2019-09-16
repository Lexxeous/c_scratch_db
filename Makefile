# Set defualt parameters
db_file ?= "test_db.txt"

run: db
	./db $(db_file) # run db executable

db:
	g++ -o db db.cpp c_scratch_db.cpp # compile and link into a "db" executable

clean:
	rm db test test_db.txt

del:
	rm -r a.out*

# debug:
# 	g++ -g db.cpp c_scratch_db.cpp
# 	sudo gdb ./a.out

test: test_db
	./test

test_db:
	g++ -o test test.cpp c_scratch_db.cpp
