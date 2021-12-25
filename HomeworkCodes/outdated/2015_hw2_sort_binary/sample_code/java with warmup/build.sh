#lang=java
rm -rf src
mkdir src
mkdir src/main
mkdir src/main/java
cp -f code src/main/java/code.java
sbt -no-colors assembly



