# Names

Arya More, Michael Chen

# Compilation

First do
```cc -D_REENTRANT -pthread countnames.c helper_functions.c hash_table.c -o countnames -Wall -Werror``` for countnames

Then do `cc shell1.c helper_functions.c hash_table.c -o shell1 -Wall -Werror` for shell1.

# Running the program
Copy and paste this to the terminal to test every text file in test.
`./countnames test/names.txt test/names1.txt test/names2.txt test/names_long.txt test/names_long_redundant.txt test/names_long_redundant1.txt test/names_long_redundant2.txt test/names_long_redundant3.txt test/namesB.txt test/custom_testcase_1.txt test/custom_testcase_2.txt test/custom_testcase_3.txt`

Check for memory leaks using this command(may first need to run chmod a+x test/*.txt):

`valgrind --leak-check=full --log-file=valgrind.out --track-children=yes --show-leak-kinds=all ./countnames test/names.txt test/names1.txt test/names2.txt test/names_long.txt test/names_long_redundant.txt test/names_long_redundant1.txt test/names_long_redundant2.txt test/names_long_redundant3.txt test/namesB.txt test/custom_testcase_1.txt test/custom_testcase_2.txt test/custom_testcase_3.txt`

Output is in output folder, first do

`ls output`

to get the filename, then do

`cat <PID>.out`

to get the output.
# Lessons Learned:

Learned how to make a data structure thread-safe by utilizing and implementing locks.

Learned how to create threads in UNIX which execute a function routine given arguments(in struct) and a thread array.

# Acknowledgments
https://stackoverflow.com/questions/5999418/why-is-strtok-considered-unsafe
https://stackoverflow.com/questions/11042218/c-restore-stdout-to-terminal