@ECHO off

SET DeployLocation=bin\
MKDIR %DeployLocation%

ECHO Deploying assets to %DeployLocation%
:: Asset folders
RMDIR "%DeployLocation%\Models"
MKLINK "%DeployLocation%\Models\" "assets\Models" /J
RMDIR "%DeployLocation%\Textures"
MKLINK "%DeployLocation%\Textures\" "assets\Textures\" /J
RMDIR "%DeployLocation%\Audio"
MKLINK "%DeployLocation%\Audio\" "assets\Audio\" /J
RMDIR "%DeployLocation%\Fonts"
MKLINK "%DeployLocation%\Fonts\" "assets\Fonts\" /J

ECHO Deploying resources to %DeployLocation%
:: Schemas
RMDIR "%DeployLocation%\Schema"
MKLINK "%DeployLocation%\Schema\" "resources\Schema" /J
:: Shaders
RMDIR /S /Q "%DeployLocation%\Shaders"
MKLINK "%DeployLocation%\Shaders\" "resources\Shaders" /J
:: Configuration files
MKLINK "%DeployLocation%\DefaultConfig.ini" "resources\DefaultConfig.ini" /H
MKLINK "%DeployLocation%\DefaultInput.ini" "resources\DefaultInput.ini" /H

:: Platform specific binaries
IF "%~1"=="" GOTO :EOF
ECHO Deploying %1 binaries to %DeployLocation%
COPY "deps\bin\%1\x64\*.dll" "%DeployLocation%"

:: Licenses
::ECHO Copying licenses %DeployLocation%
::COPY "libs\assimp-3.1.1\LICENSE" "%ConfigPath%\Assimp License.txt"
::COPY "libs\glew-1.11.0\LICENSE.txt" "%ConfigPath%\GLEW License.txt"
::COPY "libs\glm-0.9.5.4\copying.txt" "%ConfigPath%\GLM License.txt"
::COPY "libs\assimp-3.1.1\LICENSE" "%ConfigPath%\Assimp License.txt"
