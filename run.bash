# For C
gcc Server.c -lsqlite3 && ./a.out 8080

# For Cpp
g++ Server.cpp -lsqlite3 && ./a.out 8080

# For Java
java -cp ".:sqlite-jdbc-3.7.2.jar" Server.java 8080

# For Python
python3.11 Server.py 8080