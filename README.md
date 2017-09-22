# High Efficiency Image File Format (HEIF) 
相对于原始版本，修复了“损坏的第七块”的问题。<br>
Relative to the original version, fix the "7th tile is corrupted" problem.<br>

About this problem:https://stackoverflow.com/questions/45485622/corrupted-heic-tile-when-converting-to-jpeg<br><br>

无法在 Red Hat 编译，提示 “error: unused parameter ‘xxx’ [-Werror=unused-parameter]” 的话，在 CmakeLists.txt 的 “Set execitable filenames” 那里加上 “CXXFLAGS="-Wno-error=unused-parameter"” 就好了。
