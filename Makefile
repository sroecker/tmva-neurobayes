#
#Makefile for standalone plugin lib for NeuroBayes in TMVA
#

NEUROBAYES_LIB:=$(NEUROBAYES)/lib
NEUROBAYES_INC:=$(NEUROBAYES)/include

#include Makefile.arch
# get the correct root configuration
ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)
ROOTINC      := $(shell root-config --incdir)

ROOTVERSION  = ROOT$(shell root-config --version | sed -e 's/\.//' -e 's/\/.*//')

# set the compiler options
CXX           = g++        #use g++ as compiler
CXXFLAGS      = -fPIC #-g -Wall  #set compiler options

# set the linker options
LD            = g++                #use g++ for linking
LDFLAGS       = -O           
SOFLAGS       = -shared

#######################
CXXFLAGS     += $(ROOTCFLAGS) 
NEUROBAYESLIBS= -L$(NEUROBAYES_LIB) -lNeuroBayesExpertCPP -lNeuroBayesTeacherCPP
LIBS          = $(ROOTLIBS) $(NEUROBAYESLIBS) $(SYSLIBS) 

MAKEFLAGS = 
LD_LIBRARY_PATH:=.:$(ROOTSYS)/lib:$(LD_LIBRARY_PATH)
INCLUDES += -I$(ROOTINC) -I$(NEUROBAYES_INC) -Iinc

PACKAGE=TMVA_NeuroBayes

LIBFILE   = libTMVANeuroBayes.so

SRCDIR = src
OBJDIR = $(SRCDIR)
DEPDIR = $(SRCDIR)
INCDIR = inc
DICTHEAD  = $(PACKAGE)_Dict.h
DICTFILE  = $(PACKAGE)_Dict.C
DICTOBJ   = $(PACKAGE)_Dict.o
DICTLDEF  = $(INCDIR)/LinkDef.h
SKIPHLIST = $(DICTLDEF)

# List of all source files to build
HLIST     = $(filter-out $(SKIPHLIST),$(wildcard $(INCDIR)/*.h))
HLIST     += $(wildcard $(NEUROBAYES_INC)/*.h)
CPPLIST   := $(wildcard $(SRCDIR)/*.cxx)
DICTHLIST  =   $(filter-out $(SKIPHLIST),$(wildcard $(INCDIR)/*.h))
OBJECTS   := $(CPPLIST:.cxx=.o)



# Implicit rule to compile all classes
default: $(LIBFILE)


$(OBJDIR)/%.o : $(SRCDIR)/%.cxx 
	@printf "Compiling $< ... "
	@$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $< -o $@
	@echo "Done"


$(DICTFILE):  $(DICTHLIST) $(DICTLDEF)
	@echo "Generating dictionary $@" 
	@echo "$^" 
	@$(ROOTSYS)/bin/rootcint -f $@ -c $(CXXFLAGS) $(INCLUDES) -p $^


$(OBJDIR)/$(DICTOBJ): $(DICTFILE)
	@echo "Compiling $<"
	@mkdir -p $(OBJDIR)
	@$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c  -o $@ $<

$(LIBFILE):  $(OBJECTS) $(OBJDIR)/$(DICTOBJ)
	@printf "Building shared library $(LIBFILE) ... "
	@rm -f $(LIBFILE)
	@$(LD) $(LIBS)  $(SOFLAGS) $(OBJECTS) $(OBJDIR)/$(DICTOBJ) -o $(LIBFILE)
	@echo "Done"

clean:
	@rm -f $(DICTFILE) $(DICTHEAD)
	@rm -f $(OBJDIR)/*.o
	@rm -f $(LIBFILE)
	@rm -f ../lib/lib$(PACKAGE).1.so

install:
	@cp libTMVANeuroBayes.so $(ROOTSYS)/lib
	@echo "libTMVANeuroBayes.so copied to $(ROOTSYS)/lib"
