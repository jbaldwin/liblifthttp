format:
	# Inlcude *.hpp|*.h|*.cpp but ignore catch lib as well as RelWithDebInfo|Release|Debug|build
	find . \( -name '*.hpp' -or -name '*.h' -or -name '*.cpp' \) 	\
		-and -not -name '*catch*' 									\
		-and -not -iwholename '*/RelWithDebInfo/*' 					\
		-and -not -iwholename '*/Release/*' 						\
		-and -not -iwholename '*/Debug/*' 							\
		-and -not -iwholename '*/build/*' 							\
		-exec clang-format -i --style=file {} \;