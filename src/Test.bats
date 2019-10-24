#!/usr/bin/env bats

load '../test/libs/bats-support/load'
load '../test/libs/bats-assert/load'

@test "dummy test1" {
	run ls 
	assert_equal $status 0 
}

@test "query the database about users' freespace and assert if they are equal to the freespace in the database" {
	run sqlite3 ~/Desktop/ntap.db "select freespace from users where name = 'user1'"
	[ "$output" = 1000 ]
	run sqlite3 ~/Desktop/ntap.db "select freespace from users where name = 'user2'"
	[ "$output" = 1000 ]
	run sqlite3 ~/Desktop/ntap.db "select freespace from users where name = 'user3'"
	[ "$output" = 100 ]
}
