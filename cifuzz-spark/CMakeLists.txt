#cifuzz:build-template:begin
#add_fuzz_test({{ .FuzzTestName }} {{ .FileName }})
#target_link_libraries({{ .FuzzTestName }} PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,Glob>")
#cifuzz:build-template:end
add_fuzz_test(fuzz_compile_pattern_to_regex fuzz_compile_pattern_to_regex.cpp)
target_link_libraries(fuzz_compile_pattern_to_regex PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,Glob>")
