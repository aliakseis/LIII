#pragma once

enum { TEST_PORT = 17621 };

enum { TEST_DATA_SIZE = 4630754 };
enum { PARTIAL_DATA_SIZE = 1747945 };


inline unsigned long testRand(unsigned long previous) { return previous * 1103515245 + 12345; }

#define FILE_NAME_REGULAR "regular.dat"
