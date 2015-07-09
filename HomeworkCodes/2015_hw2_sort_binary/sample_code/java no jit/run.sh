#@lang=java
#@memcheck=false
java $(tr '\n\r;' ' ' < jvm.options)  -jar target/homework-assembly-1.0-SNAPSHOT.jar 
