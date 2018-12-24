### USER CONFIGURATION #########################################################

# Boost libraries
#boost_home=$(CURDIR)/thirdparty/boost_1_63_0
boost_home=

# CPLEX Studio (for CPLEX Optimizer and CP Optimizer)
#cplex_home=$(CURDIR)/thirdparty/ibm/ILOG/CPLEX_Studio127
cplex_home=

################################################################################

ifndef boost_home
$(error Variable 'boost_home' is undefined)
endif

ifndef cplex_home
$(error Variable 'cplex_home' is undefined)
endif


project_path=$(CURDIR)

CC=$(CXX)
CXXFLAGS+=-Wall -Wextra -std=c++11 -pedantic
CXXFLAGS+=-g -Og -UNDEBUG
#CXXFLAGS+=-O3 -DNDEBUG

CXXFLAGS+=-I$(project_path)/include
CXXFLAGS+=-DDCS_LOGGING_STREAM=std::cout
CXXFLAGS+=-DDCS_DEBUG_STREAM=std::cout
LDLIBS+=-lm

# Boost libraries
CXXFLAGS+=-I$(boost_home)

# Optimizers
CXXFLAGS+=-DDCS_FGT_GT_USE_NATIVE_CP_SOLVER
CXXFLAGS+=-I$(cplex_home)/cplex/include -I$(cplex_home)/cpoptimizer/include -I$(cplex_home)/concert/include -DIL_STD
#CXXFLAGS += -O -DNDEBUG -fPIC -fstrict-aliasing -fexceptions -frounding-math -Wno-long-long -m64 -DILOUSEMT -D_REENTRANT -DILM_REENTRANT
LDFLAGS+=-L$(cplex_home)/cplex/lib/x86-64_linux/static_pic -L$(cplex_home)/cpoptimizer/lib/x86-64_linux/static_pic -L$(cplex_home)/concert/lib/x86-64_linux/static_pic
LDLIBS+=-lilocplex -lcp -lcplex -lconcert -lm -lpthread


.PHONY: all clean


all: src/fog_coalform

clean:
	$(RM) src/fog_coalform \
		  src/*.o
