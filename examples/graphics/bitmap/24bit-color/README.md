24bit Bitmap Example
=======

This example shows on bottom screen an upscaled version of the nds examples Drunken Coders logo that can be found in devkitPro.

If you want to try with your own image follow these steps:

1. Download & install: http://www.imagemagick.org/ (If you get an option to add the application to the path make sure to check it!).
2. convert fileIn.png -channel B -separate fileIn.png -channel G -separate fileIn.png -channel R -separate -channel RGB -combine -rotate 90 fileOut.rgb
3. Rename fileOut.rgb in fileOut.bin
4. Copy fileOut.bin in the data folder of your project
5. Replace any reference of drunkenlogo_bin in main.cpp with fileOut_bin (or however you named it)
6. Re-Build the project


As you can see from the previos steps the image is clockwise rotated by 90 degrees and its B and R channels are swapped. The first operation is done because the 3DS screens are actually portrait screens rotated by 90 degrees (in a counter-clockwise direction), while the second one is done because the 3DS screens' framebuffers have a BGR888 color format, by default.