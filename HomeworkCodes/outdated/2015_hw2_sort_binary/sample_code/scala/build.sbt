import AssemblyKeys._

assemblySettings

// Project name (artifact name in Maven)
name := "homework"

// orgnization name (e.g., the package name of the project)
organization := "com.hank"

version := "1.0-SNAPSHOT"

// project description
description := "Codesensor Homework Project"

// Enables publishing to maven repo
publishMavenStyle := false

// Do not append Scala versions to the generated artifacts
crossPaths := false



// library dependencies. (orginization name) % (project name) % (version)
libraryDependencies ++= Seq(
   "org.msgpack" %% "msgpack-scala" % "0.6.11"
)
