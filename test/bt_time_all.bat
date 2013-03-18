bt_time 5000000 -stl -p4096 -c32 -native -html 2>times.html
bt_time 5000000 -stl -p4096 -c2100 -native -html 2>>times.html
bt_time 5000000 -stl -p4096 -c4200 -native -html 2>>times.html
bt_time 5000000 -stl -p4096 -c8400 -native -html 2>>times.html
bt_time 5000000 -stl -p4096 -c8400 -little -html 2>>times.html
bt_time 5000000 -stl -p4096 -c8400 -big -html 2>>times.html
type times.html
