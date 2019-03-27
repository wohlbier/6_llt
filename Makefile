HDRS = algebra.hh types.hh
SRCS = main.cc
EMU_OBJS = $(subst .cc,.emu.o,$(SRCS))

#EMU_PATH = /local/devel/packages/emu-18.11-cplus
#EMU_PATH = /local/devel/packages/emu-19.02
EMU_PATH = /home/jgwohlbier/devel/packages/emu-19.02
EMU_CXX = $(EMU_PATH)/bin/emu-cc
EMU_SIM = $(EMU_PATH)/bin/emusim.x

EMU_SIM_ARGS =
#EMU_SIM_ARGS += --short_trace
#EMU_SIM_ARGS += --memory_trace

EMU_PROFILE = $(EMU_PATH)/bin/emusim_profile

EXE  = llt
EMU_EXE = $(EXE).mwx
#INPUT = tri-1.tsv
#INPUT = tri-3.tsv
#INPUT = tri-63.tsv
INPUT = tri-184.tsv
#INPUT = tri-379.tsv
#INPUT = tri-994.tsv

$(EMU_EXE) : $(EMU_OBJS)
	$(EMU_CXX) -o $(EMU_EXE) $(EMU_OBJS) $(LDFLAGS)

run : $(EMU_EXE)
	$(EMU_SIM) $(EMU_SIM_ARGS) $(EMU_EXE) $(INPUT)

profile : $(EMU_EXE)
	$(EMU_PROFILE) profile $(EMU_SIM_ARGS) -- $(EMU_EXE) $(INPUT)

%.emu.o: %.cc $(HDRS)
	$(EMU_CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY : clean

clean :
	-$(RM) *~ $(OBJS) $(EMU_OBJS) $(EXE) $(EMU_EXE) *.cdc *.hdd *.vsf
	-$(RM) -r profile $(EXE).txt
