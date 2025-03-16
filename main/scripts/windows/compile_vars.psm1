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

Export-ModuleMember -Variable imGuiSrc
Export-ModuleMember -Variable openCVLibs
