# Define the compiler and compiler flags
CC = gcc
# gcc-13 for mac
# CFLAGS = -Wall -Wextra -Wshadow -Wpedantic -Wformat -Wconversion -fsanitize=undefined -fsanitize=leak

# CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow -Wformat=2 \
#          -Wfloat-equal -Wcast-qual -Wcast-align -Wpointer-arith -Wwrite-strings \
#          -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition \
#          -Wmissing-declarations -Winline -Wundef -Wredundant-decls \
#          -Wno-unused-parameter -Wstack-protector -fstack-protector-strong \
#          -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fsanitize=address -O2

# Define the target executable
TARGET = shell

# List of source files
SRCS = hop/hop.c \
       reveal/reveal.c \
       log/log.c \
       proclore/proclore.c \
       syscomm/syscomm.c \
       seek/seek.c \
       iMan/iMan.c \
       shrc/shrc.c \
       activites/activities.c \
       signals/signals.c \
       neonate/neonate.c \
       processes/process.c \
       base/base.c \
       init/init.c

# Mark 'all' as a phony target so it's always executed
.PHONY: all clean

# Default rule to build the executable
all: $(TARGET)

# Rule to compile and link the source files directly into the final executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean rule to remove the executable
clean:
	rm -f $(TARGET)

clear:
	rm -f $(TARGET)






