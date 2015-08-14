
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>

#define TESTEXE
#include "hosttest.h"

#define TEST_FILE_SIZE 4096

//Test functions.
static int fail_malloc = -1;
static int fail_uname = -1;
static int fail_write = -1;
static int fail_open = -1;
static int fail_lseek = -1;
static int fail_ioctl = -1;
static int fail_magic = -1;
static int fail_read = -1;
static int fail_magic_read = -1;
static int fail_read_args = -1;


static int fake_uname_fail = 0;
static int second_part_magic = 0;
static int must_fail = 0;

char * fake_malloc(size_t n) {
	if(fail_malloc == 0) {
		fail_malloc--;
		return NULL;
	} else {
		fail_malloc--;
		return malloc(n);
	}
}

int fake_uname(struct utsname *ut) {
	if(fake_uname_fail == 1) {
		uname(ut);
		ut->release[0] = '2';
		ut->release[2] = '4';
		printf("<Fake uname() result>\n");
		return 0;
	} else if(fail_uname == 0) {
		printf("<Fake uname() error>\n");
		fail_uname--;
		return -1;
	} else {
		fail_uname--;
		return uname(ut);
	}
}

ssize_t fake_write(int fd, const void *buf, size_t n) {
	if(n == sizeof(uint32_t)) {
		if(fail_magic == 0) {
			printf("<Fake magic write() error>\n");
			fail_magic--;
			return -1;
		} else {
			fail_magic--;
			return write(fd, buf, n);
		}
	} else {
		if(fail_write == 0) {
			printf("<Fake write() error>\n");
			fail_write--;
			return -1;
		} else {
			fail_write--;
			return write(fd, buf, n);
		}
	}
}

ssize_t fake_read(int fd, void *buf, size_t n) {
	if(n == sizeof(uint32_t)) {
		if(fail_magic_read == 0) {
			printf("<Fake magic read() error>\n");
			fail_magic_read--;
			return -1;
		} else {
			fail_magic_read--;
			return read(fd, buf, n);
		}
	} else {
		if(fail_read == 0) {
			printf("<Fake read() error>\n");
			fail_read--;
			return -1;
		} else {
			fail_read--;
			return read(fd, buf, n);
		}
	}
}


int fake_open(char *name, int flag, int perm) {
	if(fail_open == 0) {
		printf("<Fake open() error>\n");
		fail_open--;
		return -1;
	} else {
		fail_open--;
		return open(name, flag, perm);
	}
}

off_t fake_lseek(int fd, off_t offset, int flag) {
	if(fail_lseek == 0) {
		printf("<Fake lseek() error>\n");
		fail_lseek--;
		return -1;
	} else {
		fail_lseek--;
		return lseek(fd, offset, flag);
	}
}

int fake_ioctl(int fd, int flag, off_t *res) {
	if(fail_ioctl == 0) {
		printf("<Fake ioctl() error>\n");
		fail_ioctl--;
		return -1;
	} else {
		if(fake_uname_fail == 1) {
			printf("<Fake ioctl() result>\n");
			*res = TEST_FILE_SIZE/512;
		} else {
			*res = TEST_FILE_SIZE;
		}
		fail_ioctl--;
		return 0;
	}
}


//Fake functions.
int ReadArgs(State *state, Expr *expr, int count, char **out0, char **out1, char **out2) {
	if(fail_read_args == 0) {
		fail_read_args--;
		return -1;
	}

	fail_read_args--;

	switch(count) {
	case 3:
		*out2 = expr[2];

	case 2:
		*out1 = expr[1];

	case 1:
		*out0 = expr[0];
		break;

	default:
		return -1;
		break;
	}

	return 0;
}

Value * StringValue(char *val) {
	return strdup(val);
}

void RegisterFunction(char *name, ...) {
}


//---------------------------

char * readfiledata(char *name) {
	char *testdata = malloc(TEST_FILE_SIZE);

	int testfd = open(name, O_RDWR);

	if(TEST_FILE_SIZE != read(testfd, testdata, TEST_FILE_SIZE)) {
		fprintf(stderr, "Could not read test data from file\n");
		exit(-1);
	}

	close(testfd);

	return testdata;
}

