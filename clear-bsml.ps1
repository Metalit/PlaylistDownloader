Param(
    [Parameter(Mandatory=$false)]
    [String] $location = "/sdcard"
)

& adb shell rm $location/*.bsml
