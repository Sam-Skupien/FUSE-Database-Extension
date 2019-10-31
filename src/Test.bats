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
	sudo umount $mountpoint
	if [ -d "$basedir" ]; then
		rm -r $basedir
	fi
	if [ -d "$mountpoint" ]; then
		rm -r $mountpoint
	fi
}

@test "Dummy Test" {
	# Do nothing but to be the first test. Therefore, the real tests will not be blocked by the password typing
	echo "Dummy..." 
}

@test "Test0: test if the database is initialized as required" {

	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'sam-skupien'"
	[ "$output" = 5 ]
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 10 ]
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
}
	
