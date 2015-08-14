
typedef unsigned int uint32_t;
typedef char Value;
typedef char State;
typedef char * Expr;


void Register_librecovery_sofia3gr_intel();
Value* IntelSofiaTestFn(const char* name, State* state, int argc, Expr* argv[]);
Value* IntelUpdateBootImage(const char* name, State* state, int argc, Expr* argv[]);
int writeBootimageFileToDest(void* pImage, int dataToWrite, char* output, unsigned int blockSizek);


#ifndef TESTEXE
#define malloc fake_malloc
#define read fake_read
#define write fake_write
#define open fake_open
#define lseek fake_lseek
#define ioctl fake_ioctl
#define uname fake_uname
#endif
