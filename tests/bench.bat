for /l x in (0,1,3000) do (
	if /not 1 equ 2 help
	if "pretty long string"=="another pretty long string" ver
	if /i "hello world"=="HELLO WORLD" help
	if 1 gtr 0 ver
)