:: Loop through all .hlsl files in the directory
for /r %%f in (*.hlsl) do (
  echo Compiling %%f ...
  fxc /Zi /E"main" /Od /Fo"%%~nf.cso" "%%f" /nologo
)
