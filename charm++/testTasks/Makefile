#-include ../../common.mk
#CHARMC=../../../bin/charmc $(OPTS)
#CHARMC=/Users/kale/software/taskqCharm/charm/bin/charmc $(OPTS)
#let us get CHARM from the environment.. to avoid merge issues

OBJS = hello.o

all: hello

hello: $(OBJS)
	$(CHARMC) -O3 -language charm++ -tracemode projections -o hello $(OBJS)

hello.decl.h: hello.ci
	$(CHARMC)  hello.ci

clean:
	rm -f *.decl.h *.def.h conv-host *.o hello charmrun *.log *.sum *.sts hello.exe hello.pdb hello.ilk

hello.o: hello.C hello.decl.h
	$(CHARMC) -O3 -c hello.C

test: all
	$(call run, ./hello +p4 10 )

testp: all
	$(call run, ./hello +p$(P) $$(( $(P) * 10)) )

smptest: all
	$(call run, ./hello 10 +p2 ++ppn 2 )
	$(call run, ./hello 10 +p4 ++ppn 2 )
