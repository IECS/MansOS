MansOS build for Sublime Text
======================================

This is the build system support for Sublime Text editor.


Developing with Sublime Text Editor
--------------------------------------

Sublime Text Editor is a fine editor for any programmer. It can also be used as IDE. To ease developing with Sublime Text we have created a build system that you can integrate with your text editor. Look for the file here:

MansOS/tools/sublime/MansOS make.sublime-build
and copy this file in your Packages/User directory for the Sublime text. The directory can be found from the editor menu: Preferences -> Browse Packages...

Then you can enable the build system by choosing Tools -> Build System -> MansOS make. The usage is as follows: whenever you want to make, compile, or upload your code to a device, choose Tools -> Build Width... or use a shortcut Ctrl + Shift + B and choose the desired action from the list.
