$global:imGuiSrc = @(
    "../imgui-master/imgui.cpp",
    "../imgui-master/imgui_draw.cpp",
    "../imgui-master/imgui_widgets.cpp",
    "../imgui-master/imgui_tables.cpp",
    "../imgui-master/backends/imgui_impl_opengl3.cpp",
    "../imgui-master/backends/imgui_impl_glfw.cpp"
)

$global:openCVLibs = @(
    "-lopencv_core",
    "-lopencv_imgproc",
    "-lopencv_highgui",
    "-lopencv_imgcodecs"
)

function Get-IncludedHeadersRecursive {
    param(
        [string]$file,
        [ref]$visited
    )

    $headers = @()
    $filePath = Resolve-Path $file -ErrorAction SilentlyContinue
    if (-not $filePath) { return $headers }

    if ($visited.Value -contains $filePath) { return $headers }
    $visited.Value += $filePath

    foreach ($line in Get-Content $file) {
        if ($line -match '^\s*#include\s+"(.+\.hpp)"') {
            $included = $matches[1]
            $includedPath = Join-Path (Split-Path $file) $included

            if (Test-Path $includedPath) {
                $headers += $includedPath
                $headers += Get-IncludedHeadersRecursive -file $includedPath -visited ([ref]$visited.Value)
            }
        }
    }

    return $headers
}

function Needs-Rebuild {
    param([string]$cppFile, [string]$objFile)

    if (-not (Test-Path $objFile)) { return $true }

    $cppTime = (Get-Item $cppFile).LastWriteTime
    $objTime = (Get-Item $objFile).LastWriteTime

    if ($cppTime -gt $objTime) { return $true }

    $visited = @()
    $headers = Get-IncludedHeadersRecursive -file $cppFile -visited ([ref]$visited)

    foreach ($header in $headers) {
        $headerTime = (Get-Item $header).LastWriteTime
        if ($headerTime -gt $objTime) { return $true }
    }

    return $false
}

Export-ModuleMember -Variable imGuiSrc
Export-ModuleMember -Variable openCVLibs
Export-ModuleMember -Function Get-IncludedHeadersRecursive, Needs-Rebuild

