#format is target-name: target dependencies
cFile = task1.c
oFile = task1.o
programName = myELF
# All Targets
all: $(programName)

# Tool invocations
# Executable "words" depends on the file count-words.o
$(programName): $(oFile)
	gcc -g -m32 -Wall -o $(programName) $(oFile)

$(oFile): $(cFile)
	gcc -m32 -g -Wall -c -o $(oFile) $(cFile)

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o $(programName)