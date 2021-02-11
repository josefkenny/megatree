#!/bin/bash

# This isn't a "normal" build system
# 
# How to use it?
# --------------
#
# Writing your own code:
#   Alter the PROJECTNAME variable in this script, replacing it with the name of your project.
#
#   Make a file in `src/` with the name `$PROJECTNAME.c`
#   This file will contain the `main()` function. 
#
#   Other `.c` files can be created and their functions and symbols will be automatically usable
#   in each others' source files, as this build script uses `makeheaders` to generate a single,
#   common header file called `common.h`, which is placed in `src/`.
#
#   To enable this to work, every file must contain the line `#include "common.h"`
#
#   No other header files are necessary for files in this project, and the only `#include`
#   statements required are for standard libraries (e.g. `#include <stdlib.h>`) or for other
#   external libraries specified in the LIBRARIES and INCLUDE_DIRECTORIES variables 
#   detailed below (e.g. `#include "SDL/SDL.h"` when ` LIBRARIES="-lSDLmain -lSDL" ` )
#
#   Note: due to the way `makeheaders` exports symbols, structure declarations used
#   outside their source file,must be prefixed with `#if INTERFACE` and suffixed with `#endif`
#
#
# Dependencies:
#   Dependencies must consist of .c files written in the style above.
#   Either manually place them in the folder `src/include/` or modify the `get_dependencies.sh` 
#   script to retrieve the files (e.g. frp, a GitHub raw link)
#   Their symbols will be added to the `common.h` header file, so can be used in any other file.
#
#
#
# External libraries and includes:
#   LIBRARIES (e.g. `-lmingw32`) can be added to the LIBRARIES variable below.
#
#   INCLUDE DIRECTORIES (e.g. `-Idirectory/subdirectory`) which contain header files referenced in
#   `#include` directives can be added to the INCLUDE_DIRECTORIES variable below.
#
#
# Compiling:
#   Simply run this script.
#
#   First, `makeheaders` will be run to gather all symbols from all `.c` source files in
#   `src/` and `src/include/` into a single header file, placed in `src/`, called `common.h`
#
#   Then, checksums will be generated for all `.c` source files. If existing checksum files inside
#   `tmp/` do not match these checksums, the corresponding `.c` source file will each be compiled
#   into an object file `.o`, also inside `tmp/`. If you wish to compile the project from scratch, 
#   run this script with the argument `-clean`, and all `.o` and `.checksum` files will be deleted.
#
#   Finally, the object files will be linked, and output as $PROJECT or $PROJECT.exe inside `bin/`
#
#  
# Microcontroller projects:  
#   TODO. See this: https://www.pjrc.com/teensy/loader_cli.html
#
# 
# Generating documentation:
#   Documentation for all code will be generated in a file called $PROJECTNAME_documentation.html
#   This is done on each compile cycle. It should be browsable in your average web browser
# 
# 
# Why not a makefile?
# -------------------
# 
# I've written exactly 8 makefiles in my life and at this point I think I know them well enough that I 
# can conclusively say one of two things must be true about them:
#
# 	1. They're bad
# 	2. My brain is too dumb and stupid to use them
#
# Either way, I'd like to now stop writing makefiles and instead start un-writing makefiles. This means 
# reimplementing them as shell scripts. Maybe soon I'll start reimplementing the shell scripts as binaries!
# 
# I think if you have to use a makefile to automate a process then the process you're automating probably 
# needs to be less complicated from the outset, or maybe you should be using something more suitable like 
# Prolog or at least something with actual syntax. 
#
# If you really enjoy makefiles and writing them is a form of recreation for you, then by all means, 
# continue, but I will only run them and never read them!!
# 
# josef 2021-02-10



# This is the c file inside src/ which has main() in it
# So if it's called project.c, PROJECTNAME=project
PROJECTNAME=test_megatree

GCC_PARAMETERS="-w -ffast-math -O2 -static-libgcc"

LIBRARIES="-lSDLmain -lSDL"

INCLUDE_DIRECTORIES=""


echo "Compilerun running"

# Check we have makeheaders
if ! command -v makeheaders &> /dev/null
then
    echo "'makeheaders' could not be found"
    echo ""
    echo "Please download http://www.hwaci.com/sw/mkhdr/makeheaders.c  (99253 bytes, shasum = 6d58c6d765c30ee7f2861fcada9b74a3af51d6a9)"
    echo "and compile it (e.g. 'gcc makeheaders.c -o makeheaders') and then install it by moving 'makeheaders' into a directory in your PATH,"
    echo "for instance, the first directory returned by 'where bash'"
    echo ""
    exit
fi


# Parse command line args
if [ $1 = "--valgrind" ]; then
	DO_VALGRIND=1
fi



# Check directories
if [ ! -e src/ ]; then 
	echo "Error: Source directory does not exist. Please create src/ and put your code in it!"; 
	exit
fi

# Create directories
mkdir -p tmp/
mkdir -p bin/
mkdir -p src/
mkdir -p src/include/

