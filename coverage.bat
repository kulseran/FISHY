del LastCoverageResults.log
FOR /D %%X IN (CoverageReport*) DO RD /S /Q "%%X"
OpenCppCoverage.exe --working_dir ./ --sources %cd%\* --excluded_sources %cd%\extern_lib\* --excluded_sources %cd%\TESTS\* -- .\.build\bin\Debug\tests.exe --log_verbosity 0 --haltonerror false
