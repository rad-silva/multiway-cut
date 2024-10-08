###############################################################################
# Makefile: building instructions using GNU Make utilities.
#
# (c) Copyright 2019-2023, Carlos Eduardo de Andrade.
# All Rights Reserved.
#
# Created on : Mar 05, 2019 by ceandrade
# Last update: Sep 28, 2023 by ceandrade
#
# This code is released under BRKGA-MP-IPR License:
# https://github.com/ceandrade/brkga_mp_ipr_cpp/blob/master/LICENSE.md
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
###############################################################################

###############################################################################
# User Defines
###############################################################################

# Optimization switch. If ommitted/commented, the debug mode is used.
OPT = opt

# Determines how the mating process will happen regarding reproducibility.
# One of the following options should be used. If more than one is given,
# MATING_FULL_SPEED takes priority over MATING_SEED_ONLY, which takes priority
# over MATING_SEQUENTIAL. If no option is supplied, BRKGA-Mp-IPR assume
# MATING_FULL_SPEED.
#
# At full speed, the mating process is done in parallel, using independent RNGs.
# The results are reproducible if and only if you use the same seed and the same
# number of threads.
#USER_FLAGS += -DMATING_FULL_SPEED

# Using the following option, the mating is still parallel, but each RNG is
# seeded on each mating. This is a little bit slower than full speed, but we
# depend only on the seed, regardless of the number of threads. I.e., Runs with
# a different number of threads, but the same seed should result in the same
# sequence of decisions.
# USER_FLAGS += -DMATING_SEED_ONLY

# Using this option, the mating process is completely sequential, as in the
# original BRKGA. The reproducibility is guaranteed with only the same seed.
# This option can be very slow for large chromosomes and large populations.
# But it makes debugging easier.
 USER_FLAGS += -DMATING_SEQUENTIAL

###############################################################################
# Build options
###############################################################################

# Include folders.
INCLUDES = \
	-I. \
	-I./brkga_mp_ipr

# Objects.
OBJS = \
	./mcp/mcp_instance.o \
	./decoders/threshold_decoder.o \
	./decoders/threshold2_decoder.o \
	./decoders/threshold3_decoder.o \
	./decoders/coloracao_decoder.o \
	./decoders/coloracao2_decoder.o \
	./decoders/coloracao3_decoder.o \
	./decoders/kruskal_decoder.o \
	./decoders/kruskal_pert_decoder.o \
	./decoders/multiplos_cortes_decoder.o \
	./decoders/multiplos_cortes2_decoder.o \
	./max_flow/graph.o \
	./max_flow/highest_push_relabel.o \
	./max_flow/list_node_height.o \
	./max_flow/priority_queue.o \
	./execute_decoders.o \
	./write_file.o\
	./geradores/geradores_cromossomo.o

# Main files.
MAIN_MCP_OBJ = main_mcp.o
MAIN_MCP_EXE = main_mcp


###############################################################################
# Compiler, linker, and flags
###############################################################################

# C++ Compiler command and flags
CXX = g++
# CXX = clang++

USER_FLAGS += -std=c++20 -g

# Compiler flags for debugging
ifneq ($(OPT), opt)
	USER_FLAGS += -ggdb3 -fexceptions -fno-omit-frame-pointer \
		-fno-optimize-sibling-calls -fno-inline 
else
	USER_FLAGS += -O3 -fomit-frame-pointer -funroll-loops
	ifeq ($(CXX), g++)
		USER_FLAGS += -ftracer -fpeel-loops -fprefetch-loop-arrays -flto=auto
	endif
endif

# Include parallel stuff
USER_FLAGS += -pthread -fopenmp

# Warning flags
USER_FLAGS += -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization \
	-Wformat=2 -Winit-self -Wmissing-format-attribute -Wshadow \
	-Wpointer-arith -Wredundant-decls -Wstrict-aliasing=2 \
	-Wfloat-equal -Weffc++

ifeq ($(CXX), g++)
	USER_FLAGS += -Wunsafe-loop-optimizations
endif

# Paranoid mode!!!
# Used to analyze format of structures
# USER_FLAGS += -Wpacked -Wpadded
# USER_FLAGS += -Wconversion

CXXFLAGS = $(USER_FLAGS)

###############################################################################
# Build Rules
###############################################################################

.PHONY: all clean
.SUFFIXES: .cpp .o

all: main_mcp

main_mcp: $(OBJS) $(MAIN_MCP_OBJ)
	@echo "--> Linking objects... "
	$(CXX) $(CXXFLAGS) $(OBJS) $(MAIN_MCP_OBJ) -o $(MAIN_MCP_EXE)
	@echo

.cpp.o:
	@echo "--> Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(USER_DEFINES) -c $< -o $@
	@echo

clean:
	@echo "--> Cleaning compiled..."
	rm -rf $(UTIL_OBJS) $(OBJS) $(MAIN_MCP_OBJ) $(MAIN_MCP_EXE) \
	rm -rf *o
	rm -rf Debug
	rm -rf *.dSYM
