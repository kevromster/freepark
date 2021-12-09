#cs ----------------------------------------------------------------------------

 AutoIt Version: 3.3.14.2
 Author:         Roman Kuskov

 Script Function:
	Takes screenshots from the web-camera given by URL for some interval
	in order to process them later by CarDetector application.

#ce ----------------------------------------------------------------------------

#include <ScreenCapture.au3>

; 1 minute for now
Global Const $SLEEP_INTERVAL_SECS = 60
Global Const $WAIT_BROWSER_OPEN_SECS = 20
Global Const $SCREENSHOTS_DIR = "S:\xproject\TakeScreenshotsForCarDetector\screenshots"

Global Const $CAMERAS_COUNT = 2
Global $CAMERA_URL[$CAMERAS_COUNT], $CAMERA_NAME[$CAMERAS_COUNT], $CAMERA_TITLE[$CAMERAS_COUNT]

; Camera names should be WITHOUT spaces and special characters as they will be used as file names!
$CAMERA_URL[0] = "https://open.ivideon.com/embed/v2/?server=100-22cbd51194b25f1f142ca1a76e55a500&camera=0&width=1920&height=1080&lang=ru&ap=&fs=&noibw="
$CAMERA_NAME[0] = "msk_dolgoprudny_nb5"
$CAMERA_TITLE[0] = "Камера «NB5» (встроенное видео) | Ivideon — Видеонаблюдение через Интернет"

$CAMERA_URL[1] = "http://saferegion.net/cams/iframe/msk_rio_1/85ab0dc3903ccaf5700cb24797a8c5b2/hls/"
$CAMERA_NAME[1] = "yaroslavl_rio"
$CAMERA_TITLE[1] = "iframe video"

Main()

Func getScreenshotName($camera_name)
	return "scr_last_" & $camera_name
EndFunc

Func TakeScreenshot($url, $name, $title)
	ShellExecute($url)
	Sleep($WAIT_BROWSER_OPEN_SECS * 1000)

	Local $browserWindow = WinGetHandle($title)
	Local $img = _ScreenCapture_CaptureWnd($SCREENSHOTS_DIR & "\" & getScreenshotName($name) & ".png", $browserWindow, 155, 105, 3700, 2110, false)
	_WinAPI_DeleteObject($img)
	WinClose($browserWindow)
EndFunc

Func TakeScreenshots()
	For $i = 0 To $CAMERAS_COUNT - 1
		TakeScreenshot($CAMERA_URL[$i], $CAMERA_NAME[$i], $CAMERA_TITLE[$i])
	Next
EndFunc

Func PrepareScreenshotsDir()
	If DirGetSize($SCREENSHOTS_DIR) <> -1 Then
		DirRemove($SCREENSHOTS_DIR, 1)
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
		TakeScreenshots()
		Sleep($SLEEP_INTERVAL_SECS * 1000)
	WEnd
EndFunc
