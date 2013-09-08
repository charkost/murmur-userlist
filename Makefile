all:
	gcc -std=gnu99 -g -pedantic -Wall -Wextra -Werror murmur_userlist.c -o murmur_userlist
clean:
	rm -r murmur_userlist
