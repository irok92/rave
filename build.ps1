$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = "src"
$watcher.Filter = "*.*"
$watcher.IncludeSubdirectories = $true

while ($true)
{
	$result = $watcher.WaitForChanged('Changed', 1000)
	if ($result.TimedOut)
	{
		continue
	}
	Clear-Host
	Write-Output "$($result.ChangeType): $($result.FullPath)"
	clang -g -Isrc/cli -Isrc/lang  src/cli/*.c src/lang/*.c -o rave-cli.exe -Xlinker /SUBSYSTEM:CONSOLE
	# if($LASTEXITCODE -ne 0)
	# {
	# 	Write-Output "Build failed with exit code $LASTEXITCODE"
	# } else
	# {
	# 	Write-Output "Build succeeded."
	# 	./rave-cli.exe
	# }
}
