#cs ----------------------------------------------------------------------------

 AutoIt Version: 3.3.14.2
 Author:         Roman Kuskov

 Script Function:
	Takes screenshots from the web-camera given by URL for every five minutes.

#ce ----------------------------------------------------------------------------

#include <ScreenCapture.au3>

$SLEEP_INTERVAL_SECS = 60 * 10
$WAIT_BROWSER_OPEN_SECS = 15
#$CAMERA_URL = "http://saferegion.net/cams/iframe/msk_rio_1/85ab0dc3903ccaf5700cb24797a8c5b2/hls/"
$CAMERA_URL = "https://open.ivideon.com/embed/v2/?server=100-2269eb3562763334bfe6ad571a9b14ec&camera=65536&width=1920&height=1080&lang=ru&ap=&fs=&noibw="
$SCREENSHOTS_DIR = "S:\xproject\screenshots"

Main()

Func getScreenshotName()
	return "scr_" & @YEAR & @MON & @MDAY & "_" & @HOUR & "-" & @MIN & "-" & @SEC
EndFunc

Func TakeScreenshot()
	ShellExecute($CAMERA_URL)
	Sleep($WAIT_BROWSER_OPEN_SECS * 1000)

	Local $browserWindow = WinGetHandle("Камера «Вид на парковку» (встроенное видео) | Ivideon — Видеонаблюдение через Интернет")
	Local $img = _ScreenCapture_CaptureWnd($SCREENSHOTS_DIR & "\" & getScreenshotName() & ".png", $browserWindow, 155, 105, 3700, 2110, false)
	_WinAPI_DeleteObject($img)
	WinClose($browserWindow)
EndFunc

Func PrepareScreenshotsDir()
	If DirGetSize($SCREENSHOTS_DIR) <> -1 Then
		Local $backupDir = $SCREENSHOTS_DIR & "_prev"
		If DirGetSize($backupDir) <> -1 Then
			DirRemove($backupDir, 1)
		EndIf
		DirMove($SCREENSHOTS_DIR, $backupDir, 1)
	EndIf
	DirCreate($SCREENSHOTS_DIR)
EndFunc

Func Terminate()
	Exit
EndFunc

Func Main()
	HotKeySet("{PAUSE}", "Terminate")
	PrepareScreenshotsDir()

	While 1
		TakeScreenshot()
		Sleep($SLEEP_INTERVAL_SECS * 1000)
	WEnd
EndFunc
