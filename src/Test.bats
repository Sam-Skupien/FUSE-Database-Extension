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
	ntapfuse mount $basedir $mountpoint -o allow_other -s 3>&-
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

function create_three_users {

	run id -u user1
	# check if a user exists
	if [ "$status" -eq 1 ]
	then
		sudo useradd user1 
	fi
	run id -u user2
	if [ "$status" -eq 1 ]
	then
		sudo useradd user2 
	fi
	run id -u user3
	if [ "$status" -eq 1 ]
	then
		sudo useradd user3 
	fi
}

function del_three_users {

	run id -u user1
	if [ "$status" -eq 0 ]
	then
		echo "user1 Deleted"
		sudo userdel user1
	fi
	run id -u user2
	if [ "$status" -eq 0 ]
	then
		echo "user2 Deleted"
		sudo userdel user2
	fi
	run id -u user3
	if [ "$status" -eq 0 ]
	then
		echo "user1 Deleted"
		sudo userdel user3
	fi

}

@test "Test0: test if the database is initialized as required" {
	# Explanation of tests
	echo "Steps for Test 0:" >&3

	echo "	1.SELECT FREESPACE from USERS where NAME = 'sam-skupien'"	>&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'sam-skupien'"

	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'sam-skupien'") == Expected output: 20">&3
	[ "$output" = 20 ]

	echo "	2.SELECT FREESPACE from USERS where NAME = 'dongbang'"	>&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'") == Expected output: 10">&3
	[ "$output" = 10 ]
}

# TODO: This test can be enhanced by dynamically query from the database and write the number of bytes based on the query.
@test "Test1: test ifd write function allows users to write bytes with enough FREESPACE" {
	# IMPORTANT: run echo "abcdefghij" >> $mountpoint/abc will not redirect the data to the file
	echo "Steps for Test 1:">&3
	echo "	1.echo 'abcdefghi' >> $mountpoint/abc" >&3
	sudo -u dongbang echo "abcdefghi" >> $mountpoint/abc 

	echo "	2.cat $mountpoint/abc" >&3 
	run cat $mountpoint/abc

	echo "	Actual output: $(cat $mountpoint/abc) == Expected output: abcdefghi" >&3
	[ "$output" = "abcdefghi" ]
	[ "$status" -eq 0 ]

	echo "	3.SELECT FREESPACE from USERS where NAME = 'dongbang'" >&3
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'") == Expected output: 0" >&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 0 ]
}

@test "Test2: test if write function prevents users from writing bytes more than their quota." {
	
	# use if to deal with errors in bash command
	echo "Steps for Test 2:">&3
	echo "	1.echo 'abcdefghij' >> $mountpoint/abc" >&3
	echo "	2.cat $mountpoint/abc" >&3
	echo "	Actual status: 1 == Expected status: 1, because file abc does not exist">&3
	if [! echo "abcdefghij" >> $mountpoint/abc ] 
	then
		# the cat command return 1 because the file 'abc' does not exist
		run cat $mountpoint/abc
		[ "$status" -eq 1 ] 
	fi

	echo "	3.SELECT FREESPACE from USERS where NAME = 'dongbang'" >&3
	echo "	Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'") == Expected output: 10, because user's write operation is rejected" >&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 10 ]
}

@test "Test3: test three users using write function to write bytes into files" {
	
	echo "Steps for Test 3:" >&3
	echo "	1.Create three users: user1, user2, and user3" >&3
	# create three users
	create_three_users

	# As user1, write "abcd" into file1
	echo "	2.As user1,write 'abcd' into file1" >&3
	sudo -u user1 echo "abcd" >> $mountpoint/file1
	run cat $mountpoint/file1 
	[ "$status" -eq 0 ]
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'user1'"
	[ "$output" = 0 ]

	# As user2, write "abcdefghi" into file2
	echo "	3.As user2,write 'abcdefghi' into file2" >&3
	sudo -u user2 echo "abcdefghi" >> $mountpoint/file2
	run cat $mountpoint/file2 
	[ "$status" -eq 0 ]
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'user2'"
	[ "$output" = 0 ]

	# As user3, write "abcdefghijklmn" into file3
	echo "	4.As user3,write 'abcdefghijklmn' into file3" >&3
	sudo -u user3 echo "abcdefghijklmn" >> $mountpoint/file3
	run cat $mountpoint/file3 
	[ "$status" -eq 0 ]
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'user3'"
	[ "$output" = 0 ]

	echo "	5.Test if the FREESAPCE for user1, user2, user3 equal to 0." >&3

	del_three_users
}

@test "Test: test if unlink function will return FREESPACE back to user" {
	echo "Steps for Test:" >&3

	echo "	1.User ‘dongbang’ writes 'abcdefghi' to file1" >&3
	sudo -u dongbang echo "abcdefghi" >> $mountpoint/file1 

	echo "	2.Test if the FREESPACE of ‘dongbang’ is equal to 0" >&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"
	[ "$output" = 0 ]

	echo "	3.Delete the file1" >&3
	sudo -u dongbang rm $mountpoint/file1 

	echo "	4.Query the FREESPACE of user ‘dongbang’ from the database." >&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"

	echo "	5.Actual output: $(sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'") == Expected output: 10" >&3	
	[ "$output" = 10 ]
}

@test "Test: Test truncate to extend the size of a file" {
	
	echo "Steps for Test:" >&3

	echo "	1.User ‘dongbang’ writes 'abc' to file1" >&3
	sudo -u dongbang echo "abc" >> $mountpoint/file1

	echo "	2.truncate the file1 size to 10">&3
	truncate -s 10 $mountpoint/file1

	echo "	3.Test if the FREESPACE for ‘dongbang’ is 0.">&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"	
	[ "$output" = 0 ]

	echo "	4.Test if the file size for file1 is 10 ">&3
	filesize=$(wc -c $mountpoint/file1 | awk '{print $1}')
	[ "$filesize" -eq 10 ]
}

@test "Test: Test truncate to shrink the size of a file" {
	
	echo "Steps for Test:" >&3

	echo "	1.User ‘dongbang’ writes 'abcdefghi' to file1" >&3
	sudo -u dongbang echo "abc" >> $mountpoint/file1

	echo "	2.truncate the file1 size to 5">&3
	truncate -s 5 $mountpoint/file1

	echo "	3.Test if the FREESPACE for ‘dongbang’ is 5.">&3
	run sqlite3 $mountpoint/ntap.db "SELECT FREESPACE from USERS where NAME = 'dongbang'"	
	[ "$output" = 5 ]

	echo "	4.Test if the file size for file1 is 5 ">&3
	filesize=$(wc -c $mountpoint/file1 | awk '{print $1}')
	[ "$filesize" -eq 5 ]
}


