@ECHO off

SET DeployLocation=bin\

ECHO Deploying resources to %DeployLocation%
:: Asset folders
RMDIR "%DeployLocation%\Models"
RMDIR "%DeployLocation%\Textures"
RMDIR "%DeployLocation%\Sounds"
MKLINK "%DeployLocation%\Models\" "assets\Models" /J
MKLINK "%DeployLocation%\Textures\" "assets\Textures\" /J
MKLINK "%DeployLocation%\Sounds\" "assets\Sounds\" /J
:: Configuration files
MKLINK "%DeployLocation%\DefaultConfig.ini" "assets\DefaultConfig.ini" /H
:: Shaders
RMDIR /S /Q "%DeployLocation%\Shaders"
MKDIR "%DeployLocation%\Shaders"
MKLINK "%DeployLocation%\Shaders\OpenGL\" "src\game\Rendering\OpenGL\Shaders\" /J
MKLINK "%DeployLocation%\Shaders\DirectX\" "src\game\Rendering\DirectX\Shaders\" /J
:: Platform specific binaries
IF "%~1"=="" GOTO :EOF
ECHO Deploying %1 binaries to %DeployLocation%
COPY "deps\bin\%1\x64\*.dll" "%DeployLocation%"
:: Licenses
::COPY "libs\assimp-3.1.1\LICENSE" "%ConfigPath%\Assimp License.txt"
::COPY "libs\glew-1.11.0\LICENSE.txt" "%ConfigPath%\GLEW License.txt"
::COPY "libs\glm-0.9.5.4\copying.txt" "%ConfigPath%\GLM License.txt"
::COPY "libs\assimp-3.1.1\LICENSE" "%ConfigPath%\Assimp License.txt"
