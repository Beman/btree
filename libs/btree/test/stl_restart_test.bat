stl_equivalence_test -max=10 -min=5 -cycles=2 -v
copy /y restart.* restart2.*
stl_equivalence_test -max=10 -min=5 -cycles=1 -v
stl_equivalence_test -restart -cycles=1 -v
bc3 stl_equivalence_test.btr restart2.btr
