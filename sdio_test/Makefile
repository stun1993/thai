LIST=CPU
ifndef QRECURSE
QCONFIG=qconfig.mk
QRECURSE=recurse.mk
ifdef QCONFIG
QRDIR=$(dir $(QCONFIG))
endif
endif
include $(QRDIR)$(QRECURSE)