int runtest3() {
	static Expr test3arg[3] = {"testImg", "testPart1", "testPart2"};

	char *testdata = malloc(TEST_FILE_SIZE);
	int i;

	for(i = 0; i < 3; i++) {
		int testfd = open(test3arg[i], O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);

		if(second_part_magic == 1 && i == 2) {
			memset(testdata, 0, TEST_FILE_SIZE);
		} else {
			memset(testdata, 'A' + i, TEST_FILE_SIZE);
		}

		if(TEST_FILE_SIZE != write(testfd, testdata, TEST_FILE_SIZE)) {
			fprintf(stderr, "Could not write test data to file\n");
			exit(-1);
		}

		close(testfd);
	}


	Value *res = IntelUpdateBootImage("test", "test", 3, (Expr **) test3arg);

	int testres = 0;

	if(must_fail == 0 && res != NULL && 0 == strlen(res)) {
		memset(testdata, 'A', TEST_FILE_SIZE);

		char *part1data = readfiledata(test3arg[1]);
		char *part2data = readfiledata(test3arg[2]);

		if(0 != memcmp(testdata, part1data, TEST_FILE_SIZE - sizeof(uint32_t)) || 0 != memcmp(testdata, part2data, TEST_FILE_SIZE - sizeof(uint32_t))) {
			fprintf(stderr, "* >>>Test failed<<<\n");
			testres = 1;
		} else {
			printf("* Test passed\n");
		}

		free(part1data);
		free(part2data);
	} else if(res == NULL && must_fail == 1) {
		printf("* Test passed\n");
	} else {
		fprintf(stderr, "* >>>Test failed<<<\n");
		testres = 1;
	}

	free(testdata);

	return testres;
}

void testheader(int *counter, char *title) {
	(*counter)++;
	printf("* Running test %d%s:\n", *counter, title);
}

int main(int argc, char **argv) {
	int failcount = 0;
	int testnumber = 0;
	int i;

	printf("Running host test\n");

	//------------------------
	testheader(&testnumber, NULL);
	Register_librecovery_sofia3gr_intel();

	//------------------------
	//Test readargs errors.
	testheader(&testnumber, " (ReadArgs() ok)");
	static Expr test2arg[3] = {"test0", "test1", "test2"};

	Value *res = IntelSofiaTestFn("test", "test", 3, (Expr **) test2arg);

	if(res == NULL || 0 != strlen(res)) {
		fprintf(stderr, "* >>>Test failed<<<\n");
		failcount++;
	}

	testheader(&testnumber, " (IntelSofiaTestFn())");
	fail_read_args = 0;

	res = IntelSofiaTestFn("test", "test", 3, (Expr **) test2arg);

	if(res != NULL) {
		fprintf(stderr, "* >>>Test failed<<<\n");
		failcount++;
	}
	fail_read_args = -1;

	//------------------------
	//Test no errors.
	testheader(&testnumber, " (no errors)");
	failcount += runtest3();

	//Second partition magic OK.
	second_part_magic = 1;
	testheader(&testnumber, " (second partiton)");
	failcount += runtest3();
	second_part_magic = 0;

	must_fail = 1;

	//Readargs fail.
	fail_read_args = 0;
	testheader(&testnumber, " (ReadArgs() fail)");
	failcount += runtest3();
	fail_read_args = -1;

	//Test malloc errors.
	for(i = 0; i < 1; i++) {
		testheader(&testnumber, " (malloc() fail)");
		fail_malloc = i;
		failcount += runtest3();
	}
	fail_malloc = -1;


	//Test ioctl errors.
	for(i = 0; i < 4; i++) {
		testheader(&testnumber, " (ioctl() fail)");
		fail_ioctl = i;
		failcount += runtest3();
	}
	fail_ioctl = -1;


	//Test seek errors.
	for(i = 0; i < 9; i++) {
		testheader(&testnumber, " (lseek() fail)");
		fail_lseek = i;
		failcount += runtest3();
	}
	fail_lseek = -1;


	//Test open errors.
	for(i = 0; i < 6; i++) {
		testheader(&testnumber, " (open() fail)");
		fail_open = i;
		failcount += runtest3();
	}
	fail_open = -1;

	//Test write errors.
	for(i = 0; i < 2; i++) {
		testheader(&testnumber, " (write() fail)");
		fail_write = i;
		failcount += runtest3();
	}
	fail_write = -1;

	//Test write magic errors.
	for(i = 0; i < 4; i++) {
		testheader(&testnumber, " (write() fail)");
		fail_magic = i;
		failcount += runtest3();
	}
	fail_magic = -1;


	//Test read errors.
	for(i = 0; i < 1; i++) {
		testheader(&testnumber, " (read() fail)");
		fail_read = i;
		failcount += runtest3();
	}
	fail_read = -1;


	//Test read magic errors.
	for(i = 0; i < 3; i++) {
		testheader(&testnumber, " (read() fail)");
		fail_magic_read = i;
		failcount += runtest3();
	}
	fail_magic_read = -1;


	//Test old uname error.
	testheader(&testnumber, " (uname() ok)");
	must_fail = 0;
	fake_uname_fail = 1;
	failcount += runtest3();
	must_fail = 1;

	//Test uname + ioctl errors.
	for(i = 0; i < 1; i++) {
		testheader(&testnumber, " (uname() + ioctl() fail)");
		fail_ioctl = i;
		failcount += runtest3();
	}
	fail_ioctl = -1;
	fake_uname_fail = 0;

	//Test uname errors.
	for(i = 0; i < 5; i++) {
		testheader(&testnumber, " (uname() fail)");
		fail_uname = i;
		failcount += runtest3();
	}

	printf("Hosttest completed\nNumber of failed tests: %d of %d\n", failcount, testnumber);

	return 0;
}
