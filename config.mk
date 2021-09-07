VERSION = 0.1

# release flags
#CFLAGS = -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wall -fPIC -Werror -Wpedantic -O2 -pthread

# debug flags
CFLAGS = -Wall -Werror -Wpedantic -O0 -g -fPIC -pthread
LDFLAGS = -pthread

TST_CFLAGS = -fsanitize=address
#TST_CFLAGS += -fprofile-arcs
#TST_CFLAGS += -ftest-coverage
FZZ_CFLAGS = -fsanitize=fuzzer,address -fcoverage-mapping -fprofile-instr-generate -g

# if you're feeling lucky:
#CFLAGS += -DNDEBUG

## Comment this in for profiling
#LDFLAGS += -lprofiler