echo "Cleaning project..."
rm tmp/*


# This is set to 1 if the last compile command failed in some way
# so don't bother running the program
LAST_COMPILE_FAILED=0


# If a change has been detected, the headers should be regenerated
# unless they have already been
HEADER_ALREADY_GENERATED=0

# True if a source file has changed
CHANGE_DETECTED=0


# Pre-preprocess files to create headers
make_headers()
{
	if [ $HEADER_ALREADY_GENERATED -eq 0 ]; then

		echo "Generating headers with makeheaders..."
		echo ""

		# Very important, as it makes globbing for files that don't exist return nothing
		# rather than the pattern
		shopt -s nullglob

		makeheaders -h src/*.c src/include/*.c > src/common.h
		HEADER_ALREADY_GENERATED=1

		shopt -u nullglob
	fi
}

###################################################################################################
# Compilation for Linux
compile_linux()
{
	make_headers


	# Compile the file
	gcc -c -ggdb -std=c99 $1 $GCC_PARAMETERS

	if [ $? -eq 0 ]; then
    	LAST_COMPILE_FAILED=0
	else
	    LAST_COMPILE_FAILED=1
	fi

	
}

link_linux()
{
	mv *.o tmp/

	killall -q $PROJECTNAME
	tput reset

	echo "You've written $(cat src/*.c | wc -l) lines of code! Good job" &

	gcc tmp/*.o -o bin/$PROJECTNAME -lm $LIBRARIES $INCLUDE_DIRECTORIES

	if [ $? -eq 0 ]; then
    	LAST_COMPILE_FAILED=0
	else
	    LAST_COMPILE_FAILED=1
	fi
}

run_linux()
{
	if [ $DO_VALGRIND -eq 1 ]; then
	    valgrind --leak-check=yes ./bin/$PROJECTNAME > bin/log.valgrind 2>&1
	    exit
	else
	    ./bin/$PROJECTNAME &
	fi
	
}


###################################################################################################
# Compilation for MinGW on Windows
compile_mingw()
{
	make_headers


	# Compile the file
	gcc -c -std=c99 $1 $GCC_PARAMETERS $INCLUDE_DIRECTORIES

	if [ $? -eq 0 ]; then
    	LAST_COMPILE_FAILED=0
	else
	    echo "Compile failed!"
	    LAST_COMPILE_FAILED=1
	fi


}

link_mingw()
{
	mv *.o tmp/

	clear
	taskkill //IM $PROJECTNAME.exe //F
	clear

	echo "You've written $(cat src/*.c | wc -l) lines of code! Good job" &

	OBJECT_FILES=`echo tmp/*.o`

	echo "$OBJECT_FILES"

	gcc -ggdb $OBJECT_FILES -lmingw32 $LIBRARIES $INCLUDE_DIRECTORIES -o bin/$PROJECTNAME.exe 

	if [ $? -eq 0 ]; then
    	LAST_COMPILE_FAILED=0
	else
	    LAST_COMPILE_FAILED=1
	fi
}

run_mingw()
{
	clear
	taskkill //IM $PROJECTNAME.exe //F
	clear

	echo "Running bin/$PROJECTNAME.exe ..."

	# This executes in a subshell and moves to the new working directory of /bin
	(cd bin; ./$PROJECTNAME.exe > $PROJECTNAME.log) &
}



###################################################################################################
# Check one file to see if it needs to be recompiled
compile_file_if_needed()
{
	# Check their checksum
	FILE_TO_CHECK=$1
	FILE_TO_CHECK_NAME=`basename $FILE_TO_CHECK`
	OLD_SUM_FILE=tmp/$FILE_TO_CHECK_NAME.checksum

	echo "Checking $FILE_TO_CHECK_NAME's checksum ($OLD_SUM_FILE)"

	if [ -e $OLD_SUM_FILE ]
	then
	    OLD_SUM=`cat $OLD_SUM_FILE`
	else
	    OLD_SUM="nothing"
	fi

	NEW_SUM=`sha1sum $FILE_TO_CHECK | awk '{print $1}'`

	echo "NEW_SUM = $NEW_SUM"

	if [ "$OLD_SUM" != "$NEW_SUM" ]
	then
	    echo "found a change"

		if cat /etc/*-release > /dev/null;  # Detect Linux or Windows (MinGW)
		then 
			compile_linux $FILE_TO_CHECK
		else
			compile_mingw $FILE_TO_CHECK
		fi

		# update old sum
	    echo "$NEW_SUM" > $OLD_SUM_FILE 
	fi
}



while :
do
	# Iterate through all files src/*.c and src/include/*.c
	# Recompile them if needed

	for i in `find . -name "*.c" -type f`; do
    	if [ $LAST_COMPILE_FAILED -eq 1 ]; then
	    	break
	    fi

	    compile_file_if_needed $i
	done


	



	# If compilation was a success, link the files
	if [ $LAST_COMPILE_FAILED -eq 0 ]; then
	    
		if cat /etc/*-release > /dev/null;  # Detect Linux or Windows (MinGW)
		then 
			link_linux
		else
			link_mingw
		fi
	fi


	# If compile and link was a success, run the program
	if [ $LAST_COMPILE_FAILED -eq 0 ]; then
		if cat /etc/*-release > /dev/null;  # Detect Linux or Windows (MinGW)
		then 
			run_linux
		else
			run_mingw
		fi
	fi

	
	sleep 4	# Loop every 4 seconds

	LAST_COMPILE_FAILED=0	# Retry
	HEADER_ALREADY_GENERATED=0
done


taskkill /IM sleep.exe /F