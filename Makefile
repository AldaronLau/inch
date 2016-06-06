ifneq ($(shell echo $(JLL_HOME)), "")
 include $(shell echo $(JLL_HOME))/compile-scripts/ProjectMakefile.mk
else
 $(error "You need to set enviroment variable JLL_HOME to the jl_lib directory")
endif
