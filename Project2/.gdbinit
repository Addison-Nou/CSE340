set logging file gdb.output
set logging on
exec-file ./a.out

run < tests/test01.txt 2

set logging off

