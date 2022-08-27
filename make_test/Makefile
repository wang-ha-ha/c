include .config

.PHONY: all clean $(APPS) 

all clean:$(APPS)

$(APPS):
	@if [ -e "$@" ] && ( [ -z ${SUB} ] || [ "${SUB}" = "$@" ] ); then \
		if [ "$(MAKECMDGOALS)" = "all" ] && [ -f "$@/make_pre.sh" ]; then	\
			cd $@; sh ./make_pre.sh; cd -; \
		fi; 	\
		$(MAKE) -C $@ $(MAKECMDGOALS);\
		if [ "$(MAKECMDGOALS)" = "all" ] && [ -f "$@/make_post.sh" ]; then	\
			cd $@; sh ./make_post.sh; cd -; \
		fi; 	\
	fi
