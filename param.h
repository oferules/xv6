#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks
#define QUANTUM 	 5 	 /// number of clock ticks between sched swap
#define ALPHA      0.5 /// alpha for SRT
#define MIN_PRIORITY    1 /// minimum priority of a process
#define MAX_PRIORITY    3 /// maximum priority of a process
/// variables constants
#define MAX_VARIABLES 32 /// global variables capcity
#define MAX_VAR_NAME 32  /// maimum length of variable name
#define MAX_VAR_VALUE 100  // maximum length of variable 
