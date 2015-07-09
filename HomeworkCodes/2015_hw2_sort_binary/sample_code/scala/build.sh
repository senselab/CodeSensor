#lang=scala
rm -rf src
mkdir src
mkdir src/main
mkdir src/main/scala
cp -f code src/main/scala/code.scala
sbt -no-colors assembly


