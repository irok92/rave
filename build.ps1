
clang -g -Isrc/cli -Isrc/lang  src/cli/*.c src/lang/*.c -o rave-cli.exe -Xlinker /SUBSYSTEM:CONSOLE
if($LASTEXITCODE -eq 0)
{
	clang -g -Isrc/cli -Isrc/lang -Isrc/tests/lib src/lang/*.c src/tests/lib/*.cpp src/tests/lang/*.cpp -o rave-tests.exe -Xlinker /SUBSYSTEM:CONSOLE
	if($LASTEXITCODE -eq 0)
	{
		Write-Output "Build succeeded."
		./rave-tests.exe
	} else
	{
		Write-Output "Tests build failed with exit code $LASTEXITCODE"
	}
}
