# Names

Arya More, Michael Chen

# Compilation

First do
```cc -D_REENTRANT -pthread countnames.c helper_functions.c hash_table.c -o countnames -Wall -Werror``` for countnames

Then do `cc shell1.c helper_functions.c hash_table.c -o shell1 -Wall -Werror` for shell1.

# Running the program

First run ./shell1.
If you want to test countnames, copy and paste this to the terminal to test every text file in test.
`./countnames test/names.txt test/names1.txt test/names2.txt test/names_long.txt test/names_long_redundant.txt test/names_long_redundant1.txt test/names_long_redundant2.txt test/names_long_redundant3.txt test/namesB.txt test/custom_testcase_1.txt test/custom_testcase_2.txt test/custom_testcase_3.txt`

Check for memory leaks using this command in shell1(may first need to run chmod a+x test/*.txt):

`valgrind --leak-check=full --log-file=valgrind.out --track-children=yes --show-leak-kinds=all ./countnames test/names.txt test/names1.txt test/names2.txt test/names_long.txt test/names_long_redundant.txt test/names_long_redundant1.txt test/names_long_redundant2.txt test/names_long_redundant3.txt test/namesB.txt test/custom_testcase_1.txt test/custom_testcase_2.txt test/custom_testcase_3.txt`
# Lessons Learned:
How to implement a hash table in C

What stack smashing is(one of the error messages mentioned it)

How to properly use valgrind

# Acknowledgments
https://stackoverflow.com/questions/1345670/stack-smashing-detected
https://stackoverflow.com/questions/25906679/valgrind-permission-denied-on-file