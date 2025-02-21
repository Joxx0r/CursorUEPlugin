@echo off
set "AppName=Cursor.exe"
set "ExePath=C:\Users\<windows_user>\AppData\Local\Programs\cursor\Cursor.exe"
set "IconPath=C:\Users\<windows_user>\AppData\Local\Programs\cursor\resources\app\resources\win32\code.ico"

:: Add the application under SOFTWARE\Classes\Applications
reg add "HKEY_CURRENT_USER\SOFTWARE\Classes\Applications\%AppName%\shell\open\command" /ve /t REG_SZ /d "\"%ExePath%\" \"%%1\"" /f

:: Set a custom icon
reg add "HKEY_CURRENT_USER\SOFTWARE\Classes\Applications\%AppName%\DefaultIcon" /ve /t REG_SZ /d "\"%IconPath%\"" /f

echo Application registered successfully!
pause
