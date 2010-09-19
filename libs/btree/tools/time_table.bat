bjam --toolset=gcc-4.5
echo "GCC/MinGW 4.5" >time_table.html
bin\bt_time 5000000 -stl -p4096 -c32 -native -html 2>>time_table.html
type time_table.html
bin\bt_time 5000000 -stl -p4096 -c1000 -native -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c10000 -native -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c10000 -big -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c10000 -little -html 2>>time_table.html
bin\bjam --toolset=msvc-10.0express
echo "VC++ 10.0 Express" >>time_table.html
bin\bt_time 5000000 -stl -p4096 -c32 -native -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c1000 -native -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c10000 -native -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c10000 -big -html 2>>time_table.html
bin\bt_time 5000000 -stl -p4096 -c10000 -little -html 2>>time_table.html
type time_table.html

