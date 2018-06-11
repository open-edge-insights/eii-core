This README describes about how to build the GOLANG specific artifact.

1. Set-up a go developement environment as per the latest GO manual
   For example instruction from following hyperlink can be refered.
	https://golang.org/doc/install
2. Move to src directory under the GO workspace folder and clone the project
	cd $GOPATH/src ; <Clone the REPO>
3. Then change the directory to the folder which has executable go files
   And execute the "go build" command.

   For example in case of stream manager and Data agent build process.
	cd $GOPATH/<REPO_NAME>/cmd/da/ ; "go build"
   This will generate a binary name "da" which represent the data Agent and
   can be used for testing.

4. In case user needs to test binary artifacts from the install path, he/she
   need to use "go install" instead of "go build". This will generate
   executable "da" under $GOPATH/bin/ and
   library stream_mngr.a under $GOPATH/<ARCH>/<REPO_NAME>/

5. To clean all artifact, use "go clean"
