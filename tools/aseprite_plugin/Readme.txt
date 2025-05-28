Aseprite Ultima VIII shapes plug-In
===================================

Aseprite is a popular pixel art editor https://aseprite.org.

This plug-in converts a shp to png and displays all the frames on hitting File -> "Import SHP".

Ultima VIII shapes are in STATIC/U8MOUSE.SHP, STATIC/*.FLX, STATIC/*.SKF (intro and outro, not supported yet).
Other than the u8mouse.shp you need to extract the shap files from the flex containers via flexpack (e.g. "flexpack -x u8shapes.flx").
You will end up with *.fxo files, rename those to *.shp.
Flexpack is bundled with the old Windows snapshot. A newer version that automatically renames shape files to *.shp can be found at 
https://pentagram.sourceforge.net/snapshots/flexpack_win.zip.

The Aseprite plugin skips over empty frames that for some engine reason are plenty in the shapes.

Big shp files will take a little while to be imported into aseprite. Especially the Avatar shape file, 0001.shp, which has 1200 frames.
The helper tool, pent_shp.exe, that the Aseprite plugin uses, can be used for quick and dirty shp conversion to png: 
"pent_shp.exe import name.shp c:\path_to_export_to\"

Issues of the plug-in
=====================
- Currently you can only import a shp file, not open it directly.
- You can not import U8skf files as these have much different palette needs
- Contrary to the Exult Aseprite plugin, offsets are not saved, as it makes no sense in this use case. Adding the offsets is very 
  time consuming in Aseprite.
