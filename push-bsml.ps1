Param(
    [Parameter(Mandatory=$false)]
    [String] $location = "./assets",

    [Parameter(Mandatory=$false)]
    [String] $destination = "/sdcard"
)

$files = Get-ChildItem -Path $location/*.bsml -Name

foreach ($file in $files) {
    & adb push $location/$file $destination/$file
}
