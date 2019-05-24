#!/bin/bash

DIR="/dist_libs"

apt-get install -y doxygen
doxygen -g Doxyfile

# Generating documentation for DataBusAbstraction c modules
PROJECT_NAME="DataBusAbstraction"

INPUT=$DIR"/DataBusAbstraction/c"
mkdir -p $INPUT/"doc"
OUTPUT=$INPUT/"doc"

(cat  Doxyfile; echo "PROJECT_NAME = "$PROJECT_NAME; echo "OUTPUT_DIRECTORY = "$OUTPUT; \
echo "EXTRACT_ALL = YES"; echo "INPUT = "$INPUT; echo "FILE_PATTERNS = *.h"; \
echo "RECURSIVE = YES"; echo "EXCLUDE = "$INPUT"/open62541.h"; \
echo "EXCLUDE += "$INPUT"/common.h"; echo "EXCLUDE_PATTERNS = */test/*"; \
echo "GENERATE_LATEX = NO"; echo "HAVE_DOT = NO") | doxygen -

# Generating documentation for DataBusAbstraction py modules
INPUT=$DIR"/DataBusAbstraction/py"
mkdir -p $INPUT/"doc"
OUTPUT=$INPUT/"doc"

(cat  Doxyfile; echo "PROJECT_NAME = "$PROJECT_NAME; echo "OUTPUT_DIRECTORY = "$OUTPUT;\
echo "EXTRACT_ALL = YES"; echo "INPUT = "$INPUT; echo "FILE_PATTERNS = *.py"; \
echo "RECURSIVE = YES"; echo "EXCLUDE = "$INPUT"/DataBusOpcua.py "; \
echo "EXCLUDE_PATTERNS = */test/*"; echo "EXCLUDE_PATTERNS += */Util/*"; \
echo "GENERATE_LATEX = NO"; echo "HAVE_DOT = NO") | doxygen -

# Generating documentation for ImageStore cpp modules
PROJECT_NAME="ImageStore"
INPUT=$DIR"/ImageStore/client/cpp"
mkdir -p $INPUT/"doc"
OUTPUT=$INPUT/"doc"

(cat  Doxyfile; echo "PROJECT_NAME = "$PROJECT_NAME; echo "OUTPUT_DIRECTORY = "$OUTPUT; \
echo "EXTRACT_ALL = YES"; echo "INPUT = "$INPUT; echo "FILE_PATTERNS = *.cc"; \
echo "RECURSIVE = YES"; echo "GENERATE_LATEX = NO"; echo "HAVE_DOT = NO") | doxygen -

# Generating documentation for ImageStore py modules
INPUT=$DIR"/ImageStore/client/py"
mkdir -p $INPUT/"doc"
OUTPUT=$INPUT/"doc"

(cat  Doxyfile; echo "PROJECT_NAME = "$PROJECT_NAME; echo "OUTPUT_DIRECTORY = "$OUTPUT; \
echo "EXTRACT_ALL = YES"; echo "INPUT = "$INPUT; echo "FILE_PATTERNS = *.py"; \
echo "RECURSIVE = YES"; echo "EXCLUDE_PATTERNS = */test/*"; \
echo "GENERATE_LATEX = NO"; echo "HAVE_DOT = NO") | doxygen -
