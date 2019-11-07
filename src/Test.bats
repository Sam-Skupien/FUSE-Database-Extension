#!/usr/bin/env bats

# TODO: Enhance the robustness of tests 
# The tests failed sometimes because of the mechanism of the framework itself.
# This framework does not run these tests in a same process. Instead, each test is run
# in a different process. In each process, each test all mount the file system first and 
# unmount the file system after they finish. We all know that operating system switch bwtween 
# process at any time and this cause the race condition. Right now, I assume that this is ok
# because it does not hurt the tests. However, when there are more testcases, this would
# be a problem.

function setup() {
	# mount the file system on Desktop. Only need to change 'ntapfuse_basepath' if moved.
	# All the variables even if the variables inside the functions are defined as golbal, by default
	ntapfuse_basepath=~/Desktop
	basedir=$ntapfuse_basepath/basedir
	mountpoint=$ntapfuse_basepath/mountpoint
	# while the file system is mounted, sleep
	# run mountpoint -q $mountpoint
	# while [ "$status" -eq 0 ]  
	# do
	# 	sleep 1
	# 	echo "It is mounted, wait"	>&3
	# 	run mountpoint -q $mountpoint
	# done

	if [ ! -d "$basedir" ]; then
		mkdir $basedir
	fi
	if [ ! -d "$mountpoint" ]; then
		mkdir $mountpoint
	fi
	# ntapfuse spawned a child process which inherit file descriptor 3. 
	# If this child process is a background process that will run indefinitely,
	# Bats will be similarly blocked for the same amount of time
	# More details: https://github.com/bats-core/bats-core
	ntapfuse mount $basedir $mountpoint 3>&-
}

function teardown() {
	# unmount the file system
	run pkill ntapfuse
	if [ -d "$basedir" ]; then
		rm -r $basedir
	fi
	if [ -d "$mountpoint" ]; then
		rm -r $mountpoint
	fi
}


@test "Test0: test if the database is initialized as required" {
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'sam-skupien'"
	[ "$output" = 5 ]
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 10 ]
	# Explanation of tests
	echo "Explanation for Test 0:">&3
	echo "	1.SELECT FREESPACE from USERS where NAME = 'sam-skupien'"	>&3
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'sam-skupien'") == Expected output: 5">&3
	echo "	1.SELECT FREESPACE from USERS where NAME = 'dongbang'"	>&3
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'sam-skupien'") == Expected output: 10">&3

}

# TODO: This test can be enhanced by dynamically query from the database and write the number of bytes based on the query.
@test "Test1: test if write function allows users to write bytes with enough FREESPACE" {
	# IMPORTANT: run echo "abcdefghij" >> $mountpoint/abc will not redirect the data to the file
	echo "abcdefghij" >> $mountpoint/abc 
	run cat $mountpoint/abc
	[ "$output" = "abcdefghij" ]
	[ "$status" -eq 0 ]
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 0 ]
	# Explanation of tests
	echo "Explanation for Test 1:">&3
	echo "	1.echo 'abcdefghij' >> $mountpoint/abc" >&3
	echo "	2.cat $mountpoint/abc" >&3 
	echo "	Actual output: $(cat $mountpoint/abc) == Expected output: abcdefghij" >&3
	echo "	3.SELECT FREESPACE from USERS where NAME = 'dongbang'" >&3
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'") == Expected output: 0" >&3
}

@test "Test2: test if write function prevents users from writing bytes more than their quota." {
	# use if to deal with errors in bash command
	if [! echo "abcdefghijk" >> $mountpoint/abc ] 
	then
		# the cat command return 1 because the file 'abc' does not exist	 
		run cat  $mountpoint/abc
		[ "$status" -eq 1 ] 
	fi
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 10 ]
	# Explanation
	echo "Explanation for Test 2:">&3
	echo "	1.echo 'abcdefghijk' >> $mountpoint/abc" >&3
	echo "	2.cat $mountpoint/abc" >&3
	echo "	Actual status: 1 == Expected status: 1, because file abc does not exist">&3
	echo "	3.SELECT FREESPACE from USERS where NAME = 'dongbang'" >&3
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'") == Expected output: 10, because user's write operation is rejected" >&3
}
	
# TODO: Test two users writting into the filesystem